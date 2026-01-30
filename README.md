Quark
=============

Quark is a cross-platform GUI framework (`Android` / `iOS` / `macOS` / `Linux`)
designed for building high-performance, interactive applications with a
clear and predictable runtime model.

Quark is implemented primarily in **C++**, with a custom **OpenGL-based
rendering pipeline**, a lightweight **layout engine**, and an embedded
**JavaScript / JSX runtime** for application logic and UI description.

Unlike browser-based frameworks, Quark is **not a web runtime**.
Its architecture and APIs are designed specifically for GUI view trees,
with explicit structure, deterministic behavior, and controllable
performance characteristics.

### Core Capabilities

- **Cross-platform GUI rendering**
  - Android / iOS / macOS / Linux
  - Unified rendering and layout behavior across platforms

- **C++ core with JS / JSX integration**
  - Native performance–critical logic in C++
  - High-level UI and interaction logic written in JavaScript / JSX

- **Lightweight layout engine**
  - Explicit layout models optimized for GUI applications
  - No dependency on browser DOM or CSS layout engines

- **Class-driven style system (CSS-like subset)**
  - Supports class-based selectors (e.g. `.a`, `.a.b`, `.a .b`)
  - Supports hierarchical selectors and limited pseudo states
    (`:normal`, `:hover`, `:active`)
  - Designed for predictable performance and efficient propagation
  - Optimized for GUI usage rather than full web CSS compatibility

- **Deterministic runtime model**
  - Explicit view hierarchy
  - Explicit event handling and state propagation
  - No implicit browser-style reflow or style invalidation

Quark is intended for developers who want **fine-grained control over UI
structure and performance**, without sacrificing development efficiency.

