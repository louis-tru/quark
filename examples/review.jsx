import { Scroll, Text } from 'qgr';
import { Mynavpage } from './public';
import { Toolbar } from 'qgr/nav';
import 'qgr/reader';

function foreground(evt) {
	var navpage = evt.sender;
	navpage.title = 'Source';
	navpage.find('text').value = reader.readFileSync(navpage.prevPage.source, 'utf8');
}

export default const vx = (
	<Mynavpage 
		navbar.backgroundColor="#333"
		navbar.backTextColor="#fff" 
		navbar.titleTextColor="#fff"
		toolbar.backgroundColor="#333"
		toolbar.hidden=true 
		backgroundColor="#333" onForeground=foreground>
		<Scroll width="full" height="full" bounceLock=0>
			<Text width="full" id="text" textColor="#fff" textSize=12 margin=5 />
		</Scroll>
	</Mynavpage>
)
