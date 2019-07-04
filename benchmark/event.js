
import { EventNoticer } from 'langou/event';
import './uu';

uu.start();

var ontest = new EventNoticer('test', this);

uu.time(1);

for (var i = 0; i < 1000000; i++) {
	ontest.on(function() {
		// noop
	}, i);
}

uu.time(2);

for (var i = 1000000; i < 2000000; i++) {
	ontest.once(function() {
		// noop
	}, i);
}

uu.time(3);

ontest.trigger();

uu.time(4);

for (var i = 0; i < 1000000; i++) {
	ontest.off(i);
}

uu.time(5);

ontest.trigger();

uu.time(6);
