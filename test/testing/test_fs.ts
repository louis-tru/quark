
import { LOG, Mv } from './tool'
import * as fs from 'quark/fs'
import path from 'quark/path';

function stat(path: string) {
	return fs.statSync(path);
}

export default async function(_: any) {
	LOG('\nFileHelper:\n');

	const A = path.documents('test/a');
	const B = path.documents('test/b');
	const FILE = path.documents('test/a/file.txt');

	fs.removeRecursionSync(A);
	fs.removeRecursionSync(B);
	fs.mkdirsSync(A);
	fs.writeFileSync(FILE, 'ABCDEFG');

	let s = stat(FILE);

	{
		await Mv(fs, 'chmodRecursion', [A, s.mode()+1]);
		Mv(stat(FILE), 'mode', [], i=>i==s.mode()+1);

		Mv(fs, 'chmodRecursionSync', [A, s.mode() + 2 ]);
		Mv(stat(FILE), 'mode', [], i=>i==s.mode()+2);

		await Mv(fs, 'chownRecursion', [A, s.owner(), s.group()]);
		Mv(stat(A), 'owner', [], i=>i==s.owner());
		Mv(stat(A), 'group', [], i=>i==s.group());

		Mv(fs, 'chownRecursionSync', [A, s.owner(), s.group()]);
		Mv(stat(A), 'owner', [], i=>i==s.owner());
		Mv(stat(A), 'group', [], i=>i==s.group());
	}

	{
		await Mv(fs, 'mkdirs', [A + '/0/1']);
		Mv(fs, 'existsSync', [A + '/0/1'], true);

		fs.writeFileSync(A + '/0/b.txt', 'ABCDEFG');
		await Mv(fs, 'removeRecursion', [A + '/0']);
		Mv(fs, 'existsSync', [A + '/0'], false);
		Mv(fs, 'existsSync', [A + '/0/b.txt'], false);
	}

	{
		Mv(fs, 'mkdirsSync', [A + '/sync/f']);
		Mv(fs, 'existsSync', [A + '/sync/f'], true);

		fs.writeFileSync(A + '/sync/c.txt', 'ABCDEFG');
		Mv(fs, 'removeRecursionSync', [A + '/sync']);
		Mv(fs, 'existsSync', [A + '/sync'], false);
		Mv(fs, 'existsSync', [A + '/sync/c.txt'], false);
	}

	await Mv(fs, 'copy', [FILE, A + '/cp.txt']);
	Mv(fs, 'existsSync', [A + '/cp.txt'], true);

	Mv(fs, 'copySync', [FILE, A + '/cp_sync.txt']);
	Mv(fs, 'existsSync', [A + '/cp_sync.txt'], true);

	await Mv(fs, 'copyRecursion', [A, B]);
	Mv(fs, 'existsSync', [B], true);
	Mv(fs, 'existsSync', [B + '/cp.txt'], true);

	Mv(fs, 'copyRecursionSync', [A, B + '_sync']);
	Mv(fs, 'existsSync', [B + '_sync'], true);
	Mv(fs, 'existsSync', [B + '_sync' + '/cp.txt'], true);

	await Mv(fs, 'readdir', [A]);
	Mv(fs, 'readdirSync', [A]);

	await Mv(fs, 'isFile', [FILE], true);
	Mv(fs, 'isFileSync', [FILE], true);
	await Mv(fs, 'isDirectory', [A], true);
	Mv(fs, 'isDirectorySync', [A], true);
}