# `quark/media`


## `Class: Video`
* `extends` [`Image`]

* 继承于[`Image`]需要加入到`gui`视图树中才可正常工作

* 如果只需要单独播放音频，可使用[`AudioPlayer`]

### Video.onWaitBuffer

缓冲等待时触发, 事件数据为`0.0-1.0`[`float`]数，`1.0`表示缓冲完成

### Video.onReady

数据源准备完成，可以开始播放时触发

### Video.onStartPlay

真正开始播放时触发

### Video.onError

异常时触发

### Video.onSourceEof

数据源结束时触发，但可能播放还未结束，因为缓冲区与解码缓冲中还有数据

### Video.onPause

播放暂停时触发

### Video.onResume

从暂停中恢复时触发

### Video.onStop

播放停止时触发

### Video.onSeek

`seek`时触发

### Video.autoPlay 

**Default** `true`

在数据源准备好时是否自动开始播放，`false`需要调用`start()`开始播放

* {[`bool`]}

### Get: Video.sourceStatus 

当前数据源状态

* {[`MultimediaSourceStatus`]}

### Get: Video.status 

当前播放状态

* {[`PlayerStatus`]}

### Video.mute 

是否静音

* {[`bool`]}

### Video.volume 

音频音量`0-100`之间

* {[`uint`]} `0-100`

### Video.src 

视频源地址，可以为本地路径也可以为网络路径

* {[`String`]}

### Get: Video.time 

视频当前播放的时间毫秒数

* {[`uint64`]} `ms`

### Get: Video.duration 

视频的总长度毫秒数

* {[`uint64`]} `ms`

### Get: Video.audioTrackIndex 

当前音频轨道索引

* {[`uint`]}

### Get: Video.audioTrackCount 

支持音频轨道数

* {[`uint`]}

### Video.disableWaitBuffer 

禁用等待数据缓冲，只要有数据立即呈现，不等待

* {[`bool`]}

### Video.selectAudioTrack(index)

选择音频轨道

* @arg `index` {[`uint`]} audio track index

### Video.audioTrack([index])

获取音频轨道信息，不传入`index`返回当前选择的轨道

* @arg `[track]` {[`uint`]}
* @ret {[`TrackInfo`]}

### Video.videoTrack()

获取视频轨道信息

* @ret {[`TrackInfo`]}

### Video.start()

开始播放

### Video.seek(time)

跳转到目标`time`位置

* @arg `time` {[`uint`]} `ms`  毫秒
* @ret {[`bool`]}  success return `true`

### Video.pause()

暂停播放

### Video.resume()

恢复播放

### Video.stop()

停止播放



## `Class: AudioPlayer`
* `extends` [`Notification`]

音频播放器与视频播放器的成员几乎完全相同

### AudioPlayer.onWaitBuffer
### AudioPlayer.onReady
### AudioPlayer.onStartPlay
### AudioPlayer.onError
### AudioPlayer.onSourceEof
### AudioPlayer.onPause
### AudioPlayer.onResume
### AudioPlayer.onStop
### AudioPlayer.onSeek

### AudioPlayer.constructor([src])

这里如果传入有效源`src`会自动开始播放

* @arg `[src]` {[`String`]}

### AudioPlayer.autoPlay 

* {[`bool`]}

### Get: AudioPlayer.sourceStatus 

* {[`MultimediaSourceStatus`]}

### Get: AudioPlayer.status 

* {[`PlayerStatus`]}

### AudioPlayer.mute 

* {[`bool`]}

### AudioPlayer.volume 

* {[`uint`]} `0-100`

### AudioPlayer.src 

* {[`String`]}

### Get: AudioPlayer.time 

* {[`uint64`]} `ms`

### Get: AudioPlayer.duration 

* {[`uint64`]} `ms`

### Get: AudioPlayer.trackIndex 

* {[`uint`]}

### Get: AudioPlayer.trackCount 

* {[`uint`]}

### AudioPlayer.disableWaitBuffer 

* {[`bool`]}


### AudioPlayer.selectTrack(index)

选择音播放的频轨道

* @arg `index` {[`uint`]} audio track index

### AudioPlayer.track([index])

不传入`index`默认返回当前播放的轨道信息

* @arg `[track]` {[`uint`]} 
* @ret {[`TrackInfo`]}

### AudioPlayer.start()

### AudioPlayer.seek(time)
* @arg `time` {[`uint`]} `ms`
* @ret {[`bool`]} success return `true`

