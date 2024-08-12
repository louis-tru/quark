
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
	let ok: boolean;

	if (typeof verify == 'function') {
		ok = (verify as any)(r);
	} else {
		ok = r === verify;
	}
	console.log( 'Prop', name, ':', ok ? 'ok': 'no');
	if (!ok)
		throw new Error('test fail');
}

export function Mv<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: VerifyType<Self, Name> | ((arg: VerifyType<Self, Name>)=>boolean),
): ReturnType<Self, Name> {
	let r = (self[name] as any)( ...args );
	let argc = arguments.length;

	function print(r: any) {
		let ok: boolean = true;
		if ( argc > 3 ) {
			if (typeof verify == 'function') {
				ok = (verify as any)(r);
			} else {
				ok = verify === r;
			}
		}
		console.log( 'Method', String(name) + '()', ':', ok ? 'ok': 'no');
		if (!ok)
			throw new Error('test fail');
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
			let ok: boolean = true;
			if ( argc > 3 ) {
				var v_args = args;
				if (sync)
					v_args = [r];
				if (typeof verify === 'function') {
					ok = (verify as any)(...v_args);
				} else {
					ok = verify === v_args[0];
				}
			}
			console.log( 'Method', String(name) + '()', ':', ok ? 'ok': 'no'/*, ...args*/);
			if (!ok)
				throw new Error('test fail');
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

export function Ca<T, Args>(func: (...args: Args[])=>Promise<T>) {
	const r = func();
	r.catch(function(err: any) {
		LOG('Error:');
		LOG(err.message);
		if (err.stack) {
			LOG(err.stack);
		}
	});
	return r;
}

exports.CALL_ASYNC = Ca;