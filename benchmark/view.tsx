
import { GUIApplication, ViewController, Root, Div, Scroll, default as ngui, _CVD } from 'ngui';
import { Color } from 'ngui/value';
import util from 'ngui/util';
import * as uu from './uu';

const {random} = util

ngui.css({
	'.item': {
		height: 40,
		width: '25%',
	},
})

class RootViewController extends ViewController {

	triggerMounted() {
		super.triggerMounted();

		uu.start();

		ngui.render<Scroll>(
			<Scroll width="full" height="full">
				{
					Array.from({ length: 10000 }, ()=>{
						var color = new Color(random(0, 255), 
							random(0, 255), random(0, 255), 255);
						return <Div backgroundColor={color} class="item" />;
					})
				}
			</Scroll>
		).appendTo(this.domAs());
	
		uu.log();

	}
}

new GUIApplication({ multisample: 2 }).start(
	<RootViewController><Root backgroundColor="#000" /></RootViewController>
);

uu.show_fsp();
