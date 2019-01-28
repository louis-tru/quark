
import 'qgr/storage';
import './uu';

storage.transaction(function() {

	uu.start();

	for (var i = 0; i < 200000; i++) {
		storage.set(i, 'localStorage_' + i);
	}

	uu.log();

	for (var i = 0; i < 200000; i++) {
		storage.get(i);
	}

	uu.log();

	for (var i = 0; i < 200000; i++) {
		storage.del(i);
	}

	storage.clear();

	uu.log();

});
