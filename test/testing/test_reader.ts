
import { LOG, Mv } from './tool'
import * as fs from 'quark/fs';
import path from 'quark/path';

export default async function(_: any) {
	const reader = fs.reader;
	const DIR = path.documents('test');
	const FILE = DIR + '/test_file.txt';
	const URL = 'https://github.com/';
	
	Mv(fs, 'mkdirsSync', [DIR]);
	Mv(fs, 'writeFileSync', [FILE, 'ABCDEFG']);

	LOG('\nreader:\n');

	await Mv(reader, 'readFile', [URL, 'utf8']);
	await Mv(reader, 'readStream', [URL, e=>e.end]);

	Mv(reader.readStream(FILE, e=>{}), 'abort', []);
	Mv(reader.readStream(URL, e=>{}), 'abort', []);

	Mv(reader, 'readFileSync', [FILE, 'utf8']);
	Mv(reader, 'existsSync', [FILE], true);
	Mv(reader, 'isFileSync', [FILE], true);
	Mv(reader, 'isDirectorySync', [FILE], false);
	Mv(reader, 'readdirSync', [FILE]);
	Mv(reader.readFile(FILE,'utf8'), 'abort', []);
	Mv(reader, 'clear', []);
}