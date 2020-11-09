#!/usr/bin/env node

import util from 'somes';
import fs = require('somes/fs');
import FtrBuild from './build';
import FtrExport from './export';
import server from './server';
import * as argument from 'somes/arguments';

const args = process.argv.slice(2);
const cmd = args.shift();
const opts = argument.options;
const help_info = argument.helpInfo;
const def_opts = argument.defOpts;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts(['port', 'p'], 1026, 	'--port=PORT,-p PORT Run ftr debugger server port [{0}]');
// def_opts(['remote', 'r'], '', 	'--remote=ADDRESS,-r ADDRESS Remote console address [none]');

if ( opts.help || opts.h /*cmd == 'help' || cmd == 'h'*/ ) {
	console.log('');
	console.log('Usage: ftrp COMMAND [OS]');
	console.log('Usage: ftrp [OPTION]...');
	console.log('');
	console.log('Examples:');
	console.log('`ftrp init`');
	console.log('`ftrp build`');
	console.log('`ftrp rebuild`');
	console.log('`ftrp export ios`');
	console.log('`ftrp export android`');
	console.log('`ftrp install`');
	console.log('`ftrp clear`');
	console.log('`ftrp`');
	// console.log('`ftrp -r http://192.168.1.124:1026`');
	console.log('');
	console.log('Defaults for the options are specified in brackets.');
	console.log('');
	console.log('Options:');
	console.log('  ' + help_info.join('\n  '));
	console.log('');
}
else if ( cmd == 'export' ) {
	util.assert(args.length, 'export Bad argument. system name required, for example "ftrp export ios"');
	new FtrExport(process.cwd(), args[0]).export().catch(e=>console.error(e));
} 
else if ( cmd == 'build' || cmd == 'rebuild' || cmd == 'init' ) {
	if ( cmd == 'rebuild' ) {
		fs.rm_r_sync(process.cwd() + '/out/install');
		fs.rm_r_sync(process.cwd() + '/out/libs');
		fs.rm_r_sync(process.cwd() + '/out/public');
	}
	var build = new FtrBuild(process.cwd(), process.cwd() + '/out');
	if ( cmd == 'init' ) {
		build.initialize();
	} else {
		build.build().catch(e=>console.error(e));
	}
}
else if (cmd == 'install') {
	new FtrBuild(process.cwd(), process.cwd() + '/out').install_depe();
} 
else if ( cmd == 'clear' ) {
	fs.rm_r_sync(process.cwd() + '/out');
}
else {
	// run wrb server
	server(argument.options);
}

export default {};