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

#include "./custom_typeface.h"
#include "../../../util/fs.h"

//#include <stdio.h>
//#include <sys/stat.h>

typedef QkFontPool_Custom::Families Families;
typedef const QkTypeface_FreeType::Scanner cScanner;

class DirectorySystemFontLoader : public QkFontPool_Custom::SystemFontLoader {
public:
	DirectorySystemFontLoader(cChar* dir) : fBaseDirectory(dir) {}
	
	static bool verifySuffix(cString& name) {
		static Array<String> suffixs{".ttf",".ttc",".otf",".pfb"};
		for (auto &s: suffixs) {
			if (name.endsWith(s))
				return true;
		}
		return false;
	}

	void loadSystemFonts(cScanner& scanner, Families* families) const override {
		load_directory_fonts(scanner, fBaseDirectory, families);
	}

private:
	static FontStyleSet_Custom* find_family(Families& families, cString &familyName)
	{
		for (int i = 0; i < families.length(); ++i) {
			if (families[i]->getFamilyName() == familyName) {
				return families[i].get();
			}
		}
		return nullptr;
	}

	static void load_directory_fonts(
		cScanner& scanner, cString& directory, Families* families)
	{
		for (auto &d: fs_readdir_sync(directory)) {
			auto filename = d.pathname;
			if (d.type == FTYPE_DIR) {
				if (!d.name.startsWith(".")) {
					load_directory_fonts(scanner, filename, families);
				}
				continue;
			}

			if (!verifySuffix(d.name)) {
				continue;
			}

			Sp<QkStream> stream;
			Qk_Try(stream = QkStream::Make(fs_read_file_sync(filename))) {
				Qk_DLog("---- failed to open <%s>\n", err.message().c_str());
				continue;
			}
			Qk_ASSERT(stream);
			//Qk_DLog("---- open <%s>\n", filename.c_str());

			int numFaces;
			if (!scanner.recognizedFont(stream.get(), &numFaces)) {
				// Qk_DLog("---- failed to open <%s> as a font\n", filename.c_str());
				continue;
			}

			for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
				bool isFixedPitch;
				String realname;
				FontStyle style = FontStyle(); // avoid uninitialized warning
				if (!scanner.scanFont(stream.get(), faceIndex,
									&realname, &style, &isFixedPitch, nullptr))
				{
					// Qk_DLog("---- failed to open <%s> <%d> as a font\n", filename.c_str(), faceIndex);
					continue;
				}

				FontStyleSet_Custom* addTo = find_family(*families, realname);
				if (nullptr == addTo) {
					addTo = new FontStyleSet_Custom(realname);
					families->push(addTo);
				}
				addTo->appendTypeface(new QkTypeface_File(style, isFixedPitch, true,
																realname, filename.c_str(),
																faceIndex));
			}
		}
	}

	String fBaseDirectory;
};

#ifndef Qk_FONT_FILE_PREFIX
#  if defined(Qk_APPLE)
#    define Qk_FONT_FILE_PREFIX "/System/Library/Fonts/"
#  else
#    define Qk_FONT_FILE_PREFIX "/usr/share/fonts/"
#  endif
#endif

qk::FontPool* qk::FontPool::Make() {
	return new QkFontPool_Custom(DirectorySystemFontLoader(Qk_FONT_FILE_PREFIX));
}
