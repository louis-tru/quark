flare (fast layout render)
===============

flare is a cross-platform (Android/iOS) front-end development framework. The core code is written in C++. The bottom layer is based on OpenGL drawing. The upper layer implements a streamlined typesetting engine and a JS/JSX running environment. The goal is to develop GUI applications on this basis, which can take both development speed and operation efficiency into account.

* Only `iOS` and `Android` systems are supported for the time being, this does not include `AndroidTV`, because `TV` applications are very different from mobile applications

* From here, [`Go API Index`](http://flare.cool/doc/) can go to `API Documents Index`

| ![Screenshot](http://flare.cool/img/0x0ss.jpg) | ![Screenshot](http://flare.cool/img/0x0ss_3.jpg) | ![Screenshot](http://flare.cool/img/0x0ss_4.jpg) |
|--|--|--|


# Simple Examples

This is a simple program to display Hello world on the screen

```jsx
import { GUIApplication, Root } from 'flare'
new GUIApplication().start(
	<Root>hello world!</Root>
)
```

You can get a more detailed [`Examples`]

# Start Usage

If you've never used Flare before, you can start from here and build your Flare program step by step.

## Install fproj

First, you need to install the toolkit provided by `Flare`

* Install `fproj` using nodejs `npm` 

* Open `Terminal` and execute the following command：

```sh
# shell
$ sudo npm install -g fproj

```
	
* Running `fproj` requires dependency on `nodejs` and `python2.7`

* And now do not support the `windows` system, you need to use it under `mac`

## Create new project

Create a new `Flare` project using the following `shell` command：

```sh
# shell
$ mkdir myproject
$ cd myproject
$ fproj init
```

## Build project

This step compresses and packages the JavaScript code and resource files inside the project,
If this is a new project, you can skip this step and go directly to the next step

```js
# shell
$ fproj build
```

## Export project

This step exports [`Xcode`] or [`Android Studio`] project，because you eventually publish the program that be a `.apk` or `.ipa`

```js
# shell
# export xcode ios project
$ fproj export ios
# export android studio project
$ fproj export android
```

After exporting the project, next you can open it using [`Xcode`] and [`Android Studio`]

## Flare test http server

`fproj` provides a test http server, each time you change the `js` or `jsx` code, you don't have to reinstall every time.

Execute the following code to start it：

```js
# shell
$ fproj
```

# Downloads

* Examples demo [`Android APK`] Install package

* Project [`Source code`] from `Github`


[`Examples`]: https://github.com/louis-tru/flare/tree/master/examples
[`Xcode`]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/ConfiguringYourApp/ConfiguringYourApp.html
[`Android Studio`]: https://developer.android.com/studio/projects/create-project.html
[`Android APK`]: https://github.com/louis-tru/flare/releases/download/v0.1.0/examples-release.apk
[`NPM`]: https://www.npmjs.com/package/fproj
[`Source code`]: https://github.com/louis-tru/flare

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
