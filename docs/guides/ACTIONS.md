# Actions and keyframes

[中文](ACTIONS-cn.md) · [All guides](README.md) · [API index](https://quarks.cc/doc/)

Quark's Action system drives animations independently from layout and
rendering. An Action changes supported properties on its bound View over time;
the View receives those values and the normal layout/render pipeline displays
the result.

Keyframe transitions support cubic Bézier curves and the built-in `linear`,
`ease`, `easeIn`, `easeOut`, and `easeInOut` curves.

## Basic animation

This example moves a `Box` from `x = 0` to `x = 100` in one second:

```ts
import { Application, Window, Morph, KeyframeAction } from 'quark'

const app = new Application()
const win = new Window()
const view = new Morph(win)
const action = new KeyframeAction(win)

action.add({ x: 0, time: 0 })
action.add({ x: 100, time: 1000 })

view.width = 50
view.height = 50
view.backgroundColor = '#f00'
view.action = action
view.appendTo(win.root)

action.loop = 1000
action.play()
```

## Action types

The public Action class hierarchy is:

* [Action] — abstract base class
	* [SpawnAction] — runs child actions in parallel
	* [SequenceAction] — runs child actions in sequence
	* [KeyframeAction] — animates View style properties through keyframes

`Action` provides playback and composition operations including `play()`,
`stop()`, `seek()`, `seekPlay()`, `seekStop()`, `append()`, and `clear()`.

### SpawnAction

A `SpawnAction` runs its child actions in parallel. Its duration is determined
by the longest child; shorter children remain at their completed state until
the group finishes or starts another loop.

### SequenceAction

A `SequenceAction` runs its child actions one after another. After the final
child completes, the sequence ends or begins its next configured loop.

## KeyframeAction and Keyframe

`KeyframeAction` contains [Keyframe] objects. `Keyframe` extends `StyleSheets`,
so its properties use the same names and value types as View styles. A property
only has an effect when the bound View supports it: for example, `x` applies to
a transform-capable View such as `Morph`, while `width` applies to `Box` and its
subclasses.

```ts
// Valid: Box supports width and height.
const boxAction = new KeyframeAction(win)
const box = new Box(win)
box.backgroundColor = '#f00'
boxAction.add({ width: 10, height: 10 })
boxAction.add({ width: 100, height: 100, time: 1000 })
box.action = boxAction
boxAction.play()

// View does not expose width or height, so these properties have no effect.
const viewAction = new KeyframeAction(win)
const view = new View(win)
viewAction.add({ width: 10, height: 10 })
viewAction.add({ width: 100, height: 100, time: 1000 })
view.action = viewAction
viewAction.play()
```

For declarative construction, [createAction] accepts a keyframe array or an
object containing `keyframe`, `spawn`, or `seq` children.

## Action events

Views expose two Action events:

- `View.onActionKeyframe` fires when an action reaches a keyframe. Because the
  event is delivered on a rendered frame, it may be slightly early or late;
  `ActionEvent.delay` contains the difference from the target time.
- `View.onActionLoop` fires when a new loop begins. It does not fire for the
  initial run. The event also reports `delay` and the current `looped` count.

The event's `action` and `frame` properties identify the Action and keyframe
that produced it.

[Action]: https://quarks.cc/doc/action.html#class-action
[SpawnAction]: https://quarks.cc/doc/action.html#class-spawnaction
[SequenceAction]: https://quarks.cc/doc/action.html#class-sequenceaction
[KeyframeAction]: https://quarks.cc/doc/action.html#class-keyframeaction
[Keyframe]: https://quarks.cc/doc/action.html#class-keyframe
[createAction]: https://quarks.cc/doc/action.html#createaction-win-arg-parent-
