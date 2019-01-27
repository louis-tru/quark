
var { Buffer } = require('buffer');
var util = require('shark-utils');
var r = require('shark-utils/request');

// 测试EOS区块链RPC交易

async function main() {

	const chain_server = 'http://192.168.1.115:8888/v1';
	var req = new r.Request(chain_server);
	req.urlencoded = false;
	req.appendTimespan = false;

	var result;

	var { 
		chain_id,
		head_block_time: timestamp,
		last_irreversible_block_num: ref_block_num, 
		last_irreversible_block_id: ref_block_id,
	} = await req.get('chain/get_info');

	var ref_block_prefix = Buffer.from(ref_block_id,'hex').readUIntLE(8, 4);

	console.log(timestamp);

	const Expiration = 10; // 交易10秒后过期,表示该交易10秒后还未执行取消
	var d = new Date(timestamp).addMs(Expiration * 1e3 + (util.timezone * 3600 * 1000));
	var expiration = d.toJSON().substr(0, 23);
	
	console.log(expiration);

	// 创建新帐号（调用EOS内置`eosio.newaccount`合约）
	// 调用前需先创建两对密钥，`owner`，`active`，并且这里发送公钥到EOS服务器，私钥用户自己保存到钱包
	var { binargs } = await req.post('chain/abi_json_to_bin', {
		"code": "eosio",
		"action": "newaccount",
		"args": {
			"creator": "louis",
			"name": "test2",
			"owner": {
				"threshold": 1,
				"keys": [
					{
						"key": "EOS8cf3UyrLWk4EkxTEvN2p9Gm9GBEV7fHV6Cd1dX4dbV11yd3iWS",
						"weight": 1
					}
				],
				"accounts": [],
				"waits": []
			},
			"active": {
				"threshold": 1,
				"keys": [
					{
						"key": "EOS8cf3UyrLWk4EkxTEvN2p9Gm9GBEV7fHV6Cd1dX4dbV11yd3iWS",
						"weight": 1
					}
				],
				"accounts": [],
				"waits": []
			}
		}
	});

	console.log(binargs);

	await req.post('wallet/unlock', // 解锁EOS钱包
		["default", "PW5JKdSTdyVm2TEECE8ZK8m8nRTjH4jE1CdX4u3d9pYw7tWrRGFdq"]); 

	var transaction = {
		"expiration": expiration,
		"ref_block_num": ref_block_num,
		"ref_block_prefix": ref_block_prefix,
		"actions": [
			{
				"account": "eosio",
				"name": "newaccount",
				"authorization": [
					{
						"actor": "louis",
						"permission": "active"
					}
				],
				"data": binargs
			}
		],
	};

	// 签名交易数据
	var { signatures } = await req.post('wallet/sign_transaction', [
		transaction, [ "EOS8Lk7bhq8bzB6bD4HHCEvEvYnSxftPraWVybcjDvVbPfZ2L1uQx" ], chain_id,
	]);

	console.log(signatures);

	// 提交交易
	result = await req.post('chain/push_transaction', {
		"compression": "none",
		"transaction": transaction,
		"signatures": signatures
	});

	console.log(JSON.stringify(result, null, 2));

	await req.post('wallet/lock', "default"); // 锁定钱包

}


main();