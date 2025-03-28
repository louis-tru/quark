/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

// Despite the name and location, this is portable code.

#include "./android_font_parser.h"

#include <expat.h>

#include <stdlib.h>
#include <string.h>

#include <memory>

#define LMP_SYSTEM_FONTS_FILE "/system/etc/fonts.xml"
#define OLD_SYSTEM_FONTS_FILE "/system/etc/system_fonts.xml"
#define FALLBACK_FONTS_FILE "/system/etc/fallback_fonts.xml"
#define VENDOR_FONTS_FILE "/vendor/etc/fallback_fonts.xml"

#define LOCALE_FALLBACK_FONTS_SYSTEM_DIR "/system/etc"
#define LOCALE_FALLBACK_FONTS_VENDOR_DIR "/vendor/etc"
#define LOCALE_FALLBACK_FONTS_PREFIX "fallback_fonts-"
#define LOCALE_FALLBACK_FONTS_SUFFIX ".xml"

#ifndef Qk_FONT_FILE_PREFIX
#    define Qk_FONT_FILE_PREFIX "/fonts/"
#endif

#include "../../../util/fs.h"

/**
 * This file contains TWO 'familieset' handlers:
 * One for JB and earlier which works with
 *   /system/etc/system_fonts.xml
 *   /system/etc/fallback_fonts.xml
 *   /vendor/etc/fallback_fonts.xml
 *   /system/etc/fallback_fonts-XX.xml
 *   /vendor/etc/fallback_fonts-XX.xml
 * and the other for LMP and later which works with
 *   /system/etc/fonts.xml
 *
 * If the 'familieset' 'version' attribute is 21 or higher the LMP parser is used, otherwise the JB.
 */

struct FamilyData;

struct TagHandler {
	/** Called at the start tag.
	 *  Called immediately after the parent tag retuns this handler from a call to 'tag'.
	 *  Allows setting up for handling the tag content and processing attributes.
	 *  If nullptr, will not be called.
	 */
	void (*start)(FamilyData* data, cChar* tag, cChar** attributes);

	/** Called at the end tag.
	 *  Allows post-processing of any accumulated information.
	 *  This will be the last call made in relation to the current tag.
	 *  If nullptr, will not be called.
	 */
	void (*end)(FamilyData* data, cChar* tag);

	/** Called when a nested tag is encountered.
	 *  This is responsible for determining how to handle the tag.
	 *  If the tag is not recognized, return nullptr to skip the tag.
	 *  If nullptr, all nested tags will be skipped.
	 */
	const TagHandler* (*tag)(FamilyData* data, cChar* tag, cChar** attributes);

	/** The character handler for this tag.
	 *  This is only active for character data contained directly in this tag (not sub-tags).
	 *  The first parameter will be castable to a FamilyData*.
	 *  If nullptr, any character data in this tag will be ignored.
	 */
	XML_CharacterDataHandler chars;
};

/** Represents the current parsing state. */
struct FamilyData {
	FamilyData(XML_Parser parser, Array<FontFamily*>& families,
					cString& basePath, bool isFallback, cChar* filename,
					const TagHandler* topLevelHandler)
		: fParser(parser)
		, fFamilies(families)
		, fCurrentFamily(nullptr)
		, fCurrentFontInfo(nullptr)
		, fVersion(0)
		, fBasePath(basePath)
		, fIsFallback(isFallback)
		, fFilename(filename)
		, fDepth(1)
		, fQkip(0)
		, fHandler({topLevelHandler})
	{}

	XML_Parser fParser;                         // The expat parser doing the work, owned by caller
	Array<FontFamily*>& fFamilies;          // The array to append families, owned by caller
	Sp<FontFamily> fCurrentFamily;          // The families being created, owned by this
	FontFileInfo* fCurrentFontInfo;             // The info being created, owned by fCurrentFamily
	int fVersion;                               // The version of the file parsed.
	cString& fBasePath;                  // The current base path.
	const bool fIsFallback;                     // The file being parsed is a fallback file
	cChar* fFilename;                      // The name of the file currently being parsed.

