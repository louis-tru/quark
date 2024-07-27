
import { LOG, Mv, Ca } from './tool'
import * as fs from 'quark/fs'
import path from 'quark/path';

// init

const DIR = path.documents('test');
const DIR2 = path.documents('test2');
const FILE = DIR + '/test_file.txt';

function stat(path: string) {
	return fs.statSync(path);
}

async function async_test() {
	LOG('\nFileHelper:\n');

	fs.removerSync(DIR);
	fs.removerSync(DIR2);
	fs.mkdirpSync(DIR);
	fs.writeFileSync(FILE, 'ABCDEFG');

	let s = stat(FILE);

	await Mv(fs, 'chmodr', [DIR, s.mode()+1]);
	Mv(stat(FILE), 'mode', [], i=>i==s.mode()+1);

	Mv(fs, 'chmodrSync', [DIR, s.mode() + 2 ]);
	Mv(stat(FILE), 'mode', [], i=>i==s.mode()+2);

	await Mv(fs, 'chownr', [DIR, s.owner(), s.group()]);
	Mv(stat(DIR), 'owner', [], i=>i==s.owner());
	Mv(stat(DIR), 'group', [], i=>i==s.group());

	Mv(fs, 'chownrSync', [DIR, s.owner(), s.group()]);
	Mv(stat(DIR), 'owner', [], i=>i==s.group());
	Mv(stat(DIR), 'group', [], i=>i==s.group());

	await Mv(fs, 'mkdirp', [DIR + '/b/c']);
	Mv(fs, 'existsSync', [DIR + '/b/c'], true);

	Mv(fs, 'mkdirpSync', [DIR + '/b_sync/c']);
	Mv(fs, 'existsSync', [DIR + '/b_sync/c'], true);

	fs.writeFileSync(FILE + '/b/b.txt', 'ABCDEFG');
	await Mv(fs, 'remover', [DIR + '/b']);
	Mv(fs, 'existsSync', [DIR + '/b'], false);
	Mv(fs, 'existsSync', [DIR + '/b/b.txt'], false);

	fs.writeFileSync(FILE + '/c_sync/c.txt', 'ABCDEFG');
	Mv(fs, 'removerSync', [DIR + '/c_sync']);
	Mv(fs, 'existsSync', [DIR + '/c_sync'], false);
	Mv(fs, 'existsSync', [DIR + '/c_sync/c.txt'], false);
	
	await Mv(fs, 'copy', [FILE, DIR + '/cp.txt']);
	Mv(fs, 'existsSync', [DIR + '/cp.txt'], true);

	Mv(fs, 'copySync', [FILE, DIR + '/cp_sync.txt']);
	Mv(fs, 'existsSync', [DIR + '/cp_sync.txt'], true);

	await Mv(fs, 'copyr', [DIR, DIR2]);
	Mv(fs, 'existsSync', [DIR2], true);
	Mv(fs, 'existsSync', [DIR2 + '/cp.txt'], true);

	Mv(fs, 'copyrSync', [DIR, DIR2 + '_sync']);
	Mv(fs, 'existsSync', [DIR2 + '_sync'], true);
	Mv(fs, 'existsSync', [DIR2 + '_sync' + '/cp.txt'], true);

	await Mv(fs, 'readdir', [DIR]);
	Mv(fs, 'readdirSync', [DIR]);

	await Mv(fs, 'isFile', [FILE], true);
	Mv(fs, 'isFileSync', [FILE], true);
	await Mv(fs, 'isDirectory', [DIR], true);
	Mv(fs, 'isDirectorySync', [DIR], true);
}

Ca(async_test)