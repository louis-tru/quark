
import {Window} from './window';
import {Application} from './app';
import {Jsx} from './ctr';

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

// import {Application,Window,ViewController} from './index'
// import * as http from './http'
// import * as buffer from './buffer'

// class MyCtr extends ViewController<{param: number}, {data?: Uint8Array}> {
// 	triggerLoad() {
// 		return http.get('http://192.168.1.100:1026/README.md?param=' + this.props.param).then(e=>this.setState({data:e.data}));
// 	}
// 	render() {
// 			return (
// 					<box width={100} height={100} backgroundColor="#f00">
// 							{this.state.data&&buffer.toString(this.state.data)}
// 					</box>
// 			)
// 	}
// }
// new Application();
// new Window().activate().render(
// 	<MyCtr param={10} />
// );