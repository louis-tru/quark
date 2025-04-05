#!/usr/bin/env node

function gk(x) {
	return Math.exp(-x*x*2);
}

var N = Number(process.argv[2]) || 13;
var fixed = Number(process.argv[3]) || 6;

if (N % 2 == 0) N += 1;

var gk_t = 0;
var stepping = 1/(N - 1);

console.log(`const lowp int N = ${N};`);
console.log(`const lowp float stepping = ${stepping.toFixed(fixed)};`);

for (var i = 0, x = 0; i < N; i++) {
	let g = gk(x);
	gk_t += g;
	x += stepping;
	// console.log(`const lowp float gk_y${i} = ${g.toFixed(fixed)};`);
}

console.log(`const lowp float gk_t = ${(gk_t-1).toFixed(fixed)};`);
// console.log(`#define gk(x) exp(-x*x*2.0)`);

// console.log(`
// vec4 tex(in lowp vec2 coord, in lowp vec2 d) {
// 	return textureLod(image, coord + d, imageLod) + textureLod(image, coord - d, imageLod);
// }`);

console.log(`void main() {`);
console.log(`	lowp vec2  coord = gl_FragCoord.xy / oResolution;`);
console.log(`	lowp vec4  o = textureLod(image, coord, imageLod);`);
console.log(`	lowp float x = stepping;`);
for (var i = 1, x = stepping; i < N; i++) {
	console.log(`	o += tex(coord, size * x) * ${gk(x).toFixed(fixed)}; x += stepping;`);
	x += stepping;
}
console.log(`	fragColor = blend(o, gk_t*2.0+1.0);`);
console.log(`}`);
