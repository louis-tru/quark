
import { P, M, LOG, AM } from './test'
import 'noug/fs';
import 'noug/reader';
import 'noug/url';

const DIR = url.documents('test');
const FILE = DIR + '/test_file.txt';
const URL = 'https://github.com/';

async function async_test() {
	
	M(fs, 'mkdirSyncP', [DIR]);
	M(fs, 'writeFileSync', [FILE, 'ABCDEFG']);
	
	LOG('\nreader:\n');

	await AM(reader, 'readStream', [FILE, d=>d.complete]);
	await AM(reader, 'readFile', [FILE, b=>1]);
	M(reader, 'readFileSync', [FILE]);
	M(reader, 'existsSync', [FILE]);
	M(reader, 'abort', [reader.readFile(FILE,()=>{ LOG('** abort read fail **') })]);
	M(reader, 'abort', [reader.readStream(FILE,()=>{ LOG('** abort read_stream fail **') })]);
	M(reader, 'abort', [reader.readStream(URL,()=>{ LOG('** abort read_stream url fail **') })]);
	await AM(reader, 'readStream', [URL, d=>d.complete]);
	await AM(reader, 'readFile', [URL, b=>1]);
	M(reader, 'readFileSync', [URL]);
	M(reader, 'existsSync', [URL], false);
}

async_test().catch(function(err) {
	LOG('Error:', err.message);
	LOG(err.stack);
});

ajs_set_method(abort, abort);
ajs_set_method(chmodSyncR, chmod_r<true>);
ajs_set_method(chownSyncR, chown_r<true>);
ajs_set_method(mkdirSyncP, mkdir_p<true>);
ajs_set_method(removeSyncR, rm_r<true>);
ajs_set_method(copySync, cp<true>);
ajs_set_method(copySyncR, cp_r<true>);
ajs_set_method(readdirSync, ls<true>);
ajs_set_method(isFileSync, exists_file<true>);
ajs_set_method(isDirectorySync, exists_dir<true>);
ajs_set_method(mkdirP, mkdir_p<false>);
ajs_set_method(readdir, ls<false>);
ajs_set_method(isFile, exists_file<false>);
ajs_set_method(isDirectory, exists_dir<false>);
ajs_set_method(chmodR, chmod_r<false>);
ajs_set_method(chownR, chown_r<false>);
ajs_set_method(removeR, rm_r<false>);
ajs_set_method(copy, cp<false>);
ajs_set_method(copyR, cp_r<false>);
