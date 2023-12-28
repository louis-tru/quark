
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
