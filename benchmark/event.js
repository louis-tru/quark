
import EventNoticer from 'ngui/event';
import './print';

print.start();

var ontest = new EventNoticer('test', this);

print.time(1);

for (var i = 0; i < 1000000; i++) {
	ontest.on(function() {
		// noop
	}, i);
}

print.time(2);

for (var i = 1000000; i < 2000000; i++) {
	ontest.once(function() {
		// noop
	}, i);
}

print.time(3);

ontest.trigger();

print.time(4);

for (var i = 0; i < 1000000; i++) {
	ontest.off(i);
}

print.time(5);

ontest.trigger();

print.time(6);
