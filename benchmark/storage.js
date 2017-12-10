
import 'ngui/storage';
import './print';

storage.transaction(function() {

	print.start();

	for (var i = 0; i < 200000; i++) {
		storage.set(i, 'localStorage_' + i);
	}

	print.time();

	for (var i = 0; i < 200000; i++) {
		storage.get(i);
	}

	print.time();

	for (var i = 0; i < 200000; i++) {
		storage.del(i);
	}

	storage.clear();

	print.time();

});