	int fDepth;                                 // The current element depth of the parse.
	int fQkip;                                  // The depth to stop skipping, 0 if not skipping.
	Array<const TagHandler*> fHandler;      // The stack of current tag handlers.
};

static bool memeq(cChar* s1, cChar* s2, size_t n1, size_t n2) {
	return n1 == n2 && 0 == memcmp(s1, s2, n1);
}
#define MEMEQ(c, s, n) memeq(c, s, sizeof(c) - 1, n)

#define ATTS_NON_NULL(a, i) (a[i] != nullptr && a[i+1] != nullptr)

#define Qk_FONTMGR_ANDROID_PARSER_PREFIX "[QkFontMgr Android Parser] "

#define Qk_FONTCONFIGPARSER_WARNING(message, ...)                                 \
	Qk_DLog(Qk_FONTMGR_ANDROID_PARSER_PREFIX "%s:%d:%d: warning: " message "\n", \
			 self->fFilename,                                                     \
			 (int)XML_GetCurrentLineNumber(self->fParser),                        \
			 (int)XML_GetCurrentColumnNumber(self->fParser),                      \
			 ##__VA_ARGS__)

static bool is_whitespace(char c) {
	return c == ' ' || c == '\n'|| c == '\r' || c == '\t';
}

static void trim_string(String* s) {
	*s = s->trim();
}

namespace lmpParser {

	static const TagHandler axisHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			FontFileInfo& file = *self->fCurrentFontInfo;
			FontByteTag axisTag = QkSetFourByteTag('\0','\0','\0','\0');
			QkFixed axisStyleValue = 0;
			bool axisTagIsValid = false;
			bool axisStyleValueIsValid = false;
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* name = attributes[i];
				cChar* value = attributes[i+1];
				size_t nameLen = strlen(name);
				if (MEMEQ("tag", name, nameLen)) {
					size_t valueLen = strlen(value);
					if (valueLen == 4) {
						axisTag = QkSetFourByteTag(value[0], value[1], value[2], value[3]);
						axisTagIsValid = true;
						for (int j = 0; j < file.fVariationDesignPosition.length() - 1; ++j) {
							if (file.fVariationDesignPosition[j].axis == axisTag) {
								axisTagIsValid = false;
								Qk_FONTCONFIGPARSER_WARNING("'%c%c%c%c' axis specified more than once",
															(axisTag >> 24) & 0xFF,
															(axisTag >> 16) & 0xFF,
															(axisTag >>  8) & 0xFF,
															(axisTag      ) & 0xFF);
							}
						}
					} else {
						Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid axis tag", value);
					}
				} else if (MEMEQ("stylevalue", name, nameLen)) {
					if (parse_fixed<16>(value, &axisStyleValue)) {
						axisStyleValueIsValid = true;
					} else {
						Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid axis stylevalue", value);
					}
				}
			}
			if (axisTagIsValid && axisStyleValueIsValid) {
				file.fVariationDesignPosition.push({ axisTag, QkFixedToScalar(axisStyleValue) });
			}
		},
		/*end*/nullptr,
		/*tag*/nullptr,
		/*chars*/nullptr,
	};

	static const TagHandler fontHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			// 'weight' (non-negative integer) [default 0]
			// 'style' ("normal", "italic") [default "auto"]
			// 'index' (non-negative integer) [default 0]
			// The character data should be a filename.
			FontFileInfo& file = self->fCurrentFamily->fFonts.push({});
			self->fCurrentFontInfo = &file;
			String fallbackFor;
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* name = attributes[i];
				cChar* value = attributes[i+1];
				size_t nameLen = strlen(name);
				if (MEMEQ("weight", name, nameLen)) {
					if (!parse_non_negative_integer(value, &file.fWeight)) {
						Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid weight", value);
					}
				} else if (MEMEQ("style", name, nameLen)) {
					size_t valueLen = strlen(value);
					if (MEMEQ("normal", value, valueLen)) {
						file.fStyle = FontFileInfo::Style::kNormal;
					} else if (MEMEQ("italic", value, valueLen)) {
						file.fStyle = FontFileInfo::Style::kItalic;
					}
				} else if (MEMEQ("index", name, nameLen)) {
					if (!parse_non_negative_integer(value, &file.fIndex)) {
						Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid index", value);
					}
				} else if (MEMEQ("fallbackFor", name, nameLen)) {
					/** fallbackFor specifies a families fallback and should have been on families. */
					fallbackFor = value;
				}
			}
			if (!fallbackFor.isEmpty()) {
				Sp<FontFamily> *fallbackFamily;
				if (!self->fCurrentFamily->fallbackFamilies.get(fallbackFor, fallbackFamily)) {
					fallbackFamily =
					&self->fCurrentFamily->fallbackFamilies.set(
							fallbackFor, new FontFamily(self->fCurrentFamily->fBasePath, true));
					(*fallbackFamily)->fLanguages = self->fCurrentFamily->fLanguages;
					(*fallbackFamily)->fVariant = self->fCurrentFamily->fVariant;
					(*fallbackFamily)->fOrder = self->fCurrentFamily->fOrder;
					(*fallbackFamily)->fFallbackFor = fallbackFor;
				}
				self->fCurrentFontInfo = &(*fallbackFamily)->fFonts.push(std::move(file));
				self->fCurrentFamily->fFonts.pop();
			}
		},
		/*end*/[](FamilyData* self, cChar* tag) {
			trim_string(&self->fCurrentFontInfo->fFileName);
		},
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("axis", tag, len)) {
				return &axisHandler;
			}
			return nullptr;
		},
		/*chars*/[](void* data, cChar* s, int len) {
			FamilyData* self = static_cast<FamilyData*>(data);
			self->fCurrentFontInfo->fFileName.append(s, len);
		}
	};

	static const TagHandler familiesHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			// 'name' (string) [optional]
			// 'lang' (space separated string) [default ""]
			// 'variant' ("elegant", "compact") [default "default"]
			// If there is no name, this is a fallback only font.
			FontFamily* families = new FontFamily(self->fBasePath, true);
			self->fCurrentFamily = families;
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* name = attributes[i];
				cChar* value = attributes[i+1];
				size_t nameLen = strlen(name);
				size_t valueLen = strlen(value);
				if (MEMEQ("name", name, nameLen)) {
					families->fNames.push(String(value, valueLen).lowerCase());
					families->fIsFallbackFont = false;
				} else if (MEMEQ("lang", name, nameLen)) {
					size_t i = 0;
					while (true) {
						for (; i < valueLen && is_whitespace(value[i]); ++i) { }
						if (i == valueLen) { break; }
						size_t j;
						for (j = i + 1; j < valueLen && !is_whitespace(value[j]); ++j) { }
						families->fLanguages.push(String(value + i, j - i));
						i = j;
						if (i == valueLen) { break; }
					}
				} else if (MEMEQ("variant", name, nameLen)) {
					if (MEMEQ("elegant", value, valueLen)) {
						families->fVariant = kElegant_FontVariant;
					} else if (MEMEQ("compact", value, valueLen)) {
						families->fVariant = kCompact_FontVariant;
					}
				}
			}
		},
		/*end*/[](FamilyData* self, cChar* tag) {
			self->fFamilies.push(self->fCurrentFamily.collapse());
		},
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("font", tag, len)) {
				return &fontHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

	static FontFamily* find_families(FamilyData* self, cString& familiesName) {
		for (int i = 0; i < self->fFamilies.length(); i++) {
			FontFamily* candidate = self->fFamilies[i];
			for (int j = 0; j < candidate->fNames.length(); j++) {
				if (candidate->fNames[j] == familiesName) {
					return candidate;
				}
			}
		}
		return nullptr;
	}

	static const TagHandler aliasHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			// 'name' (string) introduces a new families name.
			// 'to' (string) specifies which (previous) families to alias
			// 'weight' (non-negative integer) [optional]
			// If it *does not* have a weight, 'name' is an alias for the entire 'to' families.
			// If it *does* have a weight, 'name' is a new families consisting of
			// the font(s) with 'weight' from the 'to' families.

			String aliasName;
			String to;
			int weight = 0;
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* name = attributes[i];
				cChar* value = attributes[i+1];
				size_t nameLen = strlen(name);
				if (MEMEQ("name", name, nameLen)) {
					aliasName = String(value).lowerCase();
				} else if (MEMEQ("to", name, nameLen)) {
					to = value;
				} else if (MEMEQ("weight", name, nameLen)) {
					if (!parse_non_negative_integer(value, &weight)) {
						Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid weight", value);
					}
				}
			}

			// Assumes that the named families is already declared
			FontFamily* targetFamily = find_families(self, to);
			if (!targetFamily) {
				Qk_FONTCONFIGPARSER_WARNING("'%s' alias target not found", to.c_str());
				return;
			}

			if (weight) {
				FontFamily* families = new FontFamily(targetFamily->fBasePath, self->fIsFallback);
				families->fNames.push(aliasName);

				for (int i = 0; i < targetFamily->fFonts.length(); i++) {
					if (targetFamily->fFonts[i].fWeight == weight) {
						families->fFonts.push(targetFamily->fFonts[i]);
					}
				}
				self->fFamilies.push(families);
			} else {
				targetFamily->fNames.push(aliasName);
			}
		},
		/*end*/nullptr,
		/*tag*/nullptr,
		/*chars*/nullptr,
	};

	static const TagHandler familiesSetHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) { },
		/*end*/nullptr,
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("families", tag, len)) {
				return &familiesHandler;
			} else if (MEMEQ("alias", tag, len)) {
				return &aliasHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

}  // namespace lmpParser

