
import { _CVD, Application, Window, createCss, mainScreenScale, Box } from 'quark'
import {reader} from 'quark/fs'
import * as types from 'quark/types'
import * as dialog from 'quark/dialog'
import {Switch} from 'quark/checkbox'
import { ClickEvent } from 'quark/event'
import { Bubbles, Priority } from 'quark/bubbles'
import { NavButton } from './tool';

const px = 1 / mainScreenScale()
const resolve = require.resolve

// Test

createCss({
	'.next_btn': {
		width: 'match',
		// height: 45,
		textLineHeight: 45,
		// textSize: 14,
		textWhiteSpace: 'pre',
		textAlign: "left",
		borderRadius: 0,
		//textBackgroundColor: "#0f0",
	},
	'.next_btn:normal': {
		textColor: '#0f0',
		// backgroundColor: '#fff0', time: 180
	},
	'.next_btn:hover': {
		textColor: '#f0f',
		// backgroundColor: '#ececec', time: 50
	},
	'.next_btn:active': {
		textColor: '#f00',
		// backgroundColor: '#E1E4E4', time: 50
	},
})

const app = new Application()

// register font icomoon-ultimate
app.fontPool.addFontFamily( reader.readFileSync(resolve('./icomoon.ttf')) )

const win = new Window({
	title: 'Examples',
	frame: types.newRect(0,0,375,700),
	//backgroundColor: types.newColor(255,0,0,255),
})
// .render(
// 	<flex width="100%" height="50%" itemsAlign="centerCenter">
// 		<button
// 			minWidth="10%"
// 			maxWidth="40%"
// 			height="100%"
// 			// minHeight="10%"
// 			// maxHeight="40%"
// 			// width="100%"
// 			paddingLeft={5}
// 			textLineHeight={1} // 100%
// 			textSize={18}
// 			textFamily="iconfont"
// 			backgroundColor="#f00"
// 			textWhiteSpace="noWrap"
// 			//weight={1}
// 			textAlign="center"
// 		>
// 			<label textFamily="default" textSize={16} textOverflow="ellipsis" value="ABCDEFGHIJKMLNOPQ" />
// 		</button>
// 		<text
// 			weight={[0,1]}
// 			height="100%"
// 			//maxWidth="50%"
// 			// width="100%"
// 			textColor="#00f"
// 			textLineHeight={1}
// 			textSize={16}
// 			textWhiteSpace="noWrap"
// 			textWeight="bold"
// 			textOverflow="ellipsisCenter"
// 			textAlign="center"
// 			value="TitleAAAAAAAAAAAAA"
// 			backgroundColor="#0f0"
// 		/>
// 		<text
// 			minWidth="10%"
// 			maxWidth="40%"
// 			height="100%"
// 			// minHeight="10%"
// 			// maxHeight="40%"
// 			// width="100%"

// 			textColor="#f0f"
// 			textLineHeight={1}
// 			// marginLeft={5}
// 			backgroundColor="#0ff"
// 			//weight={1}
// 			textAlign="center"
// 			value="A"
// 			opacity={0.5}
// 		/>
// 	</flex>
// 	// <box width="50%" height="50%" align="centerTop" backgroundColor="#f0f">
// 	// 	<text value="Start-------" align="start"  backgroundColor="#f00" />
// 	// 	<text value="Start1" align="start"  backgroundColor="#a00" />
// 	// 	<text value="Center" align="center" backgroundColor="#0f0" />
// 	// 	<text value="Center1" align="center" backgroundColor="#0a0" />
// 	// 	<text value="End" align="end" backgroundColor="#00f" />
// 	// 	<text value="End1" align="end" backgroundColor="#00a" />
// 	// </box>
// )

// win.render(<Switch style={{margin:10}} initSelected={true} />);

function show_bubbles(evt: ClickEvent) {
	Bubbles.renderShow(
		evt.sender,
		<Bubbles>
			{/* <box> */}
			<NavButton>Menu A</NavButton>
			<NavButton>Menu B------C</NavButton>
			<NavButton>Menu C</NavButton>
			<NavButton style={{borderWidth:0}}>Menu D</NavButton>
			{/* </box> */}
		</Bubbles>
	)
}

win.render(<button class="long_btn" onClick={show_bubbles}> Show Bubbles </button>);

// dialog.alert(win, 'Hello')
dialog.prompt(win, 'Hello, Make sure please as asaaaaaaaaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbbbbb', (ok, text)=>{
	if (ok)
		dialog.alert(win, text);
});
// dialog.sheetConfirm(win, [<label value='Confirm Delete' textColor="#f00" />]);
