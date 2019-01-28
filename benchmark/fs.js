
import 'qgr/fs';
import 'qgr/url';
import './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 1000; j++) {
	fs.copySyncR(path, path + '.' + j);
}

uu.time();

for (var j = 0; j < 1000; j++) {
	fs.copyR(path, path + '.' + j, function(err) {
		if (err) {
			console.log('copy err', ++i);
		}
	});
}

var stat = fs.statSync(path);

for (var j = 0; j < 1000; j++) {
	fs.chmodR(path + '.' + j, stat.mode, function(err) {
		if (err) {
			console.log('chmodR err', ++i);
		}
	});
}

for (var j = 0; j < 1000; j++) {
	fs.chownR(path + '.' + j, stat.uid, stat.gid, function(err) {
		if (err) {
			console.log('chownR err', ++i);
		}
	});
}

for (var j = 0; j < 1000; j++) {
	fs.isFile(path + '.' + j, function(err, ok) {
		if (err) {
			console.log('isFile err', ++i);
		}
	});
}

uu.time();
