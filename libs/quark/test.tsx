
import {_CVD, Application,Window} from './index';

new Application();

const view = ()=><view ref='qq' style={{width:'100%'}} onClick={e=>{}} cursor={'resizeLeftRight'}></view>;

const box = ()=><box style={{}} width={'100!'} />;

const box2 = ()=><box><image src='AA' /></box>;

const image = ()=><image src='http://www.aaa.com/img.png' width={100} />;

new Window().activate().render(
	<flex style={{
		width: 'match',
		height: 'match',
		backgroundColor: '#f00',
	}} class='flex'
	action={[
		{time:0,backgroundColor: '#f00'},
		{time:1e4,backgroundColor: '#0f0'}
	]}>
		<box style={{width: 'match', backgroundColor: '#0f0'}}>ABCDEFG</box>
	</flex>
);