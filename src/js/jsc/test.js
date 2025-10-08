
(function() {
const _rejectionListener = globalThis._rejectionListener;
const NativePromise = globalThis.Promise;
class Promise extends NativePromise {
	constructor(executor) {
		super((resolve, reject)=>{
			executor(resolve, (reason)=>{
				queueMicrotask(()=>{
					if (!this._handled)
						_rejectionListener(this,reason);
				});
				reject(reason);
			});
		});
	}
	then(onfulfilled, onrejected) {
		this._handled ||= onrejected instanceof Function;
		return super.then(onfulfilled, onrejected);
	}
	catch(onrejected) {
		this._handled ||= onrejected instanceof Function;
		return super.catch(onrejected);
	}
}
globalThis.Promise = Promise;
})();

// tests
new Promise(function(cb,errCb){   errCb(new Error('ABCDEFG ERR')) }).then(function(){console.log('ok')}, function(err){ console.log('---catch', err) })

new Promise(function(cb,errCb){   errCb(new Error('ABCDEFG ERR')) }).catch(function(err){ console.log('---catch', err) })

new Promise(function(cb,errCb){   errCb(new Error('ABCDEFG ERR')) })

new Promise(function(cb,errCb){ errCb('AAAAAAA'); throw new Error('ABCDEFG ERR') })