namespace jbParser {

	static const TagHandler fileHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			// 'variant' ("elegant", "compact") [default "default"]
			// 'lang' (string) [default ""]
			// 'index' (non-negative integer) [default 0]
			// The character data should be a filename.
			auto& currentFamily = **self->fCurrentFamily;
			auto& newFileInfo = currentFamily.fFonts.push({});
			if (attributes) {
				for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
					cChar* name = attributes[i];
					cChar* value = attributes[i+1];
					size_t nameLen = strlen(name);
					size_t valueLen = strlen(value);
					if (MEMEQ("variant", name, nameLen)) {
						const FontVariant prevVariant = currentFamily.fVariant;
						if (MEMEQ("elegant", value, valueLen)) {
							currentFamily.fVariant = kElegant_FontVariant;
						} else if (MEMEQ("compact", value, valueLen)) {
							currentFamily.fVariant = kCompact_FontVariant;
						}
						if (currentFamily.fFonts.length() > 1 && currentFamily.fVariant != prevVariant) {
							Qk_FONTCONFIGPARSER_WARNING("'%s' unexpected variant found\n"
								"Note: Every font file within a families must have identical variants.",
								value);
						}

					} else if (MEMEQ("lang", name, nameLen)) {
						QkLanguage currentLanguage(value, valueLen);
						bool showWarning = false;
						if (currentFamily.fLanguages.isNull()) {
							showWarning = (currentFamily.fFonts.length() > 1);
							currentFamily.fLanguages.push(std::move(currentLanguage));
						} else if (currentFamily.fLanguages[0] != currentLanguage) {
							showWarning = true;
							currentFamily.fLanguages[0] = std::move(currentLanguage);
						}
						if (showWarning) {
							Qk_FONTCONFIGPARSER_WARNING("'%s' unexpected language found\n"
								"Note: Every font file within a families must have identical languages.",
								value);
						}

					} else if (MEMEQ("index", name, nameLen)) {
						if (!parse_non_negative_integer(value, &newFileInfo.fIndex)) {
							Qk_FONTCONFIGPARSER_WARNING("'%s' is an invalid index", value);
						}
					}
				}
			}
			self->fCurrentFontInfo = &newFileInfo;
		},
		/*end*/nullptr,
		/*tag*/nullptr,
		/*chars*/[](void* data, cChar* s, int len) {
			FamilyData* self = static_cast<FamilyData*>(data);
			self->fCurrentFontInfo->fFileName.append(s, len);
		}
	};

	static const TagHandler fileSetHandler = {
		/*start*/nullptr,
		/*end*/nullptr,
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("file", tag, len)) {
				return &fileHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

	static const TagHandler nameHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			// The character data should be a name for the font.
			self->fCurrentFamily->fNames.push(String());
		},
		/*end*/nullptr,
		/*tag*/nullptr,
		/*chars*/[](void* data, cChar* s, int len) {
			FamilyData* self = static_cast<FamilyData*>(data);
			String tolc(s, len);
			self->fCurrentFamily->fNames.back().append(tolc.lowerCase());
		}
	};

	static const TagHandler nameSetHandler = {
		/*start*/nullptr,
		/*end*/nullptr,
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("name", tag, len)) {
				return &nameHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

	static const TagHandler familiesHandler = {
		/*start*/[](FamilyData* self, cChar* tag, cChar** attributes) {
			self->fCurrentFamily = new FontFamily(self->fBasePath, self->fIsFallback);
			// 'order' (non-negative integer) [default -1]
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* value = attributes[i+1];
				parse_non_negative_integer(value, &self->fCurrentFamily->fOrder);
			}
		},
		/*end*/[](FamilyData* self, cChar* tag) {
			self->fFamilies.push(self->fCurrentFamily.collapse());
		},
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("nameset", tag, len)) {
				return &nameSetHandler;
			} else if (MEMEQ("fileset", tag, len)) {
				return &fileSetHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

	static const TagHandler familiesSetHandler = {
		/*start*/nullptr,
		/*end*/nullptr,
		/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
			size_t len = strlen(tag);
			if (MEMEQ("families", tag, len)) {
				return &familiesHandler;
			}
			return nullptr;
		},
		/*chars*/nullptr,
	};

} // namespace jbParser

