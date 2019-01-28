
import { LOG, AM, M, P, VM, VP, CA } from './test'
import 'qgr/base'
import 'qgr/http'

async function test() {

	LOG('\nTest Util:\n')

	P(util, 'dev')
	M(util, 'hashCode', ['ABCDEFG'])
	M(util, 'hash', ['ABCDEFG'])
	M(util, 'id')
	M(util, 'version')
	M(util, 'runScript', ['console.log("ABCDEFG")'])
	await AM(process, 'nextTick', [()=>1])
	M(util, 'garbageCollection')
	M(util, 'transformJSX', ['var vx = (<Div> ABCDEFG </Div>); New(vx)', 'test.jsx'])
	M(util, 'transformJS', ['import "qgr/base"', 'test.js'])
	P(util, 'noop')
	M(util, 'extend', [{a:10}, {b:100}])
	M(util, 'err', ['Err'])
	M(util, 'err', [new Error('Err')])
	M(util, 'err', [{ message: 'Err' }])
	M(util, 'cb', [function() { console.log('OK') }])
	M(util, 'cb')
	// M(util, 'throw', ['Err', util.noop])
	P(util, 'options')
	M(String, 'format', ['ABC{1}DE{0}F{0}G', '-0-', '-1-'])
	M(Date, 'parseDate', ['2016-02-13 01:12:13']);
	M(Date, 'parseDate', ['20160213011213']);
	M(Date, 'formatTimeSpan', [1000*100, 'mm:ss'])
	P(util, 'timezone')
	M(new Date(), 'toString', ['yyyy-MM-dd hh:mm:ss'])
	M(Array, 'isArray', [[]])
	M(Array, 'toArray', [{0:100, 1:200, length: 2}])
	M(util, 'isDefaultThrow', [function(){}])
	M(util, 'get', ['a.b.c.d', { a:{b:{c:{d:100}}} }])
	M(util, 'set', ['a.b.c.d2', 200, { a:{b:{c:{d:100}}} }])
	M(util, 'del', ['a.b.c.d', { a:{b:{c:{d:100}}} }])
	M(util, 'random', [0, 10])
	M(util, 'random', [0, 10])
	M(util, 'random', [0, 10])
	M(util, 'random', [0, 10])
	M(util, 'fixRandom', [10, 20, 30, 40])
	M(util, 'fixRandom', [10, 20, 30, 40])
	M(util, 'fixRandom', [10, 20, 30, 40])
	M(util, 'fixRandom', [10, 20, 30, 40])
	M(util, 'clone', [{ a:{b:100} }])
	M(util, 'wrap', [{ a:100,b:200 }])
	P(util, 'libs')
	M(JSON, 'stringify', [new ReferenceError('Err')])
	M(util, 'equalsClass', [http.NativeHttpClientRequest, http.HttpClientRequest])
	M(util, 'select', [true, 1])
	M(util, 'filter', [{ a:'a',b:'b',c:'c', d:'d' }, ['a', 'c'], true])
	M(util, 'filter', [{ a:'a',b:'b',c:'c', d:'d' }, function(i){return i=='a'}])
	M(util, 'update', [{ a:'a',b:'b',c:'c', d:'d' }, { a:100,d:'KKKK' }])
	P(util, 'config')
	//util.assert(0)
	//M(util, 'fatal')
}

CA(test);
