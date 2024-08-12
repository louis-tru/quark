
import { LOG, Pv, Mv, Mvcb, Ca } from './tool'
import util from 'quark/util';
import * as http from 'quark/http';

export default async function(_: any) {
	LOG('\nTest Util:\n')
	Pv(util, 'debug', util.debug)
	Mv(util, 'version', [])
	Mv(util, 'hashCode', ['ABCDEFG'])
	Mv(util, 'hash', ['ABCDEFG'])
	Mv(util, 'getId', [])
	Mv(util, 'runScript', ['console.log("ABCDEFG")'])
	await Mvcb(util, 'nextTick', [()=>{}])
	Mv(util, 'gc', [])
	Mv(util, 'noop', [])
	Mv(util, 'extend', [{a:10}, {b:100}])
	Pv(util, 'options', e=>true)
	Mv(String, 'format', ['ABC{1}DE{0}F{0}G', '-0-', '-1-'])
	Mv(Date, 'parseDate', ['2016-02-13 01:12:13']);
	Mv(Date, 'parseDate', ['20160213011213']);
	Mv(Date, 'formatTimeSpan', [1000*100, 'mm:ss'])
	Pv(Date, 'currentTimezone', e=>true)
	Mv(new Date(), 'toString', ['yyyy-MM-dd hh:mm:ss'])
	Mv(Array, 'isArray', [[]])
	Mv(Array, 'toArray', [{0:100, 1:200, length: 2}])
	Mv(util, 'getProp', ['a.b.c.d', { a:{b:{c:{d:100}}} }])
	Mv(util, 'setProp', ['a.b.c.d2', 200, { a:{b:{c:{d:100}}} }])
	Mv(util, 'removeProp', ['a.b.c.d', { a:{b:{c:{d:100}}} }])
	Mv(util, 'random', [0, 10])
	Mv(util, 'random', [0, 10])
	Mv(util, 'random', [0, 10])
	Mv(util, 'random', [0, 10])
	Mv(util, 'fixRandom', [10, 20, 30, 40])
	Mv(util, 'fixRandom', [10, 20, 30, 40])
	Mv(util, 'fixRandom', [10, 20, 30, 40])
	Mv(util, 'fixRandom', [10, 20, 30, 40])
	Mv(util, 'clone', [{ a:{b:100} }])
	Mv(JSON, 'stringify', [new ReferenceError('Err')])
	Mv(util, 'equalsClass', [Object, http.HttpClientRequest])
	Mv(util, 'select', [true, 1])
	Mv(util, 'filter', [{ a:'a',b:'b',c:'c', d:'d' }, ['a', 'c'], true])
	Mv(util, 'filter', [{ a:'a',b:'b',c:'c', d:'d' }, function(i){return i=='a'}])
	Mv(util, 'update', [{ a:'a',b:'b',c:'c', d:'d' }, { a:100,d:'KKKK' }])
	Pv(util, 'config', e=>true)
}