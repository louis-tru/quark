
import 'ngui/keys';
import 'ngui/fs';
import 'ngui/path';
import './uu';

uu.start();

var json = [];

for (var i = 0; i < 10000; i++) {
	json.push({
		a: 100,
		b: { a: 'abcdefg', c: 90.9 },
		c: { ll: 'aaaaa' },
		d: [ '0', '1', 2 ],
	});
}

uu.log('init');

var str = keys.stringify(json);

uu.log('keys.stringify(json)');

fs.writeFileSync(path.documents('benchmark-keys.keys'), str);

var json2 = keys.parseFile(path.documents('benchmark-keys.keys'));

uu.log('keys.parseFile(benchmark-keys.keys)');
