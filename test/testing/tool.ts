
import util from 'quark/util';

type Functions<Type> = {
	[Key in keyof Type as Type[Key] extends Function ? Key: never]: Type[Key];
};
type FunctionName<Self, Name extends keyof Self> = Self[Name] extends Function ? Name : never;
type Parameters<Self, Name extends keyof Self> = Self[Name] extends (...args: infer Args) => any ? Args : never;
type VerifyType<Self, Name extends keyof Self> =
	Self[Name] extends (...args: any) => Promise<infer R> ? R:
	Self[Name] extends (...args: any) => infer R ? R : any;
type ReturnType<Self, Name extends keyof Self> = 
	Self[Name] extends (...args: any) => Promise<infer R> ? Promise<R>: 
	Self[Name] extends (...args: any) => infer R ? R : any;

export function LOG(...args: any[]) {
	console.log(...args);
}

export function Pv<Self,Name extends keyof Self>(
	self: Self,
	name: Name,
	verify: Self[Name] | ((arg: Self[Name])=>boolean),
	preSet?: (self: Self, name: Name)=>void
) {
	if (preSet)
		preSet(self, name);
	let r = self[name];
	if (typeof verify == 'function') {
		console.log( 'Prop', name, ':', (verify as any)(r) ? 'ok': 'no', r);
	} else {
		console.log( 'Prop', name, ':', r === verify ? 'ok': 'no', r);
	}
}

export function Mv<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: VerifyType<Self, Name> | ((arg: VerifyType<Self, Name>)=>boolean)
): ReturnType<Self, Name> {
	let r = (self[name] as any)( ...args );
	let argc = arguments.length;

	function print(r: any) {
		let vv = 'ok';
		if ( argc > 3 ) {
			if (typeof verify == 'function') {
				vv = (verify as any)(r) ? 'ok': 'no';
			} else {
				vv = verify === r ? 'ok': 'no';
			}
		}
		console.log( 'Method', String(name) + '()', ':', vv, r);
	}

	if (r instanceof Promise) {
		return new Promise<void>((resolve,reject)=>{
			r.then(function(e) {
				print(e);
				resolve(e);
			}).catch(reject);
		}) as any;
	} else {
		return print(r), r;
	}
}

export function Mvcb<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: any
) {
	var argc = arguments.length;

	return new Promise<any>(function(resolve, reject) {
		let sync = false;
		let r: any;

		function ok(...args: any[]) {
			var vv = '';
			if ( argc > 3 ) {
				var v_args = args;
				if (sync)
					v_args = [r];
				if (typeof verify === 'function') {
					vv = (verify as any)(...v_args) ? 'ok': 'no';
				} else {
					vv = verify === v_args[0] ? 'ok': 'no';
				}
			}
			console.log( 'Method', String(name) + '()', ':', vv, r, ...args);
			resolve(args[0]);
		}

		if (args.length && args.indexReverse(0)) {
			let last = args.length - 1;
			let f = args[last] as any;
			args[last] = async (err: any, ...args: any[])=>{
				if (err) {
					reject(err)
				} else {
					await f(...args);
					ok(...args);
				}
			}
		} else {
			sync = true;
		}

		try {
			r = (self[name] as any)( ...args );
		} catch(err) {
			reject(err);
		}

		if (sync) {
			ok(r);
		}
	});
}

export function Gc() {
	let i = 0;

	function gc() {
		util.gc();
		i++;
		if ( i < 3 ) {
			setTimeout(gc, 1000);
		}
	}
	gc();
}

export function Ca(func: Function) { // callback
	func(function(err: any) {
		if (err) {
			LOG('Error:');
			LOG(err.message);
			if (err.stack) {
				LOG(err.stack);
			}
		}
	});
}

exports.CALL_ASYNC = Ca;
