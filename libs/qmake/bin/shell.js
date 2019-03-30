#!/usr/bin/env node

var util = require('qkit');
var fs = require('qkit/fs');
var { QgrBuild } = require('../build');
var { QgrExport } = require('../export');
var server = require('../server');
var arguments = require('qkit/arguments');
var args = process.argv.slice(2);
var cmd = args.shift();
var opts = arguments.options;
var help_info = arguments.helpInfo;
var def_opts = arguments.defOpts;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts(['port', 'p'], 1026, 	'--port=PORT,-p PORT Run qgr debugger server port [{0}]');
def_opts(['remote', 'r'], '', 	'--remote=ADDRESS,-r ADDRESS Remote console address [none]');

if ( opts.help || opts.h /*cmd == 'help' || cmd == 'h'*/ ) { 
	console.log('');
	console.log('Usage: qgr COMMAND [OS]');
	console.log('Usage: qgr [OPTION]...');
	console.log('');
	console.log('Examples:');
	console.log('`qgr init`');
	console.log('`qgr build`');
	console.log('`qgr rebuild`');
	console.log('`qgr export ios`');
	console.log('`qgr export android`');
	console.log('`qgr clear`');
	console.log('`qgr`');
	console.log('`qgr -r http://192.168.1.124:1026`');
	console.log('');
	console.log('Defaults for the options are specified in brackets.');
	console.log('');
	console.log('Options:');
	console.log('  ' + help_info.join('\n  '));
	console.log('');
	return;
} 
else if ( cmd == 'export' ) {
	util.assert(args.length, 'export Bad argument. system name required, for example "qgr export ios"');
	new QgrExport(process.cwd(), args[0]).export();
} 
else if ( cmd == 'build' || cmd == 'rebuild' || cmd == 'init' ) {
	if ( cmd == 'rebuild' ) {
		fs.rm_r_sync(process.cwd() + '/out/install');
		fs.rm_r_sync(process.cwd() + '/out/libs');
		fs.rm_r_sync(process.cwd() + '/out/public');
	}
	var build = new QgrBuild(process.cwd(), process.cwd() + '/out');
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
