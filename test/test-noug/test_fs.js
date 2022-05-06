
import { P, M, LOG, AM, VM, VP, CA } from './test'
import 'noug/fs';
import 'noug/url';
import { Buffer } from 'buffer';

// init

const DIR = url.documents('test');
const DIR2 = url.documents('test2');
const FILE = DIR + '/test_file.txt';
const FILE2 = DIR + '/test_file2.txt';

// start

function stat(path) {
	return fs.statSync(path);
}

async function async_test() {
	LOG('\nFileHelper:\n');

	fs.removeSyncR(DIR);
	fs.removeSyncR(DIR2);
	fs.mkdirSyncP(DIR);
	fs.writeFileSync(FILE, 'ABCDEFG');

	var s = stat(FILE);

	await AM(fs, 'chmodr', [DIR, s.mode + 1, ()=>true ]);
	VM(stat(FILE), 'mode', [], i=>i==s.mode+1);

	M(fs, 'chmodrSync', [DIR, s.mode + 2 ]);
	VM(stat(FILE), 'mode', [], i=>i==s.mode+2);

	await AM(fs, 'chownr', [DIR, s.uid, s.gid, ()=>true ]);
	VM(stat(DIR), 'owner', [], i=>i==s.uid);
	VM(stat(DIR), 'group', [], i=>i==s.gid);

	M(fs, 'chownrSync', [DIR, s.uid, s.gid ]);
	VM(stat(DIR), 'owner', [], i=>i==s.uid);
	VM(stat(DIR), 'group', [], i=>i==s.gid);

	await AM(fs, 'mkdirp', [DIR + '/b/c', ()=>true]);
	VM(fs, 'existsSync', [DIR + '/b/c']);

	M(fs, 'mkdirpSync', [DIR + '/b_sync/c']);
	VM(fs, 'existsSync', [DIR + '/b_sync/c']);
	
	fs.writeFileSync(FILE + '/b/b.txt', 'ABCDEFG');
	await AM(fs, 'remover', [DIR + '/b', ()=>1]);
	VM(fs, 'existsSync', [DIR + '/b'], false);
	VM(fs, 'existsSync', [DIR + '/b/b.txt'], false);
	
	fs.writeFileSync(FILE + '/c_sync/c.txt', 'ABCDEFG');
	M(fs, 'removerSync', [DIR + '/c_sync']);
	VM(fs, 'existsSync', [DIR + '/c_sync'], false);
	VM(fs, 'existsSync', [DIR + '/c_sync/c.txt'], false);
	
	await AM(fs, 'copy', [FILE, DIR + '/cp.txt', ()=>1]);
	VM(fs, 'existsSync', [DIR + '/cp.txt']);

	M(fs, 'copySync', [FILE, DIR + '/cp_sync.txt']);
	VM(fs, 'existsSync', [DIR + '/cp_sync.txt']);
	
	await AM(fs, 'copyr', [DIR, DIR2, ()=>1]);
	VM(fs, 'existsSync', [DIR2]);
	VM(fs, 'existsSync', [DIR2 + '/cp.txt']);
	
	M(fs, 'copyrSync', [DIR, DIR2 + '_sync']);
	VM(fs, 'existsSync', [DIR2 + '_sync']);
	VM(fs, 'existsSync', [DIR2 + '_sync' + '/cp.txt']);

	await AM(fs, 'readdir', [DIR, d=>1]);
	M(fs, 'readdirSync', [DIR]);

	await AM(fs, 'isFile', [FILE, i=>1], true);
	M(fs, 'isFileSync', [FILE], true);
	await AM(fs, 'isDirectory', [DIR, i=>1], true);
	M(fs, 'isDirectorySync', [DIR], true);

}

CA(async_test)