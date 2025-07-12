Quark简介
===============

Quark是一个跨平台（Android/iOS）前端开发框架，核心代码使用C++编写，底层基于OpenGL绘图，上层实现了一个精简的排版引擎以及一个JS/JSX运行环境。目标是想实现在此基础上开发GUI应用程序可兼顾开发速度与运行效率。

* [`开源跨平台移动项目Quark【简介】`](http://www.jianshu.com/p/2104b885eae6)

* [`开源跨平台移动项目Quark【入门】`](http://www.jianshu.com/p/b21bf5380c7f)

* [`开源跨平台移动项目Quark【视图与布局系统】`](http://www.jianshu.com/p/4e9d927c3724)

* [`开源跨平台移动项目Quark【Action动作系统】`](http://www.jianshu.com/p/01064b100315)

* [`开源跨平台移动项目Quark【CSS样式表规则及用法】`](http://www.jianshu.com/p/fb86b020554b)

* [`Quark API 文档`](http://quarks.cc/doc/)

# 什么是Action动作

什么是动作呢？顾名思义它是管理运行环境中所有动作的中枢，通俗点讲就是动画。它也是总个框架核心组件之一，它提供动作的创建、删除、插入，以及提供对关键帧与过渡的诸多操作。关键帧的过渡可以使用三次贝塞尔曲线，或内置的曲线  linear/ease/ease_in/ease_out/ease_in_out，这也和大多数主流框架以及游戏引擎类似。

# 动作是什么原理

动作怎么驱动视图进行流畅运动的呢？其实原理很简单，我们可以把动作系统看做一个独立的系统与视图或渲染完全不相关。它们之间的关系是动作自身的变化最终会映射调视图，这个过程是通过调用视图暴露的公有方法或属性来完成的。这个过程完全是单向的，且视图不会向动作发出任何指令。
比如说现在创建了一个新的关键帧动作，给它设置两个关键帧，且x的值经过1秒钟从0变化到100，然后再经过1秒回到0。这个过程是动作自身发生的变化并且带动与之相关的视图一同发生改变，请记住这个过程视图是被动的，而动作才是主动的发生改变。

```js
import { quark, Div } from 'quark';
import KeyframeAction from 'quark/action';
var div = new Div();
var act = new KeyframeAction();
act.add({ x: 0, time: 0 });
act.add({ x: 100, time: 1e3/*毫秒*/ });
act.add({ x: 0, time: 2e3/*毫秒*/ });
div.width = 50;
div.height = 50;
div.backgroundColor = '#f00';
div.action = act;
div.appendTo(quark.root);
act.play();
```

# 动作类别

以下是框架提供的几个类型与继承联系

注：带*号的为抽象类型或协议没有构造函数

* [Action]*
	* [GroupAction]*
		* [SpawnAction] 
		* [SequenceAction]
	* [KeyframeAction]

## Action* 

这是所有动作的基础类型，也是抽象类型不可以直接被实例。
提供了一些基本的api操作，`播放`，`停止`，`跳转` 等，具体可查看API手册。

## GroupAction*

这是个集合的动作类型，提供子动作的添加删除插入。有了子动作就可以帮你实现更加复杂的动画场景。
它也有两个具体的子类型 [SpawnAction] 、[SequenceAction]。

## SpawnAction

并行动作顾名思义即就是它的子动作都是并行运行的。并且以最长子动作的时长做为自身的时长来执行动作，较短时长的子动作则在结尾等待动作的结束或一个循环的的终止。

## SequenceAction

串行动作这个比较好理解，子动作一个接着一个执行，全部执行完成后结束或开始新的循环。

# KeyframeAction与Frame

关键帧动作这是动作系统的核心。所有动作的实现均在这里完成它是动作系统基本单元，前面的[GroupAction]只有包含[KeyframeAction]类型的动作才有真正的义意。
而关键帧动作又包含理更加基本的元素`关键帧`[Frame],关键帧包含的属性与`CSS`属性是同名的且与所有视图的属性都是对应关键。通俗的说比如[View]上会有`x`属性而[Frame]上也会有`x`属性，如果关键帧上有视图上并不存在的属性，那么这个属性对视图是无效的。比如[View]上就不存在`width`属性那么这个属性的改变不会影响到[View],但如果绑定的视图是[Div]那么`width`的改变一定会影响到它，这与`CSS`样式表类似。

看下面的例子:
```js
// 这是有效的动作
var act1 = new KeyframeAction();
var div = new Div();
div.backgroundColor = '#f00';
act1.add({ width: 10, height: 10 });
act1.add({ width: 100, height: 100, time: 1e3 });
div.action = act1;
act1.paly();
// 这是无效的
var act2 = new KeyframeAction();
var view = new View();
act2.add({ width: 10, height: 10 });
act2.add({ width: 100, height: 100, time: 1e3 });
view.action = act2;
act2.paly();
```

# View.action属性

[View.action]做为[View]的一个属性可接收多种类型的参数，之前给大家展示的例子中创建动作是很繁琐的，但`active`提供多种类型的参数类型的支持，包括`json`数据与`Action`对像实例本身。前面的例子中已介绍过`Action`方式，下面着重说`json`数据方法。大家也可研读`quark.js`与`action.js`中的源代码，其它[View.action]属性只是做简单的调用转发，功能的实现其实是在`action.js`文件中的`create()`方法里实现的。

看例子:
```js
// 这是创建KeyframeAction
var div = new Div();
div.action = {
	keyframe: [
		{ x: 0 },
		{ x: 100, time: 500 },
		{ x: 0, time: 1000 },
	],
	loop: -1,
};
var div2 = new Div();
div.action = [
	{ x: 0 },
	{ x: 100, time: 500 },
];

// 这是创建SequenceAction
var div3 = new Div();
div3.action = {
	seq: [
		[ // 这是个子KeyframeAction
			{ x: 0 },
			{ x: 100, time: 1e3 },
		],
		{ // 这是个子SpawnAction
			spawn: [
				[ // 这是个子KeyframeAction
					{ y : 100 }, { y: 200, time: 2e3 }
				],
				[ // 这是个子KeyframeAction
					{ width : 200 }, { width: 100, time: 1e3 }
				],
			] 
		}
	],
};

// 这是创建SpawnAction
var div4 = new Div();
div4.action = {
	spawn: [ // 这里只包含一个子KeyframeAction
		{x: 0}, {x: 200, time: 2e3} 
	]
};
```

大家可以看到上面的例子中有4种典型的创建方法。主要看你给的`json`数据是否存在这三个属性`seq`、`spawn`、`keyframe`，对应[SpawnAction]、[SequenceAction]、[KeyframeAction]，外加一个`json`数据类型检查，数据类型为数组就创建[KeyframeAction]。并且这可以嵌套使用。

# View.transition()方法

这是一个简单创建简单过渡动画的方法，实现原型为`action.js`的`transition()`方法，与[View.action]一样[View.transition()]只做简单的转发。

典型应用：
```js
view.transition({
    time: 1000,
    y: 100, 
    x: 100,
})
```

具体可查阅手册。

# View.onActionKeyframe与View.onActionLoop

这两个事件是由动作产生并发送的。

* `View.onActionKeyframe`为动作执行到达关键帧时触发。因为画面渲染是固定的帧率，触发总是发生在帧的渲染时，所以可能会与理想中的时间值有所误差提前或延后，这个延时值会保存在事件数据的`delay`上。提前为负数，延时为正数。

* `View.onActionLoop`动作循环开始时触发，第一次执行动作并不会触发。同样它也会有延时，也同样记录在`delay`。

[Action]: https://quarks.cc/doc/action.html#class-action
[GroupAction]: https://quarks.cc/doc/action.html#class-groupaction
[SpawnAction]: https://quarks.cc/doc/action.html#class-spawnaction
[SequenceAction]: https://quarks.cc/doc/action.html#class-sequenceaction
[KeyframeAction]: https://quarks.cc/doc/action.html#class-keyframeaction
[Frame]: https://quarks.cc/doc/action.html#class-frame
[View]: https://quarks.cc/doc/quark.html#class-view
[Div]: https://quarks.cc/doc/quark.html#class-div
[View.action]: https://quarks.cc/doc/quark.html#set-view-action
[View.transition()]: https://quarks.cc/doc/quark.html#view-transition-style-delay-cb-