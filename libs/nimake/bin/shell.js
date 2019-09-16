#!/usr/bin/env node

var util = require('nikit');
var fs = require('nikit/fs');
var { NguiBuild } = require('../build');
var { NguiExport } = require('../export');
var server = require('../server');
var arguments = require('nikit/arguments');
var args = process.argv.slice(2);
var cmd = args.shift();
var opts = arguments.options;
var help_info = arguments.helpInfo;
var def_opts = arguments.defOpts;

def_opts(['help','h'], 0,       '-h, --help     print help info');
def_opts(['port', 'p'], 1026, 	'--port=PORT,-p PORT Run ngui debugger server port [{0}]');
def_opts(['remote', 'r'], '', 	'--remote=ADDRESS,-r ADDRESS Remote console address [none]');

if ( opts.help || opts.h /*cmd == 'help' || cmd == 'h'*/ ) { 
	console.log('');
	console.log('Usage: nimake COMMAND [OS]');
	console.log('Usage: nimake [OPTION]...');
	console.log('');
	console.log('Examples:');
	console.log('`nimake init`');
	console.log('`nimake build`');
	console.log('`nimake rebuild`');
	console.log('`nimake export ios`');
	console.log('`nimake export android`');
	console.log('`nimake install`');
	console.log('`nimake clear`');
	console.log('`nimake`');
	console.log('`nimake -r http://192.168.1.124:1026`');
	console.log('');
	console.log('Defaults for the options are specified in brackets.');
	console.log('');
	console.log('Options:');
	console.log('  ' + help_info.join('\n  '));
	console.log('');
	return;
} 
else if ( cmd == 'export' ) {
	util.assert(args.length, 'export Bad argument. system name required, for example "nimake export ios"');
	new NguiExport(process.cwd(), args[0]).export();
} 
else if ( cmd == 'build' || cmd == 'rebuild' || cmd == 'init' ) {
	if ( cmd == 'rebuild' ) {
		fs.rm_r_sync(process.cwd() + '/out/install');
		fs.rm_r_sync(process.cwd() + '/out/libs');
		fs.rm_r_sync(process.cwd() + '/out/public');
	}
	var build = new NguiBuild(process.cwd(), process.cwd() + '/out');
	if ( cmd == 'init' ) {
		build.initialize();
	} else {
		build.build();
	}
}
else if (cmd == 'install') {
	new NguiBuild(process.cwd(), process.cwd() + '/out').install_depe();
} 
else if ( cmd == 'clear' ) {
	fs.rm_r_sync(process.cwd() + '/out');
}
else {
	// run wrb server
	server.start_server(arguments.options);
}
