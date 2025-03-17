
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

function MethodV<
	Self, Name extends keyof Self,
>(
	print: boolean,
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: VerifyType<Self, Name> | ((arg: VerifyType<Self, Name>)=>boolean)
): ReturnType<Self, Name> {
	let r = (self[name] as any)( ...args );
	let argc = arguments.length;

	function ok(r: any) {
		let ok: boolean = true;
		if ( argc > 4 ) {
			if (typeof verify == 'function') {
				ok = (verify as any)(r);
			} else {
				ok = verify === r;
			}
		}
		console.log( 'Method', String(name) + '()', ':', ok ? 'ok': 'no', print ? r: '');
		if (!ok)
			throw new Error('test fail');
	}

	if (r instanceof Promise) {
		return new Promise<void>((resolve,reject)=>{
			r.then(function(e) {
				ok(e);
				resolve(e);
			}).catch(reject);
		}) as any;
	} else {
		return ok(r), r;
	}
}

function MethodVcb<
	Self, Name extends keyof Self,
>(
	print: boolean,
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: any | null
) {
	let argc = arguments.length;

	return new Promise<any>(function(resolve, reject) {
		function ok(r: any[]) {
			let ok: boolean = true;
			if ( argc > 4 ) {
				if (typeof verify === 'function') {
					ok = (verify as any)(...r);
				} else {
					ok = verify === r[0];
				}
			}
			console.log( 'Method', String(name) + '()', ':', ok ? 'ok': 'no', ...(print ? r: []));
			if (ok) {
				resolve(r);
			} else {
				reject(new Error('test fail'));
			}
		}
		if (typeof args.indexReverse(0) == 'function' ) {
			let cb = args.indexReverse(0) as any;
			args[args.length - 1] = async (err: any, ...args: any[])=>{
				await cb(err, ...args);
				if (err) {
					reject(err)
				} else {
					ok(args);
				}
			}
			(self[name] as any)(...args);
		} else {
			ok([(self[name] as any)(...args)]);
		}
	});
}

export function Mv<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: VerifyType<Self, Name> | ((arg: VerifyType<Self, Name>)=>boolean)
): ReturnType<Self, Name> {
	return arguments.length > 3 ?
	MethodV(false, self, name, args, verify): MethodV(false, self, name, args);
}

export function Mvp<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: VerifyType<Self, Name> | ((arg: VerifyType<Self, Name>)=>boolean)
): ReturnType<Self, Name> {
	return arguments.length > 3 ?
	MethodV(true, self, name, args, verify): MethodV(true, self, name, args);
}

export function Mvcb<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: any | null
) {
	return arguments.length > 3 ?
	MethodVcb(false, self, name, args, verify): MethodVcb(false, self, name, args);
}

export function Mvcbp<
	Self, Name extends keyof Self,
>(
	self: Self,
	name: FunctionName<Self, Name>,
	args: Parameters<Self, Name>,
	verify?: any | null
) {
	return arguments.length > 3 ?
		MethodVcb(true, self, name, args, verify): MethodVcb(true, self, name, args);
}

export function Ca<T, Args>(func: (arg: Args, cb: (err?: Error, t?: T)=>void)=>void, arg: Args): Promise<T> {
	return new Promise<T>(function(resolve, reject) {
		try {
			func(arg, function(err, t) {
				if (err) 
					reject(err)
				else
					resolve(t!);
			});
		} catch(err) {
			reject(err);
		}
	});
}

export const Gc = util.gc;