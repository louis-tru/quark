/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

module.exports = {

	/**
	 * Calculates the index of the Array where item X should be placed, assuming the Array is sorted.
	 *
	 * @param  {Array}  array The array containing the items.
	 * @param  {Number} x     The item that needs to be added to the array.
	 * @param  {Number} low   Inital Index that is used to start searching, optional.
	 * @param  {Number} high  The maximum Index that is used to stop searching, optional.
	 * @return {Number} the   index where item X should be placed
	 * @static
	 */
	right: function (array, x, low, high) {
		// The low and high bounds the inital slice of the array that needs to be searched
		// this is optional
		low || (low = 0);
		high || (high = array.length);

		var mid;

		while (low < high) {
			mid = (low + high) >> 1;

			if (x < array[mid]) {
				high = mid;
			} else {
				low = mid + 1;
			}
		}

		return low;
	},

	/**
	* Calculates the index of the Array where item X should be placed, assuming the Array is sorted.
	* @param  {Array}  array The array containing the items.
	* @param  {number} x     The item that needs to be added to the array.
	* @param  {number} low   Inital Index that is used to start searching, optional.
	* @param  {number} high  The maximum Index that is used to stop searching, optional.
	* @return {number} the   index where item X should be placed
	*/
	left: function (array, x, low, high) {
		// The low and high bounds the inital slice of the array that needs to be searched
		// this is optional
		low || (low = 0);
		high || (high = array.length);

		var mid;

		while (low < high) {
			mid = (low + high) >> 1;

			if (x < array[mid]) {
				low = mid + 1;
			} else {
				high = mid;
			}
		}

		return low;
	}
};


