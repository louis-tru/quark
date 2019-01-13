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

#if defined(__linux__)

/*
 * snapd-xdg-open
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <gio/gio.h>

int test2_xopen(int argc, char *argv[])
{
	GDBusConnection *bus = NULL;
	GVariant *result = NULL;
	GError *error = NULL;
	int retval = 0;

	if (argc != 2)
	{
		g_printerr("syntax: %s <url>\n", argv[0]);
		retval = 1;
		goto out;
	}

	bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);

	if (bus == NULL)
	{
		g_printerr ("error: %s\n", error->message);
		retval = 1;
		goto out;
	}

	result = g_dbus_connection_call_sync (bus,
																				"com.canonical.SafeLauncher",
																				"/",
																				"com.canonical.SafeLauncher",
																				"OpenURL",
																				g_variant_new("(s)", argv[1]),
																				G_VARIANT_TYPE_UNIT,
																				G_DBUS_CALL_FLAGS_NONE,
																				-1,
																				NULL,
																				&error);

	if (result == NULL)
	{
		g_printerr ("error: %s\n", error->message);
		retval = 1;
	}

 out:
	g_clear_object (&bus);
	g_clear_object (&result);
	g_clear_object (&error);

	return retval;
}

#endif
