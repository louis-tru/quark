/**
 * @license
 * lodash 3.10.1 (Custom Build) <https://lodash.com/>
 * Build: `lodash modern -d -o ./index.js`
 * Copyright 2012-2015 The Dojo Foundation <http://dojofoundation.org/>
 * Based on Underscore.js 1.8.3 <http://underscorejs.org/LICENSE>
 * Copyright 2009-2015 Jeremy Ashkenas, DocumentCloud and Investigative Reporters & Editors
 * Available under MIT license <https://lodash.com/license>
 */

var lodash = {};

/** Used to make template sourceURLs easier to identify. */
var templateCounter = -1;

/** Used to match empty string literals in compiled template source. */
var reEmptyStringLeading = /\b__p \+= '';/g,
		reEmptyStringMiddle = /\b(__p \+=) '' \+/g,
		reEmptyStringTrailing = /(__e\(.*?\)|\b__t\)) \+\n'';/g;

/** Used to match template delimiters. */
var reEscape = /<%-([\s\S]+?)%>/g,
		reEvaluate = /<%([\s\S]+?)%>/g,
		reInterpolate = /<%=([\s\S]+?)%>/g;

/** Used to ensure capturing order of template delimiters. */
var reNoMatch = /($^)/;

/**
 * Used to match
 * [ES template delimiters](http://ecma-international.org/ecma-262/7.0/#sec-template-literal-lexical-components).
 */
var reEsTemplate = /\$\{([^\\}]*(?:\\.[^\\}]*)*)\}/g;

