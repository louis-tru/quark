Ngui简介
===============

这是一个GUI的排版显示引擎和跨平台的GUI应用程序开发框架，基于NodeJS/OpenGL，这也是第一个在移动端Android/iOS融合NodeJS的前端GUI项目，至此JavaScript成为了真正意义上前后端通吃的语言。

Ngui的目标：在此基础上开发GUI应用程序可拥有开发WEB应用般简单与速度同时兼顾Native应用程序的性能与体验。

* 暂时只支持`iOS`与`Android`系统，并不包含`AndroidTV`因为`TV`应用与手机应用有很大的不同

* 从这里[`Go API Index`](http://nodegui.org/doc/)可以到 `API Documents Index`

# Simple Examples

这是一个简单的在屏幕上显示`hello world!`的程序

```jsx
import { GUIApplication, Root } from 'ngui'
new GUIApplication().start(
	<Root>hello world!</Root>
)
```

你可以获取到更加详细的[Examples]

# Start Usage

如果你从来没有使用过`Ngui`你可以从这里开始，一步步创建你的`Ngui`程序。

## Install ngui-tools

首先你需要安装`Ngui`提供的工具包

* 使用 nodejs `npm` 安装 `ngui-tools`

* 打开`Terminal`并执行以下命令：

```sh
# shell
$ sudo npm install -g ngui-tools

```
	
* 运行`ngui-tools` 需要依赖`nodejs`与`python2.7`

* 并且不能运行在`windows`系统, 暂时只能在`mac`下使用

## Create new project

使用下面的`shell`命令创建一个新的`Ngui`工程：

```sh
# shell
$ mkdir myproject
$ cd myproject
$ ngui init
```

## Build project

这一步会把工程里面的javascript代码以及资源文件进行压缩并打包，如果这是一个新的工程可以跳过这一步骤直接到下一步

```sh
# shell
$ ngui build
```

## Export project

这一步导出[Xcode]或[Android Studio]工程，因为你最终要发布程序将会是一个`.apk`或`.ipa`

```sh
# shell
# export xcode ios project
$ ngui export ios
# export android studio project
$ ngui export android
```

导出工程后，接下来你可以使用[Xcode]与[Android Studio]打开它


## Ngui test http server

`ngui-tools`提供了一个测试http服务器，你不需要每次修改完`js`或`jsx`代码都进行重新安装

执行下面的代码可以启动它：

```sh
# shell
$ ngui
```

# Downloads

* Examples demo [Android APK] Install package

* Project [Source code] from `Github`


[Examples]: https://github.com/louis-tru/ngui/tree/master/demo
[Xcode]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/ConfiguringYourApp/ConfiguringYourApp.html
[Android Studio]: https://developer.android.com/studio/projects/create-project.html
[Android APK]: https://github.com/louis-tru/ngui/releases/download/v0.1.0/examples-release.apk
[NPM]: https://www.npmjs.com/package/ngui-tools
[Source code]: https://github.com/louis-tru/ngui


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

