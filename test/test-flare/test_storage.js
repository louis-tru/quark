
import { LOG, M } from './test'
import 'noug/storage'

LOG('\nTest Storage:\n')

M(storage, 'get', ['a'])
M(storage, 'set', ['a', 'A'])
M(storage, 'get', ['a'])
M(storage, 'del', ['a'])
M(storage, 'get', ['a'])
M(storage, 'getJSON', ['a'])
M(storage, 'setJSON', ['a', { a: 100, b: 100, c:[0,1,2,3,4,5,6] }])
M(storage, 'getJSON', ['a'])
M(storage, 'delJSON', ['a'])
M(storage, 'getJSON', ['a'])


M(storage, 'set', ['a', 'A'])
M(storage, 'set', ['b', 'B'])
M(storage, 'set', ['c', 'C'])
M(storage, 'set', ['d', 'D'])

M(storage, 'setJSON', ['a', 'A'])
M(storage, 'setJSON', ['b', 'B'])
M(storage, 'setJSON', ['c', 'C'])
M(storage, 'setJSON', ['d', 'D'])

M(storage, 'get', ['a'])
M(storage, 'get', ['b'])
M(storage, 'get', ['c'])
M(storage, 'get', ['d'])

M(storage, 'getJSON', ['a'])
M(storage, 'getJSON', ['b'])
M(storage, 'getJSON', ['c'])
M(storage, 'getJSON', ['d'])

M(storage, 'clear');

M(storage, 'get', ['a'])
M(storage, 'get', ['b'])
M(storage, 'get', ['c'])
M(storage, 'get', ['d'])

M(storage, 'getJSON', ['a'])
M(storage, 'getJSON', ['b'])
M(storage, 'getJSON', ['c'])
M(storage, 'getJSON', ['d'])
