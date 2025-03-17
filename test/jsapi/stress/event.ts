
import { EventNoticer } from 'quark/event';
import * as uu from './uu';

uu.start();

var onTest = new EventNoticer('Test', {});

uu.log(1);

for (var i = 0; i < 1000000; i++) {
	onTest.on(function() {
		// noop
	}, String(i));
}

uu.log(2);

for (var i = 1000000; i < 2000000; i++) {
	onTest.once(function() {
		// noop
	}, String(i));
}

uu.log(3);

onTest.trigger(0);

uu.log(4);

for (var i = 0; i < 1000000; i++) {
	onTest.off(String(i));
}

uu.log(5);

onTest.trigger(0);

uu.log(6);
