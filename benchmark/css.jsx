
import { GUIApplication, Root, Div, Indep, ngui, New, CSS } from 'ngui';
import Color from 'ngui/value';
import random from 'ngui/util';
import './print';

new GUIApplication({ multisample: 2 }).start(
	<Root backgroundColor="#000" class="root">
		%{
			(()=>{
				var w = ngui.displayPort.width;
				var h = ngui.displayPort.height;
				var csss = {};
				var r = [];
				for (var i = 0; i < 100000; i++) {
					var color = new Color(random(0, 255), random(0, 255), random(0, 255), 255);
					var s = random(20, 30);
					var s2 = s / 2;
					var x = random(0, w + s) - s2;
					var y = random(0, h + s) - s2;
					csss['.root .css_' + i] = {
						backgroundColor: color,
						width: s,
						height: s,
						x: x,
						y: y,
					};
					r.push(<Indep class=('css_' + i) />);
				}
				
				CSS(csss);

				return r;
			})()
		}
	</Root>
)
