quark
===============

quark is a cross-platform (Android/iOS/Mac/Linux) front-end development framework. The core code is written in C++. The bottom layer is based on OpenGL drawing. The upper layer implements a streamlined typesetting engine and a JS/JSX running environment. The goal is to develop GUI applications on this basis, which can take both development speed and operation efficiency into account.

* Only `iOS` and `Android` systems are supported for the time being, this does not include `AndroidTV`, because `TV` applications are very different from mobile applications

* From here, [`Go API Index`](http://quarks.cc/doc/) can go to `API Documents Index`

| ![Screenshot](http://quarks.cc/img/0x0ss.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_3.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_4.jpg) |
|--|--|--|

Install qmake from npm
===============

use qkmake tools create project.

first create an empty file directory, then execute in the directory

`qkmake init`

export ios xcode project `qkmake export ios`



Build source
===============

1. Build must dependent `Xcode` / `JDK` / `Android-SDK` / `python` / `nodejs` .

2. Setting environment variable `ANDROID_SDK` and `NDK` directory.

3. Pull dependent librarys, execute `make sync`.

Compile and install qkmake, execute `make` or `make install`, It takes a long time.

[`Simple Examples`](https://github.com/louis-tru/quark/tree/master/docs/README.md)
