
import * as http from 'ngui/http';
import util from 'ngui/util';
import * as sys from 'ngui/sys';
import { Indep, ViewController, default as ngui, Text, _CVD } from 'ngui';

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
		var fsp_value = ngui.displayPort.fsp();
		var cpu_usage = sys.cpuUsage();
		this.state = { fsp_value, cpu_usage };
		setTimeout(e=>this.up_fsp(), 1000);
	}

}

var fsp: FSP;

export function show_fsp() {
	util.assert(!fsp);
	util.assert(ngui.app);

	function show_fsp_ok() {
		if (ngui.root) {
			fsp = ngui.render(<FSP />, ngui.root);
			fsp.up_fsp();
		}
	}

	if (ngui.app.isLoaded) {
		show_fsp_ok.setTimeout(1000);
	} else {
		ngui.app.onLoad.on(e=>show_fsp_ok.setTimeout(1000));
	}
};
