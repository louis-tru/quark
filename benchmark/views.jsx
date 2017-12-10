
import { GUIApplication, Root, Div, Scroll, New } from 'ngui';
import Color from 'ngui/value';
import './print';
import 'ngui/util';

new GUIApplication({ multisample: 2 }).start(
	<Root backgroundColor="#000" />
).onLoad = function() {

	print.start();

	New(
		<Scroll width="full" height="full">
			${
				(()=>{
					var r = [];
					for (var i = 0; i < 100000; i++) {
						var color = new Color(util.random(0, 255), 
							util.random(0, 255), util.random(0, 255), 255);
						r.push(<Div backgroundColor=color width="25%" height=40 />);
					}
					return r;
				})()
			}
		</Scroll>
	).appendTo(this.root);

	print.time();

};
