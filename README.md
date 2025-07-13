quark
===============

quark is a cross-platform (Android/iOS/Mac/Linux) front-end development framework. The core code is written in C++. The bottom layer is based on OpenGL drawing. The upper layer implements a streamlined typesetting engine and a JS/JSX running environment. The goal is to develop GUI applications on this basis, which can take both development speed and operation efficiency into account.

* Only `iOS` and `Android` systems are supported for the time being, this does not include `AndroidTV`, because `TV` applications are very different from mobile applications
* From here, [`Go API Index`](http://quarks.cc/doc/) can go to `API Documents Index`

| ![Screenshot](http://quarks.cc/img/0x0ss.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_3.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_4.jpg) |
|--|--|--|


Build source
===============

1. Build must dependent `Xcode` / `JDK` / `Android-SDK` / `python` / `nodejs` .
2. Setting environment variable `ANDROID_SDK` and `NDK` directory.
3. Pull dependent librarys, execute `make sync`.

Compile and install qkmake, execute `make` or `make install`, It takes a long time.


# Simple Examples

This is a simple program to display Hello world on the screen

```tsx
import { Jsx,Application,Window } from 'quark'
new Application();
new Window().activate().render(
	<text value="Hello world" textSize={48} align="centerMiddle" />
);
```

You can get a more detailed [`Examples`]

# Start Usage

If you've never used Quark before, you can start from here and build your Quark program step by step.

## Install qkmake from npm

First, you need to install the toolkit provided by `Quark`

* Install `qkmake` using nodejs `npm` 
* Open `Terminal` and execute the following command：

```sh
# shell
$ sudo npm install -g qkmake
```
	
* Running `qkmake` requires dependency on `nodejs` and `python`

* And now do not support the `windows` system, you need to use it under `mac`

## Create new project by qkmake tool

Create a new `Quark` project using the following `shell` command：

```sh
# shell
$ mkdir myproj
$ cd myproj
$ qkmake init
```

## Build project

This step compresses and packages the JavaScript code and resource files inside the project,
If this is a new project, you can skip this step and go directly to the next step

```js
# shell
$ qkmake build
```

## Export project

This step exports [`Xcode`] or [`Android Studio`] project，because you eventually publish the program that be a `.apk` or `.ipa`

```js
# shell
# export xcode ios project
$ qkmake export ios
# export android studio project
$ qkmake export android
```

After exporting the project, next you can open it using [`Xcode`] and [`Android Studio`]

## Quark test http server

`qkmake` provides a test http server, each time you change the `js` or `jsx` code, you don't have to reinstall every time.

Execute the following code to start it：

```js
# shell
$ qkmake watch
```

# Downloads

* Examples demo [`Android APK`] Install package
* Project [`Source code`] from `Github`


[`Examples`]: https://github.com/louis-tru/quark/tree/master/examples
[`Xcode`]: https://developer.apple.com/library/content/documentation/IDEs/Conceptual/AppDistributionGuide/ConfiguringYourApp/ConfiguringYourApp.html
[`Android Studio`]: https://developer.android.com/studio/projects/create-project.html
[`Android APK`]: https://github.com/louis-tru/quark/releases/download/v0.1.0/examples-release.apk
[`NPM`]: https://www.npmjs.com/package/qkmake
[`Source code`]: https://github.com/louis-tru/quark
