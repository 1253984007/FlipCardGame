window.__require = function t(e, o, n) {
function r(c, a) {
if (!o[c]) {
if (!e[c]) {
var s = c.split("/");
s = s[s.length - 1];
if (!e[s]) {
var p = "function" == typeof __require && __require;
if (!a && p) return p(s, !0);
if (i) return i(s, !0);
throw new Error("Cannot find module '" + c + "'");
}
c = s;
}
var _ = o[c] = {
exports: {}
};
e[c][0].call(_.exports, function(t) {
return r(e[c][1][t] || t);
}, _, _.exports, t, e, o, n);
}
return o[c].exports;
}
for (var i = "function" == typeof __require && __require, c = 0; c < n.length; c++) r(n[c]);
return r;
}({
Rotate_Y: [ function(t, e, o) {
"use strict";
cc._RF.push(e, "f35499UdFJP5JeRtgwSk7OA", "Rotate_Y");
var n, r = this && this.__extends || (n = function(t, e) {
return (n = Object.setPrototypeOf || {
__proto__: []
} instanceof Array && function(t, e) {
t.__proto__ = e;
} || function(t, e) {
for (var o in e) Object.prototype.hasOwnProperty.call(e, o) && (t[o] = e[o]);
})(t, e);
}, function(t, e) {
n(t, e);
function o() {
this.constructor = t;
}
t.prototype = null === e ? Object.create(e) : (o.prototype = e.prototype, new o());
}), i = this && this.__decorate || function(t, e, o, n) {
var r, i = arguments.length, c = i < 3 ? e : null === n ? n = Object.getOwnPropertyDescriptor(e, o) : n;
if ("object" == typeof Reflect && "function" == typeof Reflect.decorate) c = Reflect.decorate(t, e, o, n); else for (var a = t.length - 1; a >= 0; a--) (r = t[a]) && (c = (i < 3 ? r(c) : i > 3 ? r(e, o, c) : r(e, o)) || c);
return i > 3 && c && Object.defineProperty(e, o, c), c;
};
Object.defineProperty(o, "__esModule", {
value: !0
});
var c = t("./gamepanel"), a = cc._decorator, s = a.ccclass, p = a.property, _ = function(t) {
r(e, t);
function e() {
var e = null !== t && t.apply(this, arguments) || this;
e.back_node = null;
e.time = 1;
e.now_state = !1;
e.back_state = !0;
e.end_sign = !1;
return e;
}
e.prototype.start_Rotate = function() {
var t = this;
if (!this.end_sign && !this.now_state) {
this.now_state = !0;
cc.tween(this.node).to(this.time / 2, {
scaleX: 0
}).call(function() {
var e = t.node.getContentSize();
t.chenge_display();
t.node.setContentSize(e);
}).to(this.time / 2, {
scaleX: 1
}).call(function() {
t.now_state = !1;
t.back_state ? t.node.getParent().getComponent(c.default)._C = null : t.node.getParent().getComponent(c.default).check(t.node);
}).start();
}
};
e.prototype.chenge_display = function() {
this.back_state = !this.back_state;
this.back_node.active = this.back_state;
};
e.prototype.start = function() {
this.back_node.active = this.back_state;
};
i([ p(cc.Node) ], e.prototype, "back_node", void 0);
i([ p ], e.prototype, "time", void 0);
return i([ s ], e);
}(cc.Component);
o.default = _;
cc._RF.pop();
}, {
"./gamepanel": "gamepanel"
} ],
S_Script: [ function(t, e, o) {
"use strict";
cc._RF.push(e, "312fdKr8EpBH68D78Ll0//g", "S_Script");
var n, r = this && this.__extends || (n = function(t, e) {
return (n = Object.setPrototypeOf || {
__proto__: []
} instanceof Array && function(t, e) {
t.__proto__ = e;
} || function(t, e) {
for (var o in e) Object.prototype.hasOwnProperty.call(e, o) && (t[o] = e[o]);
})(t, e);
}, function(t, e) {
n(t, e);
function o() {
this.constructor = t;
}
t.prototype = null === e ? Object.create(e) : (o.prototype = e.prototype, new o());
}), i = this && this.__decorate || function(t, e, o, n) {
var r, i = arguments.length, c = i < 3 ? e : null === n ? n = Object.getOwnPropertyDescriptor(e, o) : n;
if ("object" == typeof Reflect && "function" == typeof Reflect.decorate) c = Reflect.decorate(t, e, o, n); else for (var a = t.length - 1; a >= 0; a--) (r = t[a]) && (c = (i < 3 ? r(c) : i > 3 ? r(e, o, c) : r(e, o)) || c);
return i > 3 && c && Object.defineProperty(e, o, c), c;
};
Object.defineProperty(o, "__esModule", {
value: !0
});
var c = cc._decorator, a = c.ccclass, s = (c.property, function(t) {
r(e, t);
function e() {
return null !== t && t.apply(this, arguments) || this;
}
e.prototype.s_game = function() {
cc.director.loadScene("game_view");
};
return i([ a ], e);
}(cc.Component));
o.default = s;
cc._RF.pop();
}, {} ],
gamepanel: [ function(t, e, o) {
"use strict";
cc._RF.push(e, "4ae147Vr89KsYgAjdmSlSMj", "gamepanel");
var n, r = this && this.__extends || (n = function(t, e) {
return (n = Object.setPrototypeOf || {
__proto__: []
} instanceof Array && function(t, e) {
t.__proto__ = e;
} || function(t, e) {
for (var o in e) Object.prototype.hasOwnProperty.call(e, o) && (t[o] = e[o]);
})(t, e);
}, function(t, e) {
n(t, e);
function o() {
this.constructor = t;
}
t.prototype = null === e ? Object.create(e) : (o.prototype = e.prototype, new o());
}), i = this && this.__decorate || function(t, e, o, n) {
var r, i = arguments.length, c = i < 3 ? e : null === n ? n = Object.getOwnPropertyDescriptor(e, o) : n;
if ("object" == typeof Reflect && "function" == typeof Reflect.decorate) c = Reflect.decorate(t, e, o, n); else for (var a = t.length - 1; a >= 0; a--) (r = t[a]) && (c = (i < 3 ? r(c) : i > 3 ? r(e, o, c) : r(e, o)) || c);
return i > 3 && c && Object.defineProperty(e, o, c), c;
};
Object.defineProperty(o, "__esModule", {
value: !0
});
var c = t("./Rotate_Y"), a = t("./uni_display"), s = cc._decorator, p = s.ccclass, _ = s.property, l = function(t) {
r(e, t);
function e() {
var e = null !== t && t.apply(this, arguments) || this;
e.num_x = 0;
e.son_mod = null;
e.f_s = [];
e.sons = [];
e._t = [];
e.init_ing = !1;
e._C = null;
return e;
}
e.prototype.back = function() {
cc.director.loadScene("start_view");
};
e.prototype.init_panel = function(t, e) {
if (!this.init_ing) {
this.init_ing = !0;
this.sons.forEach(function(t) {
t.destroy();
});
this._t = [];
for (var o = 0; o < e * this.num_x; o += 2) {
var n = Math.floor(40 * Math.random());
this._t[o] = n;
this._t[o + 1] = n;
}
var r = function(t) {
cc.tween(i).to(t / 10, {}).call(function(e) {
var o = cc.instantiate(e.son_mod);
e.node.addChild(o);
var n = e._t.splice(Math.floor(Math.random() * e._t.length), 1);
o.getComponent(a.default).init_display(n[0]);
e.sons[t] = o;
if (0 == t) {
e.uni_size = o.getContentSize();
e.node.setContentSize((e.uni_size.width + 5) * e.num_x - 5, e.uni_size.height);
}
0 == e._t.length && (e.init_ing = !1);
}).start();
}, i = this;
for (o = 0; o < e * this.num_x; o++) r(o);
}
};
e.prototype.check = function(t) {
if (this._C != t) if (null != this._C) {
if (this._C.getComponent(a.default).id != t.getComponent(a.default).id) {
this._C.getComponent(c.default).start_Rotate();
t.getComponent(c.default).start_Rotate();
} else {
this._C.getComponent(c.default).end_sign = !0;
t.getComponent(c.default).end_sign = !0;
}
this._C = null;
} else this._C = t; else {
this._C.getComponent(c.default).start_Rotate();
this._C = null;
}
};
i([ _(cc.Integer) ], e.prototype, "num_x", void 0);
i([ _(cc.Prefab) ], e.prototype, "son_mod", void 0);
i([ _(cc.SpriteFrame) ], e.prototype, "f_s", void 0);
return i([ p ], e);
}(cc.Component);
o.default = l;
cc._RF.pop();
}, {
"./Rotate_Y": "Rotate_Y",
"./uni_display": "uni_display"
} ],
uni_display: [ function(t, e, o) {
"use strict";
cc._RF.push(e, "68c40uXz5FDe7ECYEMzKZTV", "uni_display");
var n, r = this && this.__extends || (n = function(t, e) {
return (n = Object.setPrototypeOf || {
__proto__: []
} instanceof Array && function(t, e) {
t.__proto__ = e;
} || function(t, e) {
for (var o in e) Object.prototype.hasOwnProperty.call(e, o) && (t[o] = e[o]);
})(t, e);
}, function(t, e) {
n(t, e);
function o() {
this.constructor = t;
}
t.prototype = null === e ? Object.create(e) : (o.prototype = e.prototype, new o());
}), i = this && this.__decorate || function(t, e, o, n) {
var r, i = arguments.length, c = i < 3 ? e : null === n ? n = Object.getOwnPropertyDescriptor(e, o) : n;
if ("object" == typeof Reflect && "function" == typeof Reflect.decorate) c = Reflect.decorate(t, e, o, n); else for (var a = t.length - 1; a >= 0; a--) (r = t[a]) && (c = (i < 3 ? r(c) : i > 3 ? r(e, o, c) : r(e, o)) || c);
return i > 3 && c && Object.defineProperty(e, o, c), c;
};
Object.defineProperty(o, "__esModule", {
value: !0
});
var c = t("./gamepanel"), a = cc._decorator, s = a.ccclass, p = a.property, _ = function(t) {
r(e, t);
function e() {
var e = null !== t && t.apply(this, arguments) || this;
e.f_1 = null;
e.f_2 = null;
e.t_1 = null;
e.t_2 = null;
return e;
}
e.prototype.init_display = function(t) {
this.id = t;
var e = this.node.getParent().getComponent(c.default);
this.sum = t % 10 + 1;
this.flower = Math.floor(t / 10);
var o = this.flower > 1 ? cc.Color.BLACK : cc.Color.RED;
this.f_1.spriteFrame = e.f_s[this.flower];
this.f_2.spriteFrame = e.f_s[this.flower + 4];
this.t_1.string = 1 == this.sum ? "A" : this.sum.toString();
this.t_1.node.color = o;
};
i([ p(cc.Sprite) ], e.prototype, "f_1", void 0);
i([ p(cc.Sprite) ], e.prototype, "f_2", void 0);
i([ p(cc.Label) ], e.prototype, "t_1", void 0);
i([ p(cc.Label) ], e.prototype, "t_2", void 0);
return i([ s ], e);
}(cc.Component);
o.default = _;
cc._RF.pop();
}, {
"./gamepanel": "gamepanel"
} ]
}, {}, [ "Rotate_Y", "S_Script", "gamepanel", "uni_display" ]);