static const TagHandler topLevelHandler = {
	/*start*/nullptr,
	/*end*/nullptr,
	/*tag*/[](FamilyData* self, cChar* tag, cChar** attributes) -> const TagHandler* {
		size_t len = strlen(tag);
		if (MEMEQ("familieset", tag, len)) {
			// 'version' (non-negative integer) [default 0]
			for (size_t i = 0; ATTS_NON_NULL(attributes, i); i += 2) {
				cChar* name = attributes[i];
				size_t nameLen = strlen(name);
				if (MEMEQ("version", name, nameLen)) {
					cChar* value = attributes[i+1];
					if (parse_non_negative_integer(value, &self->fVersion)) {
						if (self->fVersion >= 21) {
							return &lmpParser::familiesSetHandler;
						}
					}
				}
			}
			return &jbParser::familiesSetHandler;
		}
		return nullptr;
	},
	/*chars*/nullptr,
};

static void XMLCALL start_element_handler(void *data, cChar *tag, cChar **attributes) {
	FamilyData* self = static_cast<FamilyData*>(data);

	if (!self->fQkip) {
		const TagHandler* parent = self->fHandler.back();
		const TagHandler* child = parent->tag ? parent->tag(self, tag, attributes) : nullptr;
		if (child) {
			if (child->start) {
				child->start(self, tag, attributes);
			}
			self->fHandler.push(child);
			XML_SetCharacterDataHandler(self->fParser, child->chars);
		} else {
			Qk_FONTCONFIGPARSER_WARNING("'%s' tag not recognized, skipping", tag);
			XML_SetCharacterDataHandler(self->fParser, nullptr);
			self->fQkip = self->fDepth;
		}
	}

	++self->fDepth;
}

