
import * as http from 'flare/http';
import util from 'flare/util';
import * as sys from 'flare/sys';
import { Indep, ViewController, default as flare, Text, _CVD } from 'flare';

var ts = 0;

export function start(...args: any[]) {
	ts = new Date().valueOf();
	console.log('start:', ...args);
}

export function log(...args: any[]) {
	var ts2 = new Date().valueOf();
	console.log('time:', ts2 - ts, ...args);
	ts = ts2;
}

class FSP extends ViewController {

	render() {
		var { fsp_value = 0, cpu_usage = 0 } = this.state;
		var value = 'FSP: ' + fsp_value + ', CPU: ' + (cpu_usage * 100).toFixed(0) + '%';
		return (
			<Indep alignY="bottom" x={5} y={-5}>
				<Text textColor="#f00" value={value} />
			</Indep>
		);
	}

	up_fsp() {
		var fsp_value = flare.displayPort.fsp();
		var cpu_usage = sys.cpuUsage();
		this.state = { fsp_value, cpu_usage };
		setTimeout(e=>this.up_fsp(), 1000);
	}

}

var fsp: FSP;

export function show_fsp() {
	util.assert(!fsp);
	util.assert(flare.app);

	function show_fsp_ok() {
		if (flare.root) {
			fsp = flare.render(<FSP />, flare.root);
			fsp.up_fsp();
		}
	}

	if (flare.app.isLoaded) {
		show_fsp_ok.setTimeout(1000);
	} else {
		flare.app.onLoad.on(e=>show_fsp_ok.setTimeout(1000));
	}
};
