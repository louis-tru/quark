
type OptionsFlags<Type> = {
	+readonly [Property in keyof Type]-?: boolean;
};

type Features = {
	darkMode: () => void;
	newUserProfile?: () => void;
};
 
type FeatureOptions = OptionsFlags<Features>;


// --------------------

type Getters<Type> = {
	-readonly [key in keyof Type as `get${Uppercase<string & key>}`]+?: () => Type[key]
};

interface Person {
	readonly name: string;
	readonly agE: number;
	readonly location: string;
}

type LazyPerson = Getters<Person>;

type RemoveReadonly<Type> = {
	-readonly [key in keyof Type]: Type[key]
}

// --------------------

// Remove the 'kind' property
type RemoveKindField<Type> = {
	[Property in keyof Type as Exclude<Property, "kind">]: Type[Property]
};

interface Circle {
	kind: "circle";
	radius: number;
}

type KindlessCircle = RemoveKindField<Circle>;


// --------------------

type ExtractPII<Type> = {
  [Property in keyof Type]: Type[Property] extends { pii: true } ? 1 : false;
};
 
type DBFields = {
  id: { format: "incrementing" };
  name: { type: string; pii: true };
};
 
type ObjectsNeedingGDPRDeletion = ExtractPII<DBFields>;


// --------------------

type UppercaseGreeting = "HELLO WORLD";
type UncomfortableGreeting = Lowercase<UppercaseGreeting>;


// --------------------

type Getters2 = {
	// -readonly [key in keyof Type as `get${Uppercase<string & key>}`]+?: () => Type[key]
	[key:`.${string}`]: number
};


var c : Getters2 = {'.aa':100,'.b':200}


// ---------------------------- Number Range -----------------------------------------

type BuildPowersOf2LengthArrays<N extends number, R extends never[][]> =
	R[0][N] extends never ? R : BuildPowersOf2LengthArrays<N, [[...R[0], ...R[0]], ...R]>;

type ConcatLargestUntilDone<N extends number, R extends never[][], B extends never[]> =
	B["length"] extends N ? B : [...R[0], ...B][N] extends never
		? ConcatLargestUntilDone<N, R extends [R[0], ...infer U] ? U extends never[][] ? U : never : never, B>
		: ConcatLargestUntilDone<N, R extends [R[0], ...infer U] ? U extends never[][] ? U : never : never, [...R[0], ...B]>;

type Replace<R extends any[], T> = { [K in keyof R]: T }

type TupleOf<T, N extends number> = number extends N ? T[] : {
	[K in N]: 
	BuildPowersOf2LengthArrays<K, [[never]]> extends infer U ? U extends never[][]
	? Replace<ConcatLargestUntilDone<K, U, []>, T> : never : never;
}[N]

type RangeOf<N extends number> = Partial<TupleOf<unknown, N>>['length'];

type RangeNumber<From extends number, To extends number> = Exclude<RangeOf<To>, RangeOf<From>> | From;

type uint8_t = RangeNumber<0,255>

var u: uint8_t = 255;

// ---------------------------------------------------------------------

type Enumerate<N extends number, Acc extends number[] = []> = Acc['length'] extends N
? Acc[number]
: Enumerate<N, [...Acc, Acc['length']]>

type IntRange<F extends number, T extends number> = Exclude<Enumerate<T>, Enumerate<F>>;

type T = IntRange<0, 512>
