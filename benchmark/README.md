Ngui简介
===============

Ngui是一个跨平台（Android/iOS）前端开发框架，核心代码使用C++编写，底层基于OpenGL绘图，上层实现了一个精简的排版引擎以及一个JS/JSX运行环境。目标是想实现在此基础上开发GUI应用程序可兼顾开发速度与运行效率。

* [`开源跨平台移动项目Ngui【简介】`](http://www.jianshu.com/p/2104b885eae6)

* [`开源跨平台移动项目Ngui【入门】`](http://www.jianshu.com/p/b21bf5380c7f)

* [`开源跨平台移动项目Ngui【视图与布局系统】`](http://www.jianshu.com/p/4e9d927c3724)

* [`开源跨平台移动项目Ngui【Action动作系统】`](http://www.jianshu.com/p/01064b100315)

* [`开源跨平台移动项目Ngui【CSS样式表规则及用法】`](http://www.jianshu.com/p/fb86b020554b)

* [`Ngui API 文档`](http://ngui.fun/doc/)

# Benchmark

在`iPhone6`上创建10万个Div并对相关属性进行设置需要10秒以上的时间，但在`Android`上这个时间会减少到1/2，
因为`Android`使用的V8，而iOS上是使用的`JSC`并通过胶水层粘合到`V8`的API，所以JS性能不如`Android`。

对于2d绘图GPU不是主要瓶颈，主要瓶颈集中在CPU，但可优化的空间还很大。

测试主要集中在图形方面，这包括JS调用API的时间开销，图形绘制的帧率，CPU的运行百分占比。对于文件IO以以及网络方面的测试相对比较少，因为库IO本身是基于`NodeJS`基本没有任何改动。

下面的数据是对`iPhone6`、 `Google Nexus6` 、`iPad mini2`的测试结果。

* 注意下面的时间单位都为毫秒，CPU占比以单核为准100%表示一个CPU核心满载运行。
* 还有一点需要注意在屏幕没有任何变化时，CPU占用一般为1%左右。

## View

在一个全屏Scroll视图中创建10万个Div视图，然后滚动这个Scroll，这时查看CPU占比以及频幕刷新率。
Div视图是自动布局的，所以10万个Div不会在屏幕中同时出现。这主要测试Dom的操作、视图排版布局、以及绘图性能。

| 设备 | Div数量 | 创建时间 | Fsp |CPU占比|
|----------|-------|------|-----|------|
|iPhone6  | 10000 | 1257 |	60	| 45% |
|Nexus6    | 10000 | 670  |	60	| 48% |
|iPad mini2| 10000 | 1269 | 60	| 60%  |
|iPhone6  | 20000 | 2457 |	60	| 58% |
|Nexus6    | 20000 | 1265 |	60	| 70% |
|iPad mini2| 20000 | 2460 |	60	| 90%  |
|iPhone6   | 50000 | 6162 |	48	| 97% |
|Nexus6    | 50000 | 2987 |	39	| 97% |
|iPad mini2| 50000 | 5959 |	42	| 97% |
|iPhone6  | 100000 | 12647 | 25	| 97% |
|Nexus6    | 100000 | 5859 | 20	| 97% |
|iPad mini2| 100000 | 11964 |	22	| 97% |

* 这是iPhone6截图：

![这是iPhone6截图](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcxMjE4MDAzMjM1NTcx)

* 下面是主要的测试源码:

```jsx
new GUIApplication({ multisample: 2 }).start(
	<Root backgroundColor="#000">
		<Scroll width="full" height="full">
			${
				Array.from({length:100000}, ()=>{
					var color = new Color(random(0, 255), 
						random(0, 255), random(0, 255), 255);
					return <Div backgroundColor=color class="item" />;
				})
			}
		</Scroll>
	</Root>
)
```

## Action

在同一屏幕随机创建4000个视图，并随机设置旋转动作，这时查看CPU占用，与屏幕刷新率。这主要测试动作系统性能、同屏绘图性能，以及设备的CPU与GPU的性能。CPU占比越低帧数越高表示性能越好。
可

| 设备 | 数量 | Fsp |CPU占比|
|---------|-------|-----|------|
|iPhone6  | 1000 |	60	| 54% |
|Nexus6    | 1000 |	60	| 65% |
|iPad mini2| 1000 | 60	| 90% |
|iPhone6  | 2000 |	60	| 98% |
|Nexus6    | 2000 |	40	| 110% |
|iPad mini2| 2000 |	40	| 88% |
|iPhone6   | 4000 |	30	| 104% |
|Nexus6    | 4000 |	20	| 110% |
|iPad mini2| 4000 |	25	| 104% |

从数据上可以看出Nexus6的单核CPU性能不如iPhone6。

* 下面是iPhone6截图：

![这是iPhone6截图](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcxMjE5MDkzOTU5OTc0)

## CSS

创建10万个样式表所消耗的时间。

| 设备 | 创建时间 |
|----------|------|
|iPhone6  | 14699 |
|Nexus6    | 10381  |
|iPad mini2| 14808 |

这里与Dom操作很类似时间有点长，这是因为对属性值的解析是通过调用JS方法完成，如果这个过程在Native中，这个时间会减少很多，这也是以后的版本所需要解决的问题。

## Storage

下面分别是调用20万次`storage.set()`、20万次`storage.get()`、20万次`storage.del()`消耗的时间。

| 设备 | set() | get() | del() |
|----------|------|------|------|
|iPhone6  | 4381 | 3821 | 3547 |
|Nexus6    | 7178  | 6539 | 6567 |
|iPad mini2| 4951 | 4256 | 4179 |


# End

上面的数据只能做大体参考，对这种GUI框架的测试我现在还没有找到比较好的标准。