static void XMLCALL end_element_handler(void* data, cChar* tag) {
	FamilyData* self = static_cast<FamilyData*>(data);
	--self->fDepth;

	if (!self->fQkip) {
		const TagHandler* child = self->fHandler.back();
		if (child->end) {
			child->end(self, tag);
		}
		self->fHandler.pop();
		const TagHandler* parent = self->fHandler.back();
		XML_SetCharacterDataHandler(self->fParser, parent->chars);
	}

	if (self->fQkip == self->fDepth) {
		self->fQkip = 0;
		const TagHandler* parent = self->fHandler.back();
		XML_SetCharacterDataHandler(self->fParser, parent->chars);
	}
}

static void XMLCALL xml_entity_decl_handler(void *data,
											const XML_Char *entityName,
											int is_parameter_entity,
											const XML_Char *value,
											int value_length,
											const XML_Char *base,
											const XML_Char *systemId,
											const XML_Char *publicId,
											const XML_Char *notationName)
{
	FamilyData* self = static_cast<FamilyData*>(data);
	Qk_FONTCONFIGPARSER_WARNING("'%s' entity declaration found, stopping processing", entityName);
	XML_StopParser(self->fParser, XML_FALSE);
}

static const XML_Memory_Handling_Suite qk_XML_alloc = {
	::malloc,//qk_malloc_throw,
	::realloc,//qk_realloc_throw,
	::free //sk_free
};

/**
 * This function parses the given filename and stores the results in the given
 * families array. Returns the version of the file, negative if the file does not exist.
 */
static int parse_config_file(cChar* filename, Array<FontFamily*>& families,
							 cString& basePath, bool isFallback)
{
	FileSync file(filename);
	// Some of the files we attempt to parse (in particular, /vendor/etc/fallback_fonts.xml)
	// are optional - failure here is okay because one of these optional files may not exist.
	if (!file.open()) {
		Qk_DLog(Qk_FONTMGR_ANDROID_PARSER_PREFIX "'%s' could not be opened\n", filename);
		return -1;
	}

	CPointerHold<std::remove_pointer_t<XML_Parser>> parser(
		XML_ParserCreate_MM(nullptr, &qk_XML_alloc, nullptr), [](auto p) {
		XML_ParserFree(p);
	});
	if (!parser) {
		Qk_DLog(Qk_FONTMGR_ANDROID_PARSER_PREFIX "could not create XML parser\n");
		return -1;
	}

	FamilyData self(*parser, families, basePath, isFallback, filename, &topLevelHandler);
	XML_SetUserData(*parser, &self);

	// Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340
	XML_SetEntityDeclHandler(*parser, xml_entity_decl_handler);

	// Start parsing oldschool; switch these in flight if we detect a newer version of the file.
	XML_SetElementHandler(*parser, start_element_handler, end_element_handler);

	// One would assume it would be faster to have a buffer on the stack and call XML_Parse.
	// But XML_Parse will call XML_GetBuffer anyway and memmove the passed buffer into it.
	// (Unless XML_CONTEXT_BYTES is undefined, but all users define it.)
	// In debug, buffer a small odd number of bytes to detect slicing in XML_CharacterDataHandler.
	static const int bufferSize = 512;// QkDEBUGCODE( - 507);
	bool done = false;
	while (!done) {
		void* buffer = XML_GetBuffer(*parser, bufferSize);
		if (!buffer) {
			Qk_DLog(Qk_FONTMGR_ANDROID_PARSER_PREFIX "could not buffer enough to continue\n");
			return -1;
		}
		int len = file.read(buffer, bufferSize);
		done = len < bufferSize;
		XML_Status status = XML_ParseBuffer(*parser, len, done);
		if (XML_STATUS_ERROR == status) {
			XML_Error error = XML_GetErrorCode(*parser);
			int line = XML_GetCurrentLineNumber(*parser);
			int column = XML_GetCurrentColumnNumber(*parser);
			const XML_LChar* errorString = XML_ErrorString(error);
			Qk_DLog(Qk_FONTMGR_ANDROID_PARSER_PREFIX "%s:%d:%d error %d: %s.\n",
					 filename, line, column, error, errorString);
			return -1;
		}
	}
	return self.fVersion;
}

