/*
 * A JavaScript implementation of the Secure Hash Algorithm, SHA-1, as defined
 * in FIPS PUB 180-1
 * Version 2.1a Copyright Paul Johnston 2000 - 2002.
 * Other contributors: Greg Holt, Andrew Kepert, Ydnar, Lostinet
 * Distributed under the BSD License
 * See http://pajhome.org.uk/crypt/md5 for details.
 */

import {chrsz,bin2binl,binl2bin} from './_md5';
import * as buffer from './buffer';
import type {Int} from './defs';

/*
 * Calculate the SHA-1 of an array of big-endian words, and a bit length
 */
function core_sha1(x: number[], len: number)
{
	x = x.map((e: number)=>transpose(e));

	/* append padding */
	x[len >> 5] |= 0x80 << (24 - len % 32);
	x[((len + 64 >> 9) << 4) + 15] = len;

	var w = Array(80);
	var a =  1732584193;
	var b = -271733879;
	var c = -1732584194;
	var d =  271733878;
	var e = -1009589776;

	for(var i = 0; i < x.length; i += 16)
	{
		var olda = a;
		var oldb = b;
		var oldc = c;
		var oldd = d;
		var olde = e;

		for(var j = 0; j < 80; j++)
		{
			if(j < 16) w[j] = x[i + j];
			else w[j] = rol(w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16], 1);
			var t = safe_add(safe_add(rol(a, 5), sha1_ft(j, b, c, d)),
											 safe_add(safe_add(e, w[j]), sha1_kt(j)));
			e = d;
			d = c;
			c = rol(b, 30);
			b = a;
			a = t;
		}

		a = safe_add(a, olda);
		b = safe_add(b, oldb);
		c = safe_add(c, oldc);
		d = safe_add(d, oldd);
		e = safe_add(e, olde);
	}
	// console.log([a, b, c, d, e]);

	return [a, b, c, d, e].map(e=>transpose(e));

}

function transpose(int32: number) {
	return (
		((int32 >> 24) & 0xff)   |
		((int32 >> 8)  & 0xff00) |
		((int32 << 8)  & 0xff0000) |
		((int32 << 24) & 0xff000000)
	);
}

/*
 * Perform the appropriate triplet combination function for the current
 * iteration
 */
function sha1_ft(t: number, b: number, c: number, d: number)
{
	if(t < 20) return (b & c) | ((~b) & d);
	if(t < 40) return b ^ c ^ d;
	if(t < 60) return (b & c) | (b & d) | (c & d);
	return b ^ c ^ d;
}

/*
 * Determine the appropriate additive constant for the current iteration
 */
function sha1_kt(t: number)
{
	return (t < 20) ?  1518500249 : (t < 40) ?  1859775393 :
				 (t < 60) ? -1894007588 : -899497514;
}

/*
 * Add integers, wrapping at 2^32. This uses 16-bit operations internally
 * to work around bugs in some JS interpreters.
 */
function safe_add(x: number, y: number)
{
	var lsw = (x & 0xFFFF) + (y & 0xFFFF);
	var msw = (x >> 16) + (y >> 16) + (lsw >> 16);
	return (msw << 16) | (lsw & 0xFFFF);
}

/*
 * Bitwise rotate a 32-bit number to the left.
 */
function rol(num: number, cnt: number)
{
	return (num << cnt) | (num >>> (32 - cnt));
}

/**
 * Calculate the SHA-1 of a string or ArrayLike<Int>
*/
export default function sha1(s: string | ArrayLike<Int>): buffer.Buffer {
	var b = buffer.from(s as any);
	return binl2bin(core_sha1(bin2binl(b), b.length * chrsz));
}