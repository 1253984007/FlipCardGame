/****************************************************************************
 Copyright (c) 2014-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#define LOG_TAG "AudioEngineImpl"

#include "audio/android/AudioEngine-inl.h"

#include <unistd.h>
// for native asset manager
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unordered_map>
#include <android/log.h>
#endif

#include <sys/types.h>
#include <thread>
#include <mutex>

#include "audio/include/AudioEngine.h"
#include "platform/CCApplication.h"
#include "base/CCScheduler.h"
#include "base/ccUTF8.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "platform/android/CCFileUtils-android.h"
#include "platform/android/jni/JniHelper.h"
#include "platform/android/jni/JniImp.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
#include "cocos/platform/openharmony/FileUtils-openharmony.h"
#endif

#include "audio/android/IAudioPlayer.h"
#include "audio/android/ICallerThreadUtils.h"
#include "audio/android/AudioPlayerProvider.h"
#include "audio/android/cutils/log.h"
#include "audio/android/UrlAudioPlayer.h"

#include "scripting/js-bindings/event/EventDispatcher.h"
#include "scripting/js-bindings/event/CustomEventTypes.h"

using namespace cocos2d;

// Audio focus values synchronized with which in cocos/platform/android/java/src/com/cocos/lib/CocosNativeActivity.java
namespace {
AudioEngineImpl *gAudioImpl = nullptr;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    int bufferSizeInFrames = getDeviceAudioBufferSizeInFramesJNI();
#elif CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
    // TODO(hack) : There is currently a bug in the opensles module,
    // so openharmony must configure a fixed size, otherwise the callback will be suspended
    int bufferSizeInFrames = 2048;
#endif

int outputSampleRate = 44100;
void getAudioInfo() {
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    outputSampleRate = getDeviceSampleRateJNI();
    #elif CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
    // In openharmony, setting to 48K does not cause audio delays
    outputSampleRate = 48000;
    #endif
}
} // namespace

// Audio focus values synchronized with which in cocos/platform/android/java/src/org/cocos2dx/lib/Cocos2dxActivity.java
static const int AUDIOFOCUS_GAIN = 0;
static const int AUDIOFOCUS_LOST = 1;
static const int AUDIOFOCUS_LOST_TRANSIENT = 2;
static const int AUDIOFOCUS_LOST_TRANSIENT_CAN_DUCK = 3;

static int __currentAudioFocus = AUDIOFOCUS_GAIN;
static AudioEngineImpl* __impl = nullptr;

class CallerThreadUtils : public ICallerThreadUtils
{
public:
    virtual void performFunctionInCallerThread(const std::function<void()>& func)
    {
        Application::getInstance()->getScheduler()->performFunctionInCocosThread(func);
    };

    virtual std::thread::id getCallerThreadId()
    {
        return _tid;
    };

    void setCallerThreadId(std::thread::id tid)
    {
        _tid = tid;
    };

private:
    std::thread::id _tid;
};

static CallerThreadUtils __callerThreadUtils;

static int fdGetter(const std::string& url, off_t* start, off_t* length)
{
    int fd = -1;
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    if (cocos2d::FileUtilsAndroid::getObbFile() != nullptr)
    {
        fd = getObbAssetFileDescriptorJNI(url.c_str(), start, length);
    } 
    if (fd <= 0)
    {
        auto asset = AAssetManager_open(cocos2d::FileUtilsAndroid::getAssetManager(), url.c_str(), AASSET_MODE_UNKNOWN);
        // open asset as file descriptor
        fd = AAsset_openFileDescriptor(asset, start, length);
        AAsset_close(asset);
    }
    #elif CC_TARGET_PLATFORM == CC_PLATFORM_OPENHARMONY
    FileUtilsOpenHarmony* fileUtils = static_cast<FileUtilsOpenHarmony*>(FileUtils::getInstance());
    if(fileUtils) {
        RawFileDescriptor descriptor;
        fileUtils->getRawFileDescriptor(url, descriptor);
        fd = descriptor.fd;
    }
    #endif
    if (fd <= 0)
    {
        ALOGE("Failed to open file descriptor for '%s'", url.c_str());
    }

    return fd;
};

//====================================================
AudioEngineImpl::AudioEngineImpl()
    : _engineObject(nullptr)
    , _engineEngine(nullptr)
    , _outputMixObject(nullptr)
    , _audioPlayerProvider(nullptr)
    , _audioIDIndex(0)
    , _lazyInitLoop(true)
{
    __callerThreadUtils.setCallerThreadId(std::this_thread::get_id());
    __impl = this;
    getAudioInfo();
}

AudioEngineImpl::~AudioEngineImpl()
{
    if (_audioPlayerProvider != nullptr)
    {
        delete _audioPlayerProvider;
        _audioPlayerProvider = nullptr;
    }

    if (_outputMixObject)
    {
        (*_outputMixObject)->Destroy(_outputMixObject);
    }
    if (_engineObject)
    {
        (*_engineObject)->Destroy(_engineObject);
    }

    __impl = nullptr;
}

bool AudioEngineImpl::init()
{
    bool ret = false;
    do{

        // create engine
        auto result = slCreateEngine(&_engineObject, 0, nullptr, 0, nullptr, nullptr);
        if(SL_RESULT_SUCCESS != result){ ERRORLOG("create opensl engine fail"); break; }

        // realize the engine
        result = (*_engineObject)->Realize(_engineObject, SL_BOOLEAN_FALSE);
        if(SL_RESULT_SUCCESS != result){ ERRORLOG("realize the engine fail"); break; }

        // get the engine interface, which is needed in order to create other objects
        result = (*_engineObject)->GetInterface(_engineObject, SL_IID_ENGINE, &_engineEngine);
        if(SL_RESULT_SUCCESS != result){ ERRORLOG("get the engine interface fail"); break; }

        // create output mix
        const SLInterfaceID outputMixIIDs[] = {};
        const SLboolean outputMixReqs[] = {};
        result = (*_engineEngine)->CreateOutputMix(_engineEngine, &_outputMixObject, 0, outputMixIIDs, outputMixReqs);           
        if(SL_RESULT_SUCCESS != result){ ERRORLOG("create output mix fail"); break; }

        // realize the output mix
        result = (*_outputMixObject)->Realize(_outputMixObject, SL_BOOLEAN_FALSE);
        if(SL_RESULT_SUCCESS != result){ ERRORLOG("realize the output mix fail"); break; }

        _audioPlayerProvider = new AudioPlayerProvider(_engineEngine, _outputMixObject, outputSampleRate, bufferSizeInFrames, fdGetter, &__callerThreadUtils);

        ret = true;
    }while (false);

    return ret;
}

void AudioEngineImpl::setAudioFocusForAllPlayers(bool isFocus)
{
    for (const auto& e : _audioPlayers)
    {
        e.second->setAudioFocus(isFocus);
    }
}

int AudioEngineImpl::play2d(const std::string &filePath ,bool loop ,float volume)
{
    ALOGV("play2d, _audioPlayers.size=%d", (int)_audioPlayers.size());
    auto audioId = AudioEngine::INVALID_AUDIO_ID;

    do 
    {
        if (_engineEngine == nullptr || _audioPlayerProvider == nullptr)
            break;

        auto fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);

        audioId = _audioIDIndex++;

        auto player = _audioPlayerProvider->getAudioPlayer(fullPath);
        if (player != nullptr)
        {
            player->setId(audioId);
            _audioPlayers.insert(std::make_pair(audioId, player));

            player->setPlayEventCallback([this, player, filePath](IAudioPlayer::State state){

                if (state != IAudioPlayer::State::OVER && state != IAudioPlayer::State::STOPPED)
                {
                    ALOGV("Ignore state: %d", static_cast<int>(state));
                    return;
                }

                int id = player->getId();

                ALOGV("Removing player id=%d, state:%d", id, (int)state);

                AudioEngine::remove(id);
                if (_audioPlayers.find(id) != _audioPlayers.end())
                {
                    _audioPlayers.erase(id);
                }
                if (_urlAudioPlayersNeedResume.find(id) != _urlAudioPlayersNeedResume.end())
                {
                    _urlAudioPlayersNeedResume.erase(id);
                }

                auto iter = _callbackMap.find(id);
                if (iter != _callbackMap.end())
                {
                    if (state == IAudioPlayer::State::OVER)
                    {
                        iter->second(id, filePath);
                    }
                    _callbackMap.erase(iter);
                }
            });

            player->setLoop(loop);
            player->setVolume(volume);
            player->setAudioFocus(__currentAudioFocus == AUDIOFOCUS_GAIN);
            player->play();
        } 
        else
        {
            ALOGE("Oops, player is null ...");
            return AudioEngine::INVALID_AUDIO_ID;
        }

        AudioEngine::_audioIDInfoMap[audioId].state = AudioEngine::AudioState::PLAYING;
        
    } while (0);

    return audioId;
}

void AudioEngineImpl::setVolume(int audioID,float volume)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        player->setVolume(volume);
    }
}

void AudioEngineImpl::setLoop(int audioID, bool loop)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        player->setLoop(loop);
    }
}

void AudioEngineImpl::pause(int audioID)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        player->pause();
    }
}

void AudioEngineImpl::resume(int audioID)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        player->resume();
    }
}

void AudioEngineImpl::stop(int audioID)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        player->stop();
    }
}

void AudioEngineImpl::stopAll()
{
    if (_audioPlayers.empty())
    {
        return;
    }

    // Create a temporary vector for storing all players since
    // p->stop() will trigger _audioPlayers.erase, 
    // and it will cause a crash as it's already in for loop
    std::vector<IAudioPlayer*> players;
    players.reserve(_audioPlayers.size());

    for (const auto& e : _audioPlayers)
    {
        players.push_back(e.second);
    }

    for (auto p : players)
    {
        p->stop();
    }
}

float AudioEngineImpl::getDuration(int audioID)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        return player->getDuration();
    }
    return 0.0f;
}

float AudioEngineImpl::getDurationFromFile(const std::string &filePath)
{
    if (_audioPlayerProvider != nullptr)
    {
        auto fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        return _audioPlayerProvider->getDurationFromFile(fullPath);
    }
    return 0;
}

float AudioEngineImpl::getCurrentTime(int audioID)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        return player->getPosition();
    }
    return 0.0f;
}

bool AudioEngineImpl::setCurrentTime(int audioID, float time)
{
    auto iter = _audioPlayers.find(audioID);
    if (iter != _audioPlayers.end())
    {
        auto player = iter->second;
        return player->setPosition(time);
    }
    return false;
}

void AudioEngineImpl::setFinishCallback(int audioID, const std::function<void (int, const std::string &)> &callback)
{
    _callbackMap[audioID] = callback;
}

void AudioEngineImpl::preload(const std::string& filePath, const std::function<void(bool)>& callback)
{
    if (_audioPlayerProvider != nullptr)
    {
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        _audioPlayerProvider->preloadEffect(fullPath, [callback](bool succeed, PcmData data){
            if (callback != nullptr)
            {
                callback(succeed);
            }
        });
    }
    else
    {
        if (callback != nullptr)
        {
            callback(false);
        }
    }
}

void AudioEngineImpl::uncache(const std::string& filePath)
{
    if (_audioPlayerProvider != nullptr)
    {
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        _audioPlayerProvider->clearPcmCache(fullPath);
    }
}

void AudioEngineImpl::uncacheAll()
{
    if (_audioPlayerProvider != nullptr)
    {
        _audioPlayerProvider->clearAllPcmCaches();
    }
}

void AudioEngineImpl::onPause()
{
    if (_audioPlayerProvider != nullptr)
    {
        _audioPlayerProvider->pause();
    }
}

void AudioEngineImpl::onResume() 
{
    if (_audioPlayerProvider != nullptr)
    {
        _audioPlayerProvider->resume();
    }
}

// It's invoked from javaactivity-android.cpp
void cocos_audioengine_focus_change(int focusChange)
{
    if (focusChange < AUDIOFOCUS_GAIN || focusChange > AUDIOFOCUS_LOST_TRANSIENT_CAN_DUCK)
    {
        CCLOGERROR("cocos_audioengine_focus_change: unknown value: %d", focusChange);
        return;
    }
    CCLOG("cocos_audioengine_focus_change: %d", focusChange);
    __currentAudioFocus = focusChange;

    if (__impl == nullptr)
    {
        CCLOGWARN("cocos_audioengine_focus_change: AudioEngineImpl isn't ready!");
        return;
    }

    if (__currentAudioFocus == AUDIOFOCUS_GAIN)
    {
        __impl->setAudioFocusForAllPlayers(true);
    }
    else
    {
        __impl->setAudioFocusForAllPlayers(false);
    }
}