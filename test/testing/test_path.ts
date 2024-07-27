
import path, {URL} from 'quark/path'
import { Pv, Mv } from './tool'

console.log('\nOutputs:\n')
const url = new URL('https://quarks.cc:81/home/index.html?a=a&b=b#c=c&d=d')

console.log('\nURL::Propertys:\n')
Pv(url, 'href', 'https://quarks.cc:81/home/index.html?a=a&b=b#c=c&d=d')
Pv(url, 'filename', '/home/index.html')
Pv(url, 'dirname', '/home')
Pv(url, 'search', '?a=a&b=b')
Pv(url, 'hash', '#c=c&d=d')
Pv(url, 'host', 'quarks.cc:81')
Pv(url, 'hostname', 'quarks.cc')
Pv(url, 'origin', 'https://quarks.cc:81')
Pv(url, 'basename', 'index.html')
Pv(url, 'extname', '.html')
Pv(url, 'port', '81')
Pv(url, 'protocol', 'https:')
Pv(url, 'params', e=>e.a=='a'&&e.b=='b')
Pv(url, 'hashParams', e=>e.c=='c'&&e.d=='d')

console.log('\nURL::Methods:\n')
Mv(url, 'getParam', ['a']);
Mv(url, 'setParam', ['a', 'A']);
Mv(url, 'deleteParam', ['a']);
Mv(url, 'clearParam', []);
Mv(url, 'getHash', ['a'], 'A');
Mv(url, 'setHash', ['e', 'E']);
Mv(url, 'getHash', ['e'], 'E');
Mv(url, 'clearHash', []);
Mv(url, 'relative', ['https://quarks.cc/A/B/C/test.js'], '../../../home/index.html');

console.log('\nMethods:\n');
Mv(path, 'executable', [])
Mv(path, 'documents', [])
Mv(path, 'temp', [])
Mv(path, 'resources', [])
Mv(path, 'fallbackPath', [path.resources()])
Mv(path, 'cwd', [])
Mv(path, 'chdir', ['/'])
Mv(path, 'cwd', [], 'file:///')
Mv(path, 'search', ['http://quarks.cc/?a=100'], '?a=100')
Mv(path, 'hash', ['http://quarks.cc/?a=100&b=test#a=200&b=300'], '#a=200&b=300')
Mv(path, 'filename', ['file:///a/b/c/kk.jsx'], '/a/b/c/kk.jsx')
Mv(path, 'dirname', ['file:///a/b/c/kk.jsx'], '/a/b/c')
Mv(path, 'host', ['a/b/c/kk.jsx'], '')
Mv(path, 'host', ['http://quarks.cc:81/a/b/c/kk.jsx'], 'quarks.cc:81')
Mv(path, 'hostname', ['a/b/c/kk.jsx'], '')
Mv(path, 'hostname', ['http://quarks.cc/a/b/c/kk.jsx'], 'quarks.cc')
Mv(path, 'origin', ['a/b/c/kk.jsx'], 'file://')
Mv(path, 'origin', ['http://quarks.cc/a/b/c/kk.jsx'], 'http://quarks.cc')
Mv(path, 'basename', ['a/b/c/kk.jsx'], 'kk.jsx')
Mv(path, 'extname', ['a/b/c/kk.jsx'], '.jsx')
Mv(path, 'port', ['http://quarks.cc:81/a/b/c/kk.jsx'], '81')
Mv(path, 'protocol', ['a/b/c/kk.jsx'], 'file:')
Mv(path, 'protocol', ['http://quarks.cc/a/b/c/kk.jsx'], 'http:')
Mv(path, 'protocol', ['lib://util/fs'], 'lib:')
Mv(path, 'protocol', ['zip:///var/data/test.apk@/assets/index'], 'zip:')
Mv(path, 'params', ['http://quarks.cc/?a=100&b=test#a=200&b=300'], e=>e.a=='100')
Mv(path, 'hashParams', ['http://quarks.cc/?a=100&b=test#a=200&b=300'], e=>e.a=='200')
Mv(path, 'getParam', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a'], '100')
Mv(path, 'setParam', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a', 'A'], 'http://quarks.cc/?a=A&b=test#a=200&b=300')
Mv(path, 'deleteParam', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a'], 'http://quarks.cc/?b=test#a=200&b=300')
Mv(path, 'clearParam', ['http://quarks.cc/?a=100&b=test#a=200&b=300'], 'http://quarks.cc/#a=200&b=300')
Mv(path, 'getHash', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a'], '200')
Mv(path, 'setHash', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a', 'H'], 'http://quarks.cc/?a=100&b=test#a=H&b=300')
Mv(path, 'deleteHash', ['http://quarks.cc/?a=100&b=test#a=200&b=300', 'a'], 'http://quarks.cc/?a=100&b=test#b=300')
Mv(path, 'clearHash', ['http://quarks.cc/?a=100&b=test#a=200&b=300'], 'http://quarks.cc/?a=100&b=test')
Mv(path, 'relative', ['http://quarks.cc/A/B/C/test.js', 'http://quarks.cc/home'], '../../../home')
Mv(path, 'isAbsolute', ['http://quarks.cc/home/index.html'], true)
Mv(path, 'isAbsolute', ['file:///a/b/c/kk.jsx'], true)
Mv(path, 'isAbsolute', ['file:///d:/a/b/c/kk.jsx'], true)
Mv(path, 'isAbsolute', ['/a/b/c/kk.jsx'], true)
// Mv(path, 'isAbsolute', ['d:/a/b/c/kk.jsx'], false)
Mv(path, 'isAbsolute', ['c/kk.jsx'], false)
Mv(path, 'resolve', ['http://quarks.cc/home', '..', 'A', 'B', '..', 'C', 'test.js'], 'http://quarks.cc/A/C/test.js')