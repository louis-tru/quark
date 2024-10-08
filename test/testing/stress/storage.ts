
import * as storage from 'quark/storage';
import * as uu from './uu';

uu.start();

for (var i = 0; i < 200000; i++) {
	storage.set(String(i), 'localStorage_' + i);
}

uu.log();

for (var i = 0; i < 200000; i++) {
	storage.get(String(i));
}

uu.log();

for (var i = 0; i < 200000; i++) {
	storage.remove(String(i));
}

storage.clear();

uu.log();
