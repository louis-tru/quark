
import { GUIApplication: App, Root, Image, Scroll } from 'langou';
import 'langou/reader';
import 'path';

new App({ multisample: 2 }).start(
	<Root backgroundColor="#000">
		<Scroll width="full" height="full">
			${
				(()=>{
					var r = [];
					reader.readdirSync(__dirname + '/img2').concat(
					reader.readdirSync(__dirname + '/img')).forEach(e=>{
						if (path.extname(e.name).match(/\.(gif|jpeg|jpg|png|webp)$/i)) {
							r.push(<Image borderBottom="5 #fff" width="100%" src=e.pathname />);
						}
					});
					return r;
				})()
			}
		</Scroll>
	</Root>
);