/** Returns the version of the system font file actually found, negative if none. */
static int append_system_font_families(Array<FontFamily*>& fontFamilies, cString& basePath)
{
	int initialCount = fontFamilies.length();
	int version = parse_config_file(LMP_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
	if (version < 0 || fontFamilies.length() == initialCount) {
		version = parse_config_file(OLD_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
	}
	return version;
}

/**
 * In some versions of Android prior to Android 4.2 (JellyBean MR1 at API
 * Level 17) the fallback fonts for certain locales were encoded in their own
 * XML files with a suffix that identified the locale.  We search the provided
 * directory for those files,add all of their entries to the fallback chain, and
 * include the locale as part of each entry.
 */
static void append_fallback_font_families_for_locale(Array<FontFamily*>& fallbackFonts,
													cChar* dir,
													cString& basePath)
{
	for (auto &dirent: fs_readdir_sync(dir)) {
		auto &fileName = dirent.name;
		// The size of the prefix and suffix.
		static const size_t fixedLen = sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1
									 + sizeof(LOCALE_FALLBACK_FONTS_SUFFIX) - 1;

		// The size of the prefix, suffix, and a minimum valid language code
		static const size_t minSize = fixedLen + 2;

		if (fileName.length() < minSize ||
			!fileName.startsWith(LOCALE_FALLBACK_FONTS_PREFIX) ||
			!fileName.endsWith(LOCALE_FALLBACK_FONTS_SUFFIX))
		{
			continue;
		}

		String locale(fileName.c_str() + sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1,
						fileName.size() - fixedLen);

		auto absoluteFilename = String::format("%s/%s", dir, fileName.c_str());

		Array<FontFamily*> langSpecificFonts;
		parse_config_file(absoluteFilename.c_str(), langSpecificFonts, basePath, true);

		for (auto families: langSpecificFonts) {
			fallbackFonts.push(families)->fLanguages.push(locale);
		}
	}
}

static void append_system_fallback_font_families(Array<FontFamily*>& fallbackFonts,
												cString& basePath)
{
	parse_config_file(FALLBACK_FONTS_FILE, fallbackFonts, basePath, true);
	append_fallback_font_families_for_locale(fallbackFonts,
											 LOCALE_FALLBACK_FONTS_SYSTEM_DIR,
											 basePath);
}

static void mixin_vendor_fallback_font_families(Array<FontFamily*>& fallbackFonts,
												cString& basePath)
{
	Array<FontFamily*> vendorFonts;
	parse_config_file(VENDOR_FONTS_FILE, vendorFonts, basePath, true);
	append_fallback_font_families_for_locale(vendorFonts,
											 LOCALE_FALLBACK_FONTS_VENDOR_DIR,
											 basePath);

	if (vendorFonts.length() == 0)
		return;

	// Sorting array
	for (auto families: vendorFonts) {
		fallbackFonts.push(families);
		for (auto i = fallbackFonts.length() - 1; i > 0; i++) {
			auto familiesPrev = fallbackFonts[i - 1];
			if (families->fOrder < familiesPrev->fOrder) {
				fallbackFonts[i] = familiesPrev; // swap
				fallbackFonts[i - 1] = families;
			} else {
				break;
			}
		}
	}
}

void GetSystemFontFamilies(Array<FontFamily*>& fontFamilies) {
	// Version 21 of the system font configuration does not need any fallback configuration files.
	String basePath(getenv("ANDROID_ROOT"));
	basePath.append(Qk_FONT_FILE_PREFIX, sizeof(Qk_FONT_FILE_PREFIX) - 1);

	if (append_system_font_families(fontFamilies, basePath) >= 21) {
		return;
	}

	// Append all the fallback fonts to system fonts
	Array<FontFamily*> fallbackFonts;
	append_system_fallback_font_families(fallbackFonts, basePath);
	mixin_vendor_fallback_font_families(fallbackFonts, basePath);
	fontFamilies.concat(std::move(fallbackFonts));
}

void GetCustomFontFamilies(Array<FontFamily*>& fontFamilies,
													cString& basePath,
													cChar* fontsXml,
													cChar* fallbackFontsXml,
													cChar* langFallbackFontsDir)
{
	if (fontsXml) {
		parse_config_file(fontsXml, fontFamilies, basePath, false);
	}
	if (fallbackFontsXml) {
		parse_config_file(fallbackFontsXml, fontFamilies, basePath, true);
	}
	if (langFallbackFontsDir) {
		append_fallback_font_families_for_locale(fontFamilies,
												langFallbackFontsDir,
												basePath);
	}
}

QkLanguage QkLanguage::getParent() const {
	Qk_ASSERT(!fTag.isEmpty());
	cChar* tag = fTag.c_str();

	// strip off the rightmost "-.*"
	cChar* parentTagEnd = strrchr(tag, '-');
	if (parentTagEnd == nullptr) {
		return QkLanguage();
	}
	size_t parentTagLen = parentTagEnd - tag;
	return QkLanguage(tag, parentTagLen);
}
