
import 'qgr/keys';
import 'qgr/fs';
import 'qgr/url';
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

uu.time('init');

var str = keys.stringify(json);

uu.time('keys.stringify(json)');

fs.writeFileSync(url.documents('benchmark-keys.keys'), str);

var json2 = keys.parseFile(url.documents('benchmark-keys.keys'));

uu.time('keys.parseFile(benchmark-keys.keys)');
