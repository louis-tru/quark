# `noug/sys`

与操作系统相关的api

`下面的实例均为在ios系统中运行结果`

## time()
* @ret {[`uint64`]} ms

获取当前系统时间戳(毫秒)

## timeMonotonic()
* @ret {[`uint64`]} ms

获取从系统启动时开始记录的单调时间戳(毫秒),这个时间不受外部修改影响

## name()
* @ret {[`String`]}

获取系统名称,这个值包括 `'iOS'`|`'Android'`|`'Linux'`|`'MacOSX'`

## info()
* @ret {[`String`]}

获取系统信息

Example:

```js
// Prints:
// host: Louis-iPhone
// sys: Darwin
// machine: iPhone7,2
// nodename: Louis-iPhone
// version: Darwin Kernel Version 16.6.0: Mon Apr 17 17:33:35 PDT 2017; root:xnu-3789.60.24~24/RELEASE_ARM64_T7000
// release: 16.6.0
console.log(sys.info());
```

## version()
* @ret {[`String`]}

获取系统版本字符串

Example:

```
// Prints: 10.3.2
console.log(sys.version());
```

## brand()
* @ret {[`String`]}

获取设备品牌名称

Example:

```
// Prints: Apple
console.log(sys.brand());
```

## subsystem()
* @ret {[`String`]}

获取子系统名称

Example:

```
// Prints: iPhone
console.log(sys.subsystem());
```

## language()
* @ret {[`String`]}

获取当前系统语言字符串,可能的值 `'en-us'`|`'zh-cn'`|`'zh-tw'`

Example:

```
// Prints: zh-Hans-CN
console.log(sys.subsystem());
```

## isWifi()
* @ret {[`bool`]}

获取当前是否为wifi网络并且网络为有效状态,如果网络为无效状态会返回`false`

## isMobile()
* @ret {[`bool`]} ms

获取当前是否为移动网络并且网络为有效状态,如果网络为无效状态会返回`false`

## networkStatus()
* @ret {[`int`]}

获取当前网络状态

@ret

* 等于`=0` - 无网络
* 等于`=1` - 有线网络
* 等于`=2` - 无线网络
* 等于或大于`>=3` - 移动网络

## isAcPower()
* @ret {[`bool`]}

获取是否连接外部电源,连接外部电源返回`true`

## isBattery()
* @ret {[`bool`]}

获取这个设备是否已连接电池设备,如果已连接电池返回`true`

## batteryLevel()
* @ret {[`float`]} `0-1`

获取电池电量等级,返回为`0-1`的浮点值,如果没有电池返回`0`

## memory()
* @ret {[`uint64`]}

获取系统内存总量,单位为字节

## usedMemory()
* @ret {[`uint64`]}

获取当前应用已经使用的内存

## availableMemory()
* @ret {[`uint64`]}

获取当前应用可用的内存

## cpuUsage()

获取CPU占用率

* @ret {[`float`]}


[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String

[`int`]: native_types.md#int
[`uint`]: native_types.md#uint
[`int16`]: native_types.md#int16
[`uint16`]: native_types.md#uint16
[`int64`]: native_types.md#int64
[`uint64`]: native_types.md#uint64
[`float`]: native_types.md#float
[`double`]: native_types.md#double
[`bool`]: native_types.md#bool

 