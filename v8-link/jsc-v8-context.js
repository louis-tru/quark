
async function AFunc() {}
function MapIterator() {}
function SetIterator() {}
MapIterator.prototype = (new Map()).entries().__proto__;
SetIterator.prototype = (new Set()).entries().__proto__;


if (!Error.captureStackTrace) {
	Error.captureStackTrace = function(targetObject, constructorOpt) {
		// TODO ..
		// print warning
	};
}

// exports
exports.Object = Object;
exports.Function = Function;
exports.Number = Number;
exports.Boolean = Boolean;
exports.String = String;
exports.Date = Date;
exports.RegExp = RegExp;
exports.Error = Error;
exports.RangeError = RangeError;
exports.ReferenceError = ReferenceError;
exports.SyntaxError = SyntaxError;
exports.TypeError = TypeError;
exports.Map = Map;
exports.Set = Set;
exports.WeakMap = WeakMap;
exports.WeakSet = WeakSet;
exports.Symbol = Symbol;
exports.Proxy = Proxy;
exports.Promise = Promise;
exports.DataView = DataView;
/*exports.SharedArrayBuffer = typeof SharedArrayBuffer == 'function' ? SharedArrayBuffer : undefined;*/
exports.ArrayBuffer = ArrayBuffer;
exports.TypedArray = Uint8Array.prototype.__proto__.constructor;
exports.Uint8Array = Uint8Array;
exports.Int8Array = Int8Array;
exports.Uint16Array = Uint16Array;
exports.Int16Array = Int16Array;
exports.Uint32Array = Uint32Array;
exports.Int32Array = Int32Array;
exports.Float32Array = Float32Array;
exports.Float64Array = Float64Array;
exports.Uint8ClampedArray = Uint8ClampedArray;
exports.SyncFunction = AFunc.constructor;
exports.MapIterator = MapIterator;
exports.SetIterator = SetIterator;

