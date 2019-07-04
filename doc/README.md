Langou
===============

A GUI typesetting display engine and cross platform GUI application development framework based on NodeJS/OpenGL

Goal: developing GUI applications on this basis can take into account both the simplicity and speed of developing WEB applications, as well as the performance and experience of Native applications.

* Only `iOS` and `Android` systems are supported for the time being, this does not include `AndroidTV`, because `TV` applications are very different from mobile applications

* From here, [`Go API Index`](http://quickgr.org/doc/) can go to `API Documents Index`

| ![Screenshot](http://quickgr.org/img/0x0ss.jpg) | ![Screenshot](http://quickgr.org/img/0x0ss_3.jpg) | ![Screenshot](http://quickgr.org/img/0x0ss_4.jpg) |
|--|--|--|


# Simple Examples

This is a simple program to display Hello world on the screen

```jsx
import { GUIApplication, Root } from 'langou'
new GUIApplication().start(
	<Root>hello world!</Root>
)
```

You can get a more detailed [`Examples`]

# Start Usage

If you've never used Langou before, you can start from here and build your Langou program step by step.

## Install lmake

First, you need to install the toolkit provided by `Langou`

* Install `lmake` using nodejs `npm` 

* Open `Terminal` and execute the following command：

```sh
# shell
$ sudo npm install -g lmake

```
	
* Running `lmake` requires dependency on `nodejs` and `python2.7`

* And now do not support the `windows` system, you need to use it under `mac`

## Create new project

Create a new `Langou` project using the following `shell` command：

```sh
# shell
$ mkdir myproject
$ cd myproject
$ lmake init
```

## Build project

This step compresses and packages the JavaScript code and resource files inside the project,
If this is a new project, you can skip this step and go directly to the next step

```js
# shell
$ lmake build
```

## Export project

This step exports [`Xcode`] or [`Android Studio`] project，because you eventually publish the program that be a `.apk` or `.ipa`

```js
# shell
# export xcode ios project
$ lmake export ios
# export android studio project
$ lmake export android
```

After exporting the project, next you can open it using [`Xcode`] and [`Android Studio`]

## Langou test http server

`lmake` provides a test http server, each time you change the `js` or `jsx` code, you don't have to reinstall every time.

Execute the following code to start it：

```js
# shell
$ lmake
```

# Downloads

* Examples demo [`Android APK`] Install package

* Project [`Source code`] from `Github`


[`Examples`]: https://github.com/louis-tru/langou/tree/master/demo
[`Xcode`]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/ConfiguringYourApp/ConfiguringYourApp.html
[`Android Studio`]: https://developer.android.com/studio/projects/create-project.html
[`Android APK`]: https://github.com/louis-tru/langou/releases/download/v0.1.0/examples-release.apk
[`NPM`]: https://www.npmjs.com/package/lmake
[`Source code`]: https://github.com/louis-tru/langou

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
