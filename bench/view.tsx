
import { GUIApplication, ViewController, Root, Div, Scroll, default as flare, _CVD } from 'flare';
import { Color } from 'flare/value';
import util from 'flare/util';
import * as uu from './uu';

const {random} = util

flare.css({
	'.item': {
		height: 40,
		width: '25%',
	},
})

class RootViewController extends ViewController {

	triggerMounted() {
		super.triggerMounted();

		uu.start();

		flare.render<Scroll>(
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