### AudioPlayer.pause()
### AudioPlayer.resume()
### AudioPlayer.stop()




## `Object: TrackInfo`

多媒体轨道信息描述数据，这是个`Object`类型描述并没有实际存在的构造函数

### TrackInfo.type 

轨道类型，声音或视频

* {[`MediaType`]}

### TrackInfo.mime 

Mime类型字符串

* {[`String`]}

### TrackInfo.width 

输出图像宽度

* {[`uint`]}

### TrackInfo.height 

输出图像高度

* {[`uint`]}

### TrackInfo.language 

语言字符串

* {[`String`]}

### TrackInfo.bitrate 

码率

* {[`uint`]}

### TrackInfo.sampleRate 

声音采样率

* {[`uint`]}

### TrackInfo.channelCount 

声音声道数量

* {[`uint`]}

### TrackInfo.channelLayout 

音频声道布局掩码 [`AudioChannelMask`]

* {[`uint64`]}

### TrackInfo.frameInterval 

图像帧时间间隔 `ms`

* {[`uint`]}



## `Enum: MediaType`
### MEDIA_TYPE_AUDIO
### MEDIA_TYPE_VIDEO

## `Enum: PlayerStatus`
### PLAYER_STATUS_STOP

* 播放已停止

### PLAYER_STATUS_START

* 已调用`start()`开始播放，但还未真正开始播放

### PLAYER_STATUS_PLAYING

* 正在播放中状态

### PLAYER_STATUS_PAUSED

* 暂停状态

## `Enum: MultimediaSourceStatus`
### MULTIMEDIA_SOURCE_STATUS_UNINITIALIZED

* 还未初始化媒体数据源

### MULTIMEDIA_SOURCE_STATUS_READYING

* 数据源正在准备中

### MULTIMEDIA_SOURCE_STATUS_READY

* 数据源已准备好，数据源正常工作时大部分时间都是这个状态

### MULTIMEDIA_SOURCE_STATUS_WAIT

* 数据源正在等待数据缓冲

### MULTIMEDIA_SOURCE_STATUS_FAULT

* 数据源故障

### MULTIMEDIA_SOURCE_STATUS_EOF

* 数据源已读取结束


## `Enum: AudioChannelMask`
### CH_INVALID
### CH_FRONT_LEFT
### CH_FRONT_RIGHT
### CH_FRONT_CENTER
### CH_LOW_FREQUENCY
### CH_BACK_LEFT
### CH_BACK_RIGHT
### CH_FRONT_LEFT_OF_CENTER
### CH_FRONT_RIGHT_OF_CENTER
### CH_BACK_CENTER
### CH_SIDE_LEFT
### CH_SIDE_RIGHT
### CH_TOP_CENTER
### CH_TOP_FRONT_LEFT
### CH_TOP_FRONT_CENTER
### CH_TOP_FRONT_RIGHT
### CH_TOP_BACK_LEFT
### CH_TOP_BACK_CENTER
### CH_TOP_BACK_RIGHT

## `Enum: VideoColorFormat`
### VIDEO_COLOR_FORMAT_INVALID
### VIDEO_COLOR_FORMAT_YUV420P
### VIDEO_COLOR_FORMAT_YUV420SP
### VIDEO_COLOR_FORMAT_YUV411P
### VIDEO_COLOR_FORMAT_YUV411SP


[`Class`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Classes
[`Object`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
[`Array`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array
[`Function`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Function
[`Date`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
[`RegExp`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp
[`ArrayBuffer`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer
[`TypedArray`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray
[`String`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
[`Number`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number
[`Boolean`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
[`null`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/null
[`undefined`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/undefined

[`int`]: native_types.md#int
[`uint`]: native_types.md#uint
[`int16`]: native_types.md#int16
[`uint16`]: native_types.md#uint16
[`int64`]: native_types.md#int64
[`uint64`]: native_types.md#uint64
[`float`]: native_types.md#float
[`double`]: native_types.md#double
[`bool`]: native_types.md#bool

[`Image`]: quark.md#class-image
[`MediaType`]: media.md#enum-mediatype
[`TrackInfo`]: media.md#object-trackinfo
[`PlayerStatus`]: media.md#enum-playerstatus
[`MultimediaSourceStatus`]: media.md#enum-multimediasourcestatus
[`AudioChannelMask`]: media.md#enum-audiochannelmask
[`Notification`]: event.md#class-notification
[`AudioPlayer`]: media.md#class-audioplayer