/** Used to match unescaped characters in compiled string literals. */
var reUnescapedString = /['\n\r\u2028\u2029\\]/g;


/**
 * By default, the template delimiters used by lodash are like those in
 * embedded Ruby (ERB) as well as ES2015 template strings. Change the
 * following template settings to use alternative delimiters.
 *
 * @static
 * @memberOf _
 * @type {Object}
 */
lodash.templateSettings = {

	/**
	 * Used to detect `data` property values to be HTML-escaped.
	 *
	 * @memberOf _.templateSettings
	 * @type {RegExp}
	 */
	'escape': reEscape,

	/**
	 * Used to detect code to be evaluated.
	 *
	 * @memberOf _.templateSettings
	 * @type {RegExp}
	 */
	'evaluate': reEvaluate,

	/**
	 * Used to detect `data` property values to inject.
	 *
	 * @memberOf _.templateSettings
	 * @type {RegExp}
	 */
	'interpolate': reInterpolate,

	/**
	 * Used to reference the data object in the template text.
	 *
	 * @memberOf _.templateSettings
	 * @type {string}
	 */
	'variable': '',

	/**
	 * Used to import variables into the compiled template.
	 *
	 * @memberOf _.templateSettings
	 * @type {Object}
	 */
	'imports': {

		/**
		 * A reference to the `lodash` function.
		 *
		 * @memberOf _.templateSettings.imports
		 * @type {Function}
		 */
		'_': lodash
	}
};

/** Used to escape characters for inclusion in compiled string literals. */
var stringEscapes = {
	'\\': '\\',
	"'": "'",
	'\n': 'n',
	'\r': 'r',
	'\u2028': 'u2028',
	'\u2029': 'u2029'
};

/**
 * Checks if `value` is the
 * [language type](http://www.ecma-international.org/ecma-262/7.0/#sec-ecmascript-language-types)
 * of `Object`. (e.g. arrays, functions, objects, regexes, `new Number(0)`, and `new String('')`)
 *
 * @static
 * @memberOf _
 * @since 0.1.0
 * @category Lang
 * @param {*} value The value to check.
 * @returns {boolean} Returns `true` if `value` is an object, else `false`.
 * @example
 *
 * _.isObject({});
 * // => true
 *
 * _.isObject([1, 2, 3]);
 * // => true
 *
 * _.isObject(_.noop);
 * // => true
 *
 * _.isObject(null);
 * // => false
 */
function isObject(value) {
	var type = typeof value;
	return value != null && (type == 'object' || type == 'function');
}

/**
 * Checks if the given arguments are from an iteratee call.
 *
 * @private
 * @param {*} value The potential iteratee value argument.
 * @param {*} index The potential iteratee index or key argument.
 * @param {*} object The potential iteratee object argument.
 * @returns {boolean} Returns `true` if the arguments are from an iteratee call,
 *  else `false`.
 */
function isIterateeCall(value, index, object) {
	if (!isObject(object)) {
		return false;
	}
	var type = typeof index;
	if (type == 'number'
				? ( typeof object.length == 'number' && index < object.length)
				: (type == 'string' && index in object)
			) {
		return object[index] === value;
	}
	return false;
}

/**
 * Used by `_.template` to escape characters for inclusion in compiled string literals.
 *
 * @private
 * @param {string} chr The matched character to escape.
 * @returns {string} Returns the escaped character.
 */
function escapeStringChar(chr) {
	return '\\' + stringEscapes[chr];
}

/**
 * Creates a compiled template function that can interpolate data properties
 * in "interpolate" delimiters, HTML-escape interpolated data properties in
 * "escape" delimiters, and execute JavaScript in "evaluate" delimiters. Data
 * properties may be accessed as free variables in the template. If a setting
 * object is given, it takes precedence over `_.templateSettings` values.
 *
 * **Note:** In the development build `_.template` utilizes
 * [sourceURLs](http://www.html5rocks.com/en/tutorials/developertools/sourcemaps/#toc-sourceurl)
 * for easier debugging.
 *
 * For more information on precompiling templates see
 * [lodash's custom builds documentation](https://lodash.com/custom-builds).
 *
 * For more information on Chrome extension sandboxes see
 * [Chrome's extensions documentation](https://developer.chrome.com/extensions/sandboxingEval).
 *
 * @static
 * @since 0.1.0
 * @memberOf _
 * @category String
 * @param {string} [string=''] The template string.
 * @param {Object} [options={}] The options object.
 * @param {RegExp} [options.escape=_.templateSettings.escape]
 *  The HTML "escape" delimiter.
 * @param {RegExp} [options.evaluate=_.templateSettings.evaluate]
 *  The "evaluate" delimiter.
 * @param {Object} [options.imports=_.templateSettings.imports]
 *  An object to import into the template as free variables.
 * @param {RegExp} [options.interpolate=_.templateSettings.interpolate]
 *  The "interpolate" delimiter.
 * @param {string} [options.sourceURL='lodash.templateSources[n]']
 *  The sourceURL of the compiled template.
 * @param {string} [options.variable='obj']
 *  The data object variable name.
 * @param- {Object} [guard] Enables use as an iteratee for methods like `_.map`.
 * @returns {Function} Returns the compiled template function.
 * @example
 *
 * // Use the "interpolate" delimiter to create a compiled template.
 * var compiled = _.template('hello <%= user %>!');
 * compiled({ 'user': 'fred' });
 * // => 'hello fred!'
 *
 * // Use the HTML "escape" delimiter to escape data property values.
 * var compiled = _.template('<b><%- value %></b>');
 * compiled({ 'value': '<script>' });
 * // => '<b>&lt;script&gt;</b>'
 *
 * // Use the "evaluate" delimiter to execute JavaScript and generate HTML.
 * var compiled = _.template('<% _.forEach(users, function(user) { %><li><%- user %></li><% }); %>');
 * compiled({ 'users': ['fred', 'barney'] });
 * // => '<li>fred</li><li>barney</li>'
 *
 * // Use the internal `print` function in "evaluate" delimiters.
 * var compiled = _.template('<% print("hello " + user); %>!');
 * compiled({ 'user': 'barney' });
 * // => 'hello barney!'
 *
 * // Use the ES template literal delimiter as an "interpolate" delimiter.
 * // Disable support by replacing the "interpolate" delimiter.
 * var compiled = _.template('hello ${ user }!');
 * compiled({ 'user': 'pebbles' });
 * // => 'hello pebbles!'
 *
 * // Use backslashes to treat delimiters as plain text.
 * var compiled = _.template('<%= "\\<%- value %\\>" %>');
 * compiled({ 'value': 'ignored' });
 * // => '<%- value %>'
 *
 * // Use the `imports` option to import `jQuery` as `jq`.
 * var text = '<% jq.each(users, function(user) { %><li><%- user %></li><% }); %>';
 * var compiled = _.template(text, { 'imports': { 'jq': jQuery } });
 * compiled({ 'users': ['fred', 'barney'] });
 * // => '<li>fred</li><li>barney</li>'
 *
 * // Use the `sourceURL` option to specify a custom sourceURL for the template.
 * var compiled = _.template('hello <%= user %>!', { 'sourceURL': '/basic/greeting.jst' });
 * compiled(data);
 * // => Find the source of "greeting.jst" under the Sources tab or Resources panel of the web inspector.
 *
 * // Use the `variable` option to ensure a with-statement isn't used in the compiled template.
 * var compiled = _.template('hi <%= data.user %>!', { 'variable': 'data' });
 * compiled.source;
 * // => function(data) {
 * //   var __t, __p = '';
 * //   __p += 'hi ' + ((__t = ( data.user )) == null ? '' : __t) + '!';
 * //   return __p;
 * // }
 *
 * // Use custom template delimiters.
 * _.templateSettings.interpolate = /{{([\s\S]+?)}}/g;
 * var compiled = _.template('hello {{ user }}!');
 * compiled({ 'user': 'mustache' });
 * // => 'hello mustache!'
 *
 * // Use the `source` property to inline compiled templates for meaningful
 * // line numbers in error messages and stack traces.
 * fs.writeFileSync(path.join(process.cwd(), 'jst.js'), '\
 *   var JST = {\
 *     "main": ' + _.template(mainText).source + '\
 *   };\
 * ');
 */
function template(string, options, guard) {
	// Based on John Resig's `tmpl` implementation
	// (http://ejohn.org/blog/javascript-micro-templating/)
	// and Laura Doktorova's doT.js (https://github.com/olado/doT).
	var settings = lodash.templateSettings;

	if (guard && isIterateeCall(string, options, guard)) {
		options = undefined;
	}
	string = String(string);
	options = { ...settings, ...options };

	var imports = { ...settings.imports, ...options.imports },
			importsKeys = Object.keys(imports),
			importsValues = Object.values(imports);

	var isEscaping,
			isEvaluating,
			index = 0,
			interpolate = options.interpolate || reNoMatch,
			source = "__p += '";

	// Compile the regexp to match each delimiter.
	var reDelimiters = RegExp(
		(options.escape || reNoMatch).source + '|' +
		interpolate.source + '|' +
		(interpolate === reInterpolate ? reEsTemplate : reNoMatch).source + '|' +
		(options.evaluate || reNoMatch).source + '|$'
	, 'g');

	// Use a sourceURL for easier debugging.
	var sourceURL = '//# sourceURL=' +
		('sourceURL' in options
			? options.sourceURL
			: ('lodash.templateSources[' + (++templateCounter) + ']')
		) + '\n';

	string.replace(reDelimiters, function(match, escapeValue, 
																				interpolateValue, esTemplateValue, 
																				evaluateValue, offset) 
	{
		interpolateValue || (interpolateValue = esTemplateValue);

		// Escape characters that can't be included in string literals.
		source += string.slice(index, offset).replace(reUnescapedString, escapeStringChar);

		// Replace delimiters with snippets.
		if (escapeValue) {
			isEscaping = true;
			source += "' +\n__e(" + escapeValue + ") +\n'";
		}
		if (evaluateValue) {
			isEvaluating = true;
			source += "';\n" + evaluateValue + ";\n__p += '";
		}
		if (interpolateValue) {
			source += "' +\n((__t = (" + interpolateValue + ")) == null ? '' : __t) +\n'";
		}
		index = offset + match.length;

		// The JS engine embedded in Adobe products needs `match` returned in
		// order to produce the correct `offset` value.
		return match;
	});

	source += "';\n";

	// If `variable` is not specified wrap a with-statement around the generated
	// code to add the data object to the top of the scope chain.
	var variable = options.variable;
	if (!variable) {
		source = 'with (obj) {\n' + source + '\n}\n';
	}
	// Cleanup code by stripping empty strings.
	source = (isEvaluating ? source.replace(reEmptyStringLeading, '') : source)
		.replace(reEmptyStringMiddle, '$1')
		.replace(reEmptyStringTrailing, '$1;');

	// Frame code as the function body.
	source = 'function(' + (variable || 'obj') + ') {\n' +
		(variable
			? ''
			: 'obj || (obj = {});\n'
		) +
		"var __t, __p = ''" +
		(isEscaping
			 ? ', __e = _.escape'
			 : ''
		) +
		(isEvaluating
			? ', __j = Array.prototype.join;\n' +
				"function print() { __p += __j.call(arguments, '') }\n"
			: ';\n'
		) +
		source +
		'return __p\n}';

	var result, err;

	try {
		result = Function(importsKeys, sourceURL + 'return ' + source).apply(undefined, importsValues)
	} catch (e) {
		result = err = util.err(e);
	}

	// Provide the compiled function's source by its `toString` method or
	// the `source` property as a convenience for inlining compiled templates.
	result.source = source;
	if (err) {
		throw result;
	}
	return result;
}

module.exports = template;