* From here, [`Go API Index`](http://quarks.cc/doc/) takes you to the API Documentation Index.

| ![Screenshot](http://quarks.cc/img/000.jpg) | ![Screenshot](http://quarks.cc/img/001.jpg) | ![Screenshot](http://quarks.cc/img/002.jpg) |
|--|--|--|

Source Code Build
===============

1. Build the required dependencies `Xcode` / `JDK` / `Android-SDK` / `python` / `nodejs`

2. Set the `ANDROID_SDK` and `NDK` environment variables

3. Pull down the dependent libraries and run `make sync`

Compiling and installing qkmake and running `make all` or `make install` will take a long time.

Get the [`Source code`] from `Github` here.

# Simple Example

This is a simple program that displays "Hello world" on the screen.

```tsx
import { Jsx, Application, Window } from 'quark'
new Application();
new Window().render(
	<text value="Hello world" fontSize={48} align="centerMiddle" />
);
```

You can get more detailed [`Examples`]

# Getting Started

If you've never used Quark before, you can start here and build your Quark program step by step.

## Installing qkmake from npm

First, you need to install the Quark toolkit.

* Install `qkmake` using the nodejs `npm` command.
* Open Terminal and execute the following command:

```sh
# shell
$ sudo npm install -g qkmake
```
* Running `qkmake` requires `nodejs` and `python`.

* Currently not supported on Windows; you need to use it on a Mac.

## Creating a New Project Using the qkmake Tool

Use the following `shell` command to create a new `Quark` project:

```sh
# shell
$ mkdir myproj
$ cd myproj
$ qkmake init
```

## Building the Project

This step compresses and packages the JavaScript code and resource files within the project.

If this is a new project, you can skip this step and proceed directly to the next one.

```sh
# shell
$ qkmake build
```

## Export Project

This step exports the [`Xcode`] or [`Android Studio`] project, as the final release will be a ``.apk`` or `.ipa`.

```sh
# shell
# export xcode ios project
$ qkmake export ios
# export android studio project
$ qkmake export android
```

After exporting the project, `project/ios` and `project/android` will be generated in the project root directory, corresponding to the [`Xcode`] and [`Android Studio`] projects, respectively.

## Test HTTP Server

`qkmake` provides a test HTTP server. Every time you modify `ts` or `tsx` code, it notifies devices connected to the server and sets the app's launch address to the debug server address. This allows the interface to automatically update when code changes are made, without having to restart the app.

When you export a project, the startup address is automatically set to this debug address. In most cases, you don't need to modify it unless you want to connect to a different location.

Start the server by executing the following code:

```sh
# shell
$ qkmake watch
# Start web server:
# http://192.168.1.200:1026/
# Watching files change:
```

# View Layout

Views describe all visible elements on the screen and are also responders to events triggered by the hardware and operating system.

For detailed API documentation, please visit [View].

Here are all the [View] classes currently available and their inheritance relationships:

* [ScrollView]
* [MorphView]
* [TextOptions]
* [View]
	* [Sprite]<[MorphView]>
	* [Spine]<[MorphView]>
	* [Box]
		* [Flex]
			* [Flow]
		* [Free]
		* [Image]
			* [Video]
		* [Input]<[TextOptions]>
			* [Textarea]<[ScrollView]>
		* [Scroll]<[ScrollView]>
		* [Text]<[TextOptions]>
			* [Button]
		* [Morph]<[MorphView]>
			* [Root]
	* [Label]


This is a bit like HTML layout:
```tsx
import {Jsx,Application,Window} from 'quark'
new Application()
new Window().render(
	<flex width="100%" height="50%" itemsAlign="centerCenter">
		<button
			minWidth="10%"
			maxWidth="40%"
			height="100%"
			paddingLeft={5}
			lineHeight={1} // 100%
			fontSize={18}
			fontFamily="iconfont"
			backgroundColor="#f00"
			whiteSpace="noWrap"
			textAlign="center"
		>
			<label fontFamily="default" fontSize={16} textOverflow="ellipsis" value="ABCDEFGHIJKMLNOPQ" />
		</button>
		<text
			weight={[0,1]}
			height="100%"
			textColor="#00f"
			lineHeight={1}
			fontSize={16}
			whiteSpace="noWrap"
			weight="bold"
			textOverflow="ellipsisCenter"
			textAlign="center"
			value="Title"
			backgroundColor="#0f0"
		/>
		<text
			minWidth="10%"
			maxWidth="40%"
			height="100%"
			textColor="#f0f"
			lineHeight={1}
			backgroundColor="#0ff"
			textAlign="center"
			value="A"
			opacity={0.5}
		/>
	</flex>
)
```

# CSS Stylesheet

Quark provides a class-driven style system inspired by CSS,
designed specifically for GUI view hierarchies.

* The style system is based on a tree-structured selector model:
each named style rule may have descendant rules separated by spaces,
forming a hierarchical relationship aligned with the view tree.

* The stylesheet data structure is actually a tree. Each named stylesheet can have child stylesheets, separated by spaces. There is no limit to the number of child stylesheets, but in theory, the more levels there are, the slower the query speed.


### Supported selector features

- Class selectors (e.g. `.a`, `.a.b`)
- Hierarchical selectors (e.g. `.a .b`)
- Direct child selectors (e.g. `.a > .b`)
- Pseudo states:
  - `normal`
  - `hover`
  - `active`

### Style transitions

Each style rule may specify a `time` value (in milliseconds)
to indicate the transition duration when switching into that rule.
If `time` is not specified, the style change is applied immediately.

When a transition is triggered, an action is created internally
and played automatically.

### Pseudo states

The style system supports three pseudo states:

1. `normal`  
   Applied when the pointer or touch leaves the view.
2. `hover`  
   Applied when the pointer enters the view or the view gains focus.
3. `active`  
   Applied when the pointer or touch is pressed.

Pseudo states are resolved at runtime based on view interaction events.

### CSS Stylesheet Examples

Here is how to write the stylesheet:
```tsx
import { Jsx, createCss } from 'quark';
createCss({
	'.a': {
		width: 'match',
		lineHeight: 45,
		whiteSpace: 'pre',
		fontSize: 16,
	},
	'.a:normal': {
		textColor: '#0f0',
	},
	'.a:hover': {
		textColor: '#f0f',
	},
	'.a:active': {
		textColor: '#f00',
	},
	'.a .b': {
		fontSize: 20,
	},
	'.a > .c': {
		width: 100,
	},
	'.a:normal .b': {
		time: 500,  // 设置一个过渡时间
		textColor: '#000',
	},
	'.a:hover .b': {
		time: 500,
		textColor: '#f00',
	},
	'.a:action .b:action': { // 这条规则无效，伪类不能存在子伪类
		time: 500,
		textColor: '#f0f',
	}, 
});
const vx = (
	<text class="a" >
		<label value="Hello A!" />
		<label class="b" value="Hello B!" />
	</text>
);
```

# Actions

What is an action? As the name suggests, it's the central hub for managing all actions in the runtime environment, or more simply, animations. It's also one of the core components of the entire framework, providing the ability to create, delete, and insert actions, as well as various operations on keyframes and transitions. Keyframe transitions can use cubic Bezier curves or built-in curves like `linear`/`ease`/`easeIn`/`easeOut`/`easeInOut`, similar to most mainstream frameworks and game engines.

## How do actions work?
How do actions drive smooth view movement? The principle is simple. Think of the action system as an independent system, completely unrelated to the view or rendering. The relationship between them is that changes in the action itself are ultimately reflected in the view. This process is accomplished by calling public methods or properties exposed by the view. It's a completely one-way process, and the view doesn't issue any instructions to the action.

For example, let's create a new keyframe action with two keyframes, and set the x value to change from 0 to 100 over one second. When an action changes, it calls the bound view's API to modify the view's layout properties. In this process, the view is passive, while the action is active.

```ts
import { Application, Window, Box, KeyframeAction } from 'quark'
var app = new Application();
var win = new Window();
var box = new Box(win);
var act = new KeyframeAction(win);
act.add({ x: 0, time: 0 });
act.add({ x: 100, time: 1e3/*milliseconds*/ });
box.width = 50;
box.height = 50;
box.backgroundColor = '#f00';
box.action = act;
box.appendTo(win.root);
act.loop = 1000;
act.play();
```

## Action Types

The following are several types provided by the framework and their inheritance relationships.

* [Action]
	* [SpawnAction]
	* [SequenceAction]
	* [KeyframeAction]

## Action

This is the base type for all actions. It's an abstract type and cannot be instantiated directly.

It provides some basic API operations, such as `Play`, `Stop`, and `Seek`. See the API manual for details.

## SpawnAction

As the name suggests, a parallel action runs its sub-actions in parallel. The duration of the longest sub-action is used as the duration of the action itself, while sub-actions with shorter durations wait for the end of the action or the end of a loop.

## SequenceAction

Sequential actions are relatively easy to understand. Sub-actions execute one after another, ending or starting a new loop after all of them complete.

# KeyframeAction and Keyframe

Keyframe actions are the core of the action system. All actions are implemented here, making them the fundamental unit of the action system. The previous [SpawnAction] and [SpawnAction] only have real meaning when they contain actions of the [KeyframeAction] type. Keyframe actions, in turn, contain the more fundamental element [Keyframe]. Keyframe properties have the same names as CSS properties and include all properties that can be changed on a view. For example, [Matrix] has an `x` property, and [Keyframe] also has an `x` property. However, if a property on a [Keyframe] doesn't exist on a view, then that property will have no effect on the view. For example, there's no `width` property on a [View], so changes to `width` won't affect the [View]. However, if the bound view is a [Box], changes to `width` will affect it, similar to a `CSS` style sheet.

See the following example:
```js
// This is a valid action
var act1 = new KeyframeAction(win);
var box1 = new Box(win);
box1.backgroundColor = '#f00';
act1.add({ width: 10, height: 10 });
act1.add({ width: 100, height: 100, time: 1e3 });
box1.action = act1;
act1.play();
// This is invalid
var act2 = new KeyframeAction(win);
var view = new View(win);
act2.add({ width: 10, height: 10 });
act2.add({ width: 100, height: 100, time: 1e3 });
view.action = act2;
act2.play();
```

# View.onActionKeyframe and View.onActionLoop

These two events are generated and sent by actions.

* `View.onActionKeyframe` fires when an action reaches a keyframe. Because the rendering frame rate is fixed, the event is always triggered when the frame is rendered, so it may be earlier or later than the ideal time. This delay is stored in the `delay` field of the event data. Early is a negative number, and late is a positive number.

* `View.onActionLoop` fires at the beginning of an action loop; it is not triggered for the first time an action is executed. It also has a delay, which is also recorded in `delay`.






[`Examples`]: https://github.com/louis-tru/quark/tree/master/examples
[`Xcode`]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/ConfiguringYourApp/ConfiguringYourApp.html
[`Android Studio`]: https://developer.android.com/studio/projects/create-project.html
[`Android APK`]: https://github.com/louis-tru/quark/releases/download/v0.1.0/examples-release.apk
[`NPM`]: https://www.npmjs.com/package/qkmake
[`Source code`]: https://github.com/louis-tru/quark

[Action]: https://quarks.cc/doc/action.html#class-action
[SpawnAction]: https://quarks.cc/doc/action.html#class-spawnaction
[SequenceAction]: https://quarks.cc/doc/action.html#class-sequenceaction
[KeyframeAction]: https://quarks.cc/doc/action.html#class-keyframeaction
[Keyframe]: https://quarks.cc/doc/action.html#class-keyframe

[View]: https://quarks.cc/doc/view.html#view
[Box]: https://quarks.cc/doc/view.html#box
[View.action]: https://quarks.cc/doc/view.html#view-action
[View.transition()]: https://quarks.cc/doc/view.html#view-transition-to-from-cb-actioncb-

[Notification]: https://quarks.cc/doc/_event.html#class-notification
[ScrollView]: https://quarks.cc/doc/view.html#scrollview
[MorphView]: https://quarks.cc/doc/view.html#morphview
[TextOptions]: https://quarks.cc/doc/view.html#textoptions
[View]: https://quarks.cc/doc/view.html#class-view
[Free]: https://quarks.cc/doc/view.html#class-free
[Box]: https://quarks.cc/doc/view.html#class-box
[Flex]: https://quarks.cc/doc/view.html#class-flex
[Flow]: https://quarks.cc/doc/view.html#class-flow
[Image]:  https://quarks.cc/doc/view.html#class-image
[Root]:  https://quarks.cc/doc/view.html#class-root
[Scroll]: https://quarks.cc/doc/view.html#class-scroll
[Button]: https://quarks.cc/doc/view.html#class-button
[Text]: https://quarks.cc/doc/view.html#class-text
[Input]: https://quarks.cc/doc/view.html#class-input
[Textarea]: https://quarks.cc/doc/view.html#class-textarea
[Label]: https://quarks.cc/doc/view.html#class-label
[Video]: http://quarks.cc/doc/view.html#class-video
[Morph]: http://quarks.cc/doc/view.html#class-morph
[Sprite]: http://quarks.cc/doc/view.html#class-sprite
[Spine]: http://quarks.cc/doc/view.html#class-spine


<script>
	<!--
	var language = (navigator.browserLanguage || navigator.language).toLowerCase();
	var isLanguageCn = language.indexOf('cn') >= 0;
	var isPageCn = location.href.indexOf('README-cn') >=0;
	var isHtml = typeof src == 'string'; // html page will have a src variable

	if ( isLanguageCn ) { // cn
		if ( !isPageCn ) { // goto to cn
			location.href = isHtml ? 'README-cn.html' : 'README-cn.md';
		}
	} else { // en
		if ( isPageCn ) { // goto to en
			location.href = isHtml ? 'README.html' : 'README.md';
		}
	}
	-->
</script>