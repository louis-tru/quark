
import { LOG, AM, M, P, VM, VP, LOG } from './test'
import { Event, EventNoticer, Notification, event } from 'flare/event';

LOG('\nTest event:\n')

var ontest = new EventNoticer('test', this)

M(ontest, 'on', [function(ev) {
	F_LOG("handle on")
	F_LOG('name', ev.name)
	F_LOG('data', ev.data)
}])

M(ontest, 'once', [function(ev) {
	F_LOG("handle once")
}])

M(ontest, '$on', [function(that, ev) {
	F_LOG("handle $on")
}])

M(ontest, '$once', [function(that, ev) {
	F_LOG("handle $once")
	ev.returnValue = 1
}])

P(ontest, 'enable')
P(ontest, 'name')
P(ontest, 'length')
M(ontest, 'trigger', [10])
P(ontest, 'length')

var id = M(ontest, 'on', [function(ev) {
	F_LOG("---------------0")
}])

function handle1(ev) {
	F_LOG("---------------1", this.t)
}

var scope = { t: 1 }

M(ontest, 'on', [handle1])
M(ontest, 'on', [handle1])
M(ontest, 'on', [handle1])
M(ontest, 'on', [handle1, scope])
M(ontest, 'on', [handle1, scope])
M(ontest, 'on', [handle1, scope])
M(ontest, 'on', [handle1, scope])

M(ontest, 'trigger', [20])
M(ontest, 'off', [handle1])
M(ontest, 'trigger')
M(ontest, 'off', [handle1])
M(ontest, 'trigger')
M(ontest, 'off', [scope])
M(ontest, 'trigger')
M(ontest, 'off', [id])
M(ontest, 'trigger')

M(ontest, 'on', [function(ev) {
	M(ontest, 'off', [scope])
}])
M(ontest, 'trigger')
M(ontest, 'trigger')
M(ontest, 'trigger')
M(ontest, 'off')
M(ontest, 'trigger')
M(ontest, 'trigger')
M(ontest, 'trigger')
M(ontest, 'trigger')

LOG('\nTest Notification: \n')

class Test extends Notification {
	event onTest;
	event onTest2;
}

var test = new Test();

test.ontest = function(ev) {
	F_LOG('onTest', ev.data)
	ev.returnValue = 200;
}

test.ontest2 = function(ev) {
	F_LOG('onTest2')
}

test.ontest.on(function(ev) {
	F_LOG('onTest.on()')
})

M(test, 'triggerTest', [100])
M(test, 'triggerTest2')
M(test, 'trigger', ['Test', 100])
M(test, 'trigger', ['Test2'])
M(test, 'removeEventListenerWithScope', [test]);
M(test, 'triggerTest', [100])
M(test, 'triggerTest2')

