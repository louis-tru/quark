
import { EventNoticer } from 'quark/event';
import * as uu from './uu';

uu.start();

var ontest = new EventNoticer('test', this);

uu.log(1);

for (var i = 0; i < 1000000; i++) {
	ontest.on(function() {
		// noop
	}, String(i));
}

uu.log(2);

for (var i = 1000000; i < 2000000; i++) {
	ontest.once(function() {
		// noop
	}, String(i));
}

uu.log(3);

ontest.trigger();

uu.log(4);

for (var i = 0; i < 1000000; i++) {
	ontest.off(String(i));
}

uu.log(5);

ontest.trigger();

uu.log(6);
