
import { _CVD, Application, Window, createCss, mainScreenScale } from 'quark'
import {reader} from 'quark/fs'

const px = 1 / mainScreenScale();
const resolve = require.resolve;

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

new Window({ title: 'Examples' }).render(
	<button class="next_btn" textColor="#0079ff" borderBottom={`${px} #c8c7cc`}>

		ABCDEFG
{/* 
		<matrix x={0} align="middle" backgroundColor="#f009">
			<text value={'\uedbe'} textFamily="icomoon-ultimate" textColor="#aaa" />
		</matrix> */}
	</button>
)