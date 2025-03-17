
import { _CVD, Application, Window } from 'quark';
import * as types from 'quark/types';
import {reader} from 'quark/fs';
import path from 'quark/path';

new Application();
new Window({
	title: 'tests image',
	frame: types.newRect(0,0,500,500),
}).activate().render(
	<scroll width="match" height="match">
	{
		reader.readdirSync(__dirname + '/img2')
		.concat(reader.readdirSync(__dirname + '/img'))
		.filter(e=>path.extname(e.name).match(/\.(gif|jpeg|jpg|png|webp)$/i)).map((e,j)=>
			(<image key={j} borderBottom="5 #fff" width="100%" src={e.pathname} />)
		)
	}
	</scroll>
);