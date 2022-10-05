#!/usr/bin/env node

import util from 'somes';
import fs = require('somes/fs');
import QuarkBuild from './build';
import QuarkExport from './export';
import server from './server';
import * as argument from 'somes/arguments';

const args = process.argv.slice(2);
const cmd = args.shift();
const opts = argument.options;
const help_info = argument.helpInfo;
const def_opts = argument.defOpts;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts(['port', 'p'], 1026, 	'--port=PORT,-p PORT Run quark debugger server port [{0}]');
// def_opts(['remote', 'r'], '', 	'--remote=ADDRESS,-r ADDRESS Remote console address [none]');

if ( opts.help || opts.h /*cmd == 'help' || cmd == 'h'*/ ) {
	console.log('');
	console.log('Usage: noproj COMMAND [OS]');
	console.log('Usage: noproj [OPTION]...');
	console.log('');
	console.log('Examples:');
	console.log('`noproj init`');
	console.log('`noproj build`');
	console.log('`noproj rebuild`');
	console.log('`noproj export ios`');
	console.log('`noproj export android`');
	console.log('`noproj install`');
	console.log('`noproj clear`');
	console.log('`noproj`');
	// console.log('`noproj -r http://192.168.1.124:1026`');
	console.log('');
	console.log('Defaults for the options are specified in brackets.');
	console.log('');
	console.log('Options:');
	console.log('  ' + help_info.join('\n  '));
	console.log('');
}
else if ( cmd == 'export' ) {
	util.assert(args.length, 'export Bad argument. system name required, for example "noproj export ios"');
	new QuarkExport(process.cwd(), args[0]).export().catch(e=>console.error(e));
} 
else if ( cmd == 'build' || cmd == 'rebuild' || cmd == 'init' ) {
	if ( cmd == 'rebuild' ) {
		fs.rm_r_sync(process.cwd() + '/out/install');
		fs.rm_r_sync(process.cwd() + '/out/libs');
		fs.rm_r_sync(process.cwd() + '/out/public');
	}
	var build = new QuarkBuild(process.cwd(), process.cwd() + '/out');
	if ( cmd == 'init' ) {
		build.initialize();
	} else {
		build.build().catch(e=>console.error(e));
	}
}
else if (cmd == 'install') {
	new QuarkBuild(process.cwd(), process.cwd() + '/out').install_depe();
} 
else if ( cmd == 'clear' ) {
	fs.rm_r_sync(process.cwd() + '/out');
}
else {
	// run wrb server
	server(argument.options);
}

export default {};