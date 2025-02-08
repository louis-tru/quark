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

#include "quark/ui/app.h"
#include "quark/js/js.h"
#include "quark/util/fs.h"

namespace qk { namespace js {

	Qk_Main() {
		String cmd;

#if Qk_ANDROID
			cmd = Android::start_cmd();
			if ( cmd.isEmpty() )
#endif
		{
			auto index = fs_resources("index");
			if (fs_reader()->exists_sync(index)) {
				for (auto s: String(fs_reader()->read_file_sync(index)).split('\n')) {
					s = s.trim();
					if ( s[0] != '#' ) {
						cmd = s;
						break;
					}
				}
			}
		}

		if ( cmd.isEmpty() ) {
			Array<String> args;
			for (auto& i : cmd.trim().split(' ')) {
				auto arg = i.trim();
				if (!arg.isEmpty()) args.push(arg);
			}
			if (args.length())
				return js::Start("", args);
		}
		if (argc > 0) {
			return js::Start(argc, argv);
		}
		return 0;
	}

} }
