
import { LOG, Pv, Mv } from './tool'
import event, { Event, EventNoticer, Notification } from 'quark/event'

export default async function (_: any) {
	LOG('\nTest event:\n')

	var onTest = new EventNoticer('Test', { t: 'Start 0' })

	Mv(onTest, 'on', [function(e) {
		LOG("handle on")
		LOG('data', e.data)
	}])

	Mv(onTest, 'once', [function(e) {
		LOG("handle once")
	}])

	Mv(onTest, 'on2', [function(that, e) {
		LOG("handle on2")
	}])

	Mv(onTest, 'once2', [function(that, e) {
		LOG("handle once2")
		e.returnValue = 1
	}])

	Pv(onTest, 'name', 'Test')
	Pv(onTest, 'length', 4)
	Mv(onTest, 'trigger', [10])
	Pv(onTest, 'length', 2)

	const id = Mv(onTest, 'on', [function(e) {
		LOG("---------------0")
	}])

	const scope = { t: 'Start 1' }

	function handle1(this: any, e: Event) {
		LOG("---------------1", this.t)
	}

	Mv(onTest, 'on', [handle1])
	Mv(onTest, 'on', [handle1])
	Mv(onTest, 'on', [handle1])
	Mv(onTest, 'on', [handle1, scope])
	Mv(onTest, 'on', [handle1, scope])
	Mv(onTest, 'on', [handle1, scope])
	Mv(onTest, 'on', [handle1, scope])

	Mv(onTest, 'trigger', [20])
	Mv(onTest, 'off', [handle1])
	Mv(onTest, 'trigger', [0])
	Mv(onTest, 'off', [handle1])
	Mv(onTest, 'trigger', [1])
	Mv(onTest, 'off', [scope])
	Mv(onTest, 'trigger', [2])
	Mv(onTest, 'off', [id])
	Mv(onTest, 'trigger', [3])

	Mv(onTest, 'on', [function(e) {
		Mv(onTest, 'off', [scope])
	}])
	Mv(onTest, 'trigger', [1])
	Mv(onTest, 'trigger', [2])
	Mv(onTest, 'trigger', [3])
	Mv(onTest, 'off', [])
	Mv(onTest, 'trigger', [4])
	Mv(onTest, 'trigger', [5])
	Mv(onTest, 'trigger', [6])
	Mv(onTest, 'trigger', [7])

	LOG('\nTest Notification: \n')

	class Test extends Notification {
		@event readonly onTest: EventNoticer<Event>;
		@event readonly onTest2: EventNoticer<Event>;
	}

	const test = new Test();

	test.addDefaultListener('Test', function(e) {
		LOG('trigget onTest', e.data)
		e.returnValue = 200;
	})

	test.addDefaultListener('Test2', function(e) {
		LOG('trigget  onTest2')
	})

	test.onTest.on(function(e) {
		LOG('-------------- onTest.on() --------------')
	})

	Mv(test, 'trigger', ['Test', 100])
	Mv(test, 'trigger', ['Test2'])
	Mv(test, 'removeEventListenerWithCtx', [test])
	Mv(test, 'trigger', ['Test', 100])
	Mv(test, 'trigger', ['Test2'])
}