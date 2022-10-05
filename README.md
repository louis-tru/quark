quark
===============

quark is a cross-platform (Android/iOS) front-end development framework. The core code is written in C++. The bottom layer is based on OpenGL drawing. The upper layer implements a streamlined typesetting engine and a JS/JSX running environment. The goal is to develop GUI applications on this basis, which can take both development speed and operation efficiency into account.

* Only `iOS` and `Android` systems are supported for the time being, this does not include `AndroidTV`, because `TV` applications are very different from mobile applications

* From here, [`Go API Index`](http://quarks.cc/doc/) can go to `API Documents Index`

| ![Screenshot](http://quarks.cc/img/0x0ss.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_3.jpg) | ![Screenshot](http://quarks.cc/img/0x0ss_4.jpg) |
|--|--|--|


Build source and install noproj
===============

1. Build must dependent `Xcode` / `JDK` / `Android-SDK` / `NDK` / `python` / `nodejs` / `yasm`.

2. Setting environment variable `ANDROID_SDK` and `ANDROID_NDK` directory.

3. Pull dependent librarys, execute `make pull`.

Compile and install noproj, execute `make` or `make install`, It takes a long time.

use noproj tools create project.

first create an empty file directory, then execute in the directory

`noproj init`

export ios xcode project `noproj export ios`


[`Simple Examples`](https://github.com/louis-tru/quark/tree/master/docs/README.md)
