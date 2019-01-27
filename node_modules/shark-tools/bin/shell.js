#!/usr/bin/env node

var util = require('shark-utils');
var fs = require('shark-utils/fs');
var { SharkBuild } = require('../toolchain/build');
var { SharkExport } = require('../toolchain/export');
var server = require('../toolchain/server');
var arguments = require('shark-utils/arguments');
var args = process.argv.slice(2);
var cmd = args.shift();
var opts = arguments.options;
var help_info = arguments.helpInfo;
var def_opts = arguments.defOpts;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts(['port', 'p'], 1026, 	'--port=PORT,-p PORT Run shark debugger server port [{0}]');
def_opts(['remote', 'r'], '', 	'--remote=ADDRESS,-r ADDRESS Remote console address [none]');

if ( opts.help || opts.h /*cmd == 'help' || cmd == 'h'*/ ) { 
	console.log('');
	console.log('Usage: shark COMMAND [OS]');
	console.log('Usage: shark [OPTION]...');
	console.log('');
	console.log('Examples:');
	console.log('`shark init`');
	console.log('`shark build`');
	console.log('`shark rebuild`');
	console.log('`shark export ios`');
	console.log('`shark export android`');
	console.log('`shark clear`');
	console.log('`shark`');
	console.log('`shark -r http://192.168.1.124:1026`');
	console.log('');
	console.log('Defaults for the options are specified in brackets.');
	console.log('');
	console.log('Options:');
	console.log('  ' + help_info.join('\n  '));
	console.log('');
	return;
} 
else if ( cmd == 'export' ) {
	util.assert(args.length, 'export Bad argument. system name required, for example "shark export ios"');
	new SharkExport(process.cwd(), args[0]).export();
} 
else if ( cmd == 'build' || cmd == 'rebuild' || cmd == 'init' ) {
	if ( cmd == 'rebuild' ) {
		fs.rm_r_sync(process.cwd() + '/out/install');
		fs.rm_r_sync(process.cwd() + '/out/libs');
		fs.rm_r_sync(process.cwd() + '/out/public');
	}
	var build = new SharkBuild(process.cwd(), process.cwd() + '/out');
	if ( cmd == 'init' ) {
		build.initialize();
	} else {
		build.build();
	}
} 
else if ( cmd == 'clear' ) {
	fs.rm_r_sync(process.cwd() + '/out');
}
else {
	// run wrb server
	server.start_server(arguments.options);
}
