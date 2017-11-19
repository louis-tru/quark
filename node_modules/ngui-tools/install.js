#!/usr/bin/env node

var os = require('os');
var child_process = require('child_process');

// console.log(process.argv);

if ( /win/i.test(process.platform) && process.platform != 'darwin' ) {
	if ( process.argv[2] == 'link' ) {
		child_process.execSync('npm link ' + __dirname);
	} else {
		child_process.execSync('npm install -g ' + __dirname);
	}
} else {
	if ( process.argv[2] == 'link' ) {
		child_process.execSync(`cd ${__dirname}; sudo npm link`);
	} else {
		child_process.execSync(`cd ${__dirname}; sudo npm install -g`);
	}
}	