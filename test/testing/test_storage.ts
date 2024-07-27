
import { LOG, Mv } from './tool'
import * as storage from 'quark/storage'

LOG('\nTest Storage:\n')

Mv(storage, 'set', ['a', 'A'])
Mv(storage, 'get', ['a'], 'A')
Mv(storage, 'remove', ['a'])
Mv(storage, 'get', ['a'], undefined)

Mv(storage, 'set', ['a', 'A'])
Mv(storage, 'set', ['b', 'B'])
Mv(storage, 'set', ['c', 'C'])
Mv(storage, 'set', ['d', 'D'])

Mv(storage, 'get', ['a'], 'A')
Mv(storage, 'get', ['b'], 'B')
Mv(storage, 'get', ['c'], 'C')
Mv(storage, 'get', ['d'], 'D')

Mv(storage, 'clear', [])

Mv(storage, 'get', ['a'], undefined)
Mv(storage, 'get', ['b'], undefined)
Mv(storage, 'get', ['c'], undefined)
Mv(storage, 'get', ['d'], undefined)