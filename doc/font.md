# `ngui/font`


## setDefaultFonts(fonts)

设置默认字体列表，使用字符时多个字体名称使用逗号分隔，字体必须是已经注册过的否则无效。

* @arg `fonts` {[`String`]|[`Array`]}

## defaultFontNames()

返回当前有效的默认字体列表

* @ret {[`Array`]}

## familyNames()

获取当前注册的所有字体家族名称列表

* @ret {[`Array`]}

## fontNames(family)

通过字体家族名称返回字体列表

* @arg `family` {[`String`]}
* @ret {[`Array`]}

## test(name) 

测试当前注册过的字体家族或字体

* @arg `name` {[`String`]} font name or family name
* @ret {[`bool`]}

## registerFont(fontData[,alias])

通过字体数据[`Buffer`]注册，成功返回`true`

这个方法会将比`registerFontFile()`消耗更多的内存，如果文件在本地最好使用`registerFontFile()`

* @arg `fontData` {[`Buffer`]} 字体数据
* @arg `[alias]` {[`String`]}  设置一个可选的别名
* @ret {[`bool`]}

## registerFontFile(path[,alias])

通过本地文件路径注册，成功返回`true`

注意：这个路径只能为本地文件路径，如`file:///var/data/font.ttf` 或 `/var/data/font.ttf`

* @arg `path` {[`String`]}   本地字体文件路径
* @arg `alias` {[`String`]}	 设置一个可选的别名
* @ret {[`bool`]}

## setFamilyAlias(family, alias)

设置字体家族别名

* @arg `family` {[`String`]} 当前注册的字体家族名
* @arg `alias` {[`String`]}	 不能根现有的家族与字体名称重复，但可以替换现有的别名


[`Object`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
[`Array`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array
[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
[`bool`]: native_types.md#bool
[`Buffer`]: https://nodejs.org/dist/latest-v8.x/docs/api/buffer.html
