
import { GUIApplication, ViewController, Root, Div, Scroll, render, CSS } from 'langou';
import { Color } from 'langou/value';
import { random } from 'langou/util';
import './uu';

CSS({
	'.item': {
		height: 40,
		width: '25%',
	},
})

class RootViewController extends ViewController {

	triggerMounted(e) {
		super.triggerMounted(e);

		uu.start();

		render(
			<Scroll width="full" height="full">
				{
					Array.from({ length: 10000 }, ()=>{
						var color = new Color(random(0, 255), 
							random(0, 255), random(0, 255), 255);
						return <Div backgroundColor=color class="item" />;
					})
				}
			</Scroll>
		).appendTo(this.dom);
	
		uu.log();

	}
}

new GUIApplication({ multisample: 2 }).start(
	<RootViewController><Root backgroundColor="#000" /></RootViewController>
);

uu.show_fsp();
