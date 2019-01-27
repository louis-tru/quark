
var wget = require('./wget');

debugger;

wget(
	'http://pgmyt3sfp.bkt.clouddn.com/2018-10-19_leveldb_full.zip', 
	'/tmp/2018-10-19_leveldb_full.zip',
	{
		broken_point: 1,
		progress: e=>{
			console.log(e);
		},
	}
).then(console.error);
