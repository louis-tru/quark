
import { GUIApplication, Root, Div, Indep, ngui, New, lock } from 'ngui';
import Color from 'ngui/value';
import {random, log } from 'ngui/util';
import './uu';

new GUIApplication({ multisample: 2 }).start(
	<Root backgroundColor="#000">
		%{
			(()=>{
				var w = ngui.displayPort.width;
				var h = ngui.displayPort.height;
				var r = [];
				for (var i = 0; i < 4000; i++) {
					var color = new Color(random(0, 255), random(0, 255), random(0, 255), 255);
					var s = random(20, 30);
					var s2 = s / 2;
					var x = random(0, w + s) - s2;
					var y = random(0, h + s) - s2;

					r.push(<Indep originX=s2 
												originY=s2 
												backgroundColor=color 
												width=s 
												height=s 
												x=x 
												y=y 
												action=[
													{ rotateZ: 0, time:0, curve:'linear' }, 
													{ rotateZ: 360 * (random(0, 1) ? -1 : 1),
														time: random(1000, 4000), curve:'linear' },
												] 
												action.loop=1e8
												action.playing=1 />);
				}
				return r;
			})()
		}
	</Root>
)

uu.show_fsp();
