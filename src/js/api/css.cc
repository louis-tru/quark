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

#include "./ui.h"
#include "../../ui/css/css.h"
#include "../../ui/app.h"

#define Js_StyleSheets_Accessor(T, Prop, Name) \
	Js_Class_Accessor_Set(Name, { \
		Js_Parse_Type(T, val, "@prop StyleSheets."#Name" = %s"); \
		Js_Self(Type); \
		self->set_##Prop(out); \
	})

namespace qk { namespace js {
	typedef qk::Wrap Wrap;

	struct WrapStyleSheets: WrapObject {
		typedef StyleSheets Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(StyleSheets, 0, { Js_Throw("Access forbidden."); });
			Js_StyleSheets_Accessor(float, opacity, opacity);
			Js_StyleSheets_Accessor(CursorStyle, cursor, cursor);
			Js_StyleSheets_Accessor(bool, visible, visible);
			Js_StyleSheets_Accessor(bool, receive, receive);
			Js_StyleSheets_Accessor(bool, clip, clip);
			Js_StyleSheets_Accessor(Align, align, align);
			Js_StyleSheets_Accessor(BoxSize, width, width);
			Js_StyleSheets_Accessor(BoxSize, height, height);
			Js_StyleSheets_Accessor(BoxSize, min_width, minWidth);
			Js_StyleSheets_Accessor(BoxSize, min_height, minHeight);
			Js_StyleSheets_Accessor(BoxSize, max_width, maxWidth);
			Js_StyleSheets_Accessor(BoxSize, max_height, maxHeight);
			Js_StyleSheets_Accessor(ArrayFloat, margin, margin);
			Js_StyleSheets_Accessor(float, margin_top, marginTop);
			Js_StyleSheets_Accessor(float, margin_right, marginRight);
			Js_StyleSheets_Accessor(float, margin_bottom, marginBottom);
			Js_StyleSheets_Accessor(float, margin_left, marginLeft);
			Js_StyleSheets_Accessor(ArrayFloat, padding, padding);
			Js_StyleSheets_Accessor(float, padding_top, paddingTop);
			Js_StyleSheets_Accessor(float, padding_right, paddingRight);
			Js_StyleSheets_Accessor(float, padding_bottom, paddingBottom);
			Js_StyleSheets_Accessor(float, padding_left, paddingLeft);
			Js_StyleSheets_Accessor(ArrayFloat, border_radius, borderRadius);
			Js_StyleSheets_Accessor(float, border_radius_left_top, borderRadiusLeftTop);
			Js_StyleSheets_Accessor(float, border_radius_right_top, borderRadiusRightTop);
			Js_StyleSheets_Accessor(float, border_radius_right_bottom, borderRadiusRightBottom);
			Js_StyleSheets_Accessor(float, border_radius_left_bottom, borderRadiusLeftBottom);
			Js_StyleSheets_Accessor(ArrayBorder, border, border); // border width
			Js_StyleSheets_Accessor(BoxBorder, border_top, borderTop);
			Js_StyleSheets_Accessor(BoxBorder, border_right, borderRight);
			Js_StyleSheets_Accessor(BoxBorder, border_bottom, borderBottom);
			Js_StyleSheets_Accessor(BoxBorder, border_left, borderLeft);
			Js_StyleSheets_Accessor(ArrayFloat, border_width, borderWidth);
			Js_StyleSheets_Accessor(ArrayColor, border_color, borderColor);
			Js_StyleSheets_Accessor(float, border_width_top, borderWidthTop); // border width
			Js_StyleSheets_Accessor(float, border_width_right, borderWidthRight);
			Js_StyleSheets_Accessor(float, border_width_bottom, borderWidthBottom);
			Js_StyleSheets_Accessor(float, border_width_left, borderWidthLeft);
			Js_StyleSheets_Accessor(Color, border_color_top, borderColorTop); // border color
			Js_StyleSheets_Accessor(Color, border_color_right, borderColorRight);
			Js_StyleSheets_Accessor(Color, border_color_bottom, borderColorBottom);
			Js_StyleSheets_Accessor(Color, border_color_left, borderColorLeft);
			Js_StyleSheets_Accessor(Color, background_color, backgroundColor);
			Js_StyleSheets_Accessor(BoxFilterPtr, background, background);
			Js_StyleSheets_Accessor(BoxShadowPtr, box_shadow, boxShadow);
			Js_StyleSheets_Accessor(float, weight, weight);
			Js_StyleSheets_Accessor(Direction, direction, direction);
			Js_StyleSheets_Accessor(ItemsAlign, items_align, itemsAlign);
			Js_StyleSheets_Accessor(CrossAlign, cross_align, crossAlign);
			Js_StyleSheets_Accessor(Wrap, wrap, wrap);
			Js_StyleSheets_Accessor(WrapAlign, wrap_align, wrapAlign);
			Js_StyleSheets_Accessor(String, src, src);
			Js_StyleSheets_Accessor(TextAlign, text_align, textAlign);
			Js_StyleSheets_Accessor(TextWeight, text_weight, textWeight);
			Js_StyleSheets_Accessor(TextSlant, text_slant, textSlant);
			Js_StyleSheets_Accessor(TextDecoration, text_decoration, textDecoration);
			Js_StyleSheets_Accessor(TextOverflow, text_overflow, textOverflow);
			Js_StyleSheets_Accessor(TextWhiteSpace, text_white_space, textWhiteSpace);
			Js_StyleSheets_Accessor(TextWordBreak, text_word_break, textWordBreak);
			Js_StyleSheets_Accessor(TextSize, text_size, textSize);
			Js_StyleSheets_Accessor(TextColor, text_background_color, textBackgroundColor);
			Js_StyleSheets_Accessor(TextColor, text_color, textColor);
			Js_StyleSheets_Accessor(TextSize, text_line_height, textLineHeight);
			Js_StyleSheets_Accessor(TextShadow, text_shadow, textShadow);
			Js_StyleSheets_Accessor(TextFamily, text_family, textFamily);
			Js_StyleSheets_Accessor(bool, security, security);
			Js_StyleSheets_Accessor(bool, readonly, readonly);
			Js_StyleSheets_Accessor(KeyboardType, type, type);
			Js_StyleSheets_Accessor(KeyboardReturnType, return_type, returnType);
			Js_StyleSheets_Accessor(Color, placeholder_color, placeholderColor);
			Js_StyleSheets_Accessor(Color, cursor_color, cursorColor);
			Js_StyleSheets_Accessor(uint32_t, max_length, maxLength);
			Js_StyleSheets_Accessor(String, placeholder, placeholder);
			Js_StyleSheets_Accessor(Color, scrollbar_color, scrollbarColor);
			Js_StyleSheets_Accessor(float, scrollbar_width, scrollbarWidth);
			Js_StyleSheets_Accessor(float, scrollbar_margin, scrollbarMargin);
			Js_StyleSheets_Accessor(Vec2, translate, translate);
			Js_StyleSheets_Accessor(Vec2, scale, scale);
			Js_StyleSheets_Accessor(Vec2, skew, skew);
			Js_StyleSheets_Accessor(ArrayOrigin, origin, origin);
			Js_StyleSheets_Accessor(float, x, x);
			Js_StyleSheets_Accessor(float, y, y);
			Js_StyleSheets_Accessor(float, scale_x, scaleX);
			Js_StyleSheets_Accessor(float, scale_y, scaleY);
			Js_StyleSheets_Accessor(float, skew_x, skewX);
			Js_StyleSheets_Accessor(float, skew_y, skewY);
			Js_StyleSheets_Accessor(float, rotate_z, rotateZ);
			Js_StyleSheets_Accessor(BoxOrigin, origin_x, originX);
			Js_StyleSheets_Accessor(BoxOrigin, origin_y, originY);

			Js_Class_Accessor_Get(itemsCount, {
				Js_Self(StyleSheets);
				Js_Return( self->itemsCount() );
			});

			// bool hasProperty(ViewProp key) const;

			Js_Class_Method(apply, {
				if (!args.length() || !Js_IsView(args[0]))
					Js_Throw("@method StyleSheets.apply(view) Bad argument.");
				Js_Self(StyleSheets);
				self->apply(wrap<View>(args[0])->self(), false);
			});

			Js_Class_Method(fetch, {
				if (!args.length() || !Js_IsView(args[0]))
					Js_Throw("@method StyleSheets.fetch(view) Bad argument.");
				Js_Self(StyleSheets);
				self->fetch(wrap<View>(args[0])->self(), false);
			});
		}
	};

	struct WrapCStyleSheets: WrapObject {
		typedef CStyleSheets Type;
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(CStyleSheets, StyleSheets, { Js_Throw("Access forbidden."); });
			Js_StyleSheets_Accessor(uint32_t, time, time);
			Js_StyleSheets_Accessor(Curve, curve, curve);
		}
	};

	struct WrapCStyleSheetsClass: WrapObject {
		typedef CStyleSheetsClass Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(CStyleSheetsClass, 0, { Js_Throw("Access forbidden."); });

		// Qk_DEFINE_PGET(bool, havePseudoType, Const); //!< The current style sheet group supports pseudo types
		// Qk_DEFINE_PGET(bool, firstApply, Const); //!< Is this the first time applying a style sheet
		// Qk_DEFINE_PGET(View*, host); //!< apply style sheet target object
		// Qk_DEFINE_PGET(CStyleSheetsClass*, parent); //!< @safe Rt apply parent ssc

			Js_Class_Method(set, {
				if (!args.length())
					Js_Throw("@method CStyleSheetsClass.set(cArray<String> &name)");
				Js_Parse_Type(ArrayString, args[0], "@method CStyleSheetsClass.set(name = %s)");
				Js_Self(Type);
				self->set(out);
			});

			Js_Class_Method(add, {
				if (!args.length())
					Js_Throw("@method CStyleSheetsClass.add(cString& name)");
				Js_Self(Type);
				self->add(args[0]->toStringValue(worker));
			});

			Js_Class_Method(remove, {
				if (!args.length())
					Js_Throw("@method CStyleSheetsClass.remove(cString& name)");
				Js_Self(Type);
				self->remove(args[0]->toStringValue(worker));
			});

			Js_Class_Method(toggle, {
				if (!args.length())
					Js_Throw("@method CStyleSheetsClass.toggle(cString& name)");
				Js_Self(Type);
				self->toggle(args[0]->toStringValue(worker));
			});

		// inline bool haveSubstyles() const;

			cls->exports("CStyleSheetsClass", exports);
		}
	};

	struct NativeCSS {
		static void create(Worker *worker, JSArray *names, JSObject *arg) {
			auto rss = shared_app()->styleSheets();

			for ( uint32_t i = 0, len = names->length(); i < len; i++ ) {
				auto key = names->get(worker, i);
				auto val = arg->get(worker, key);
				if ( !val ) return; // js error
				if ( !val->isObject() ) {
					Js_Throw("NativeCSS.create() Invalid style sheets object");
				}
				auto arr = rss->search( key->toStringValue(worker, true), true );

				if ( arr.length() ) {
					auto props = val->as<JSObject>();
					auto names = props->getPropertyNames(worker);

					for ( uint32_t j = 0, len = names->length(); j < len; j++ ) {
						auto key = names->get(worker, j);
						auto val = props->get(worker, key);
						if ( !val ) return; // js error
						for ( auto ss : arr ) {
							auto that = WrapObject::wrap<CStyleSheets>(ss)->that();
							if ( !that->set(worker, key, val) ) {
								return; // js error
							}
						}
					}
				} // if (arr.length)
			} // for
		}

		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_types");
			WrapStyleSheets::binding(exports, worker);
			WrapCStyleSheets::binding(exports, worker);
			WrapCStyleSheetsClass::binding(exports, worker);

			Js_Method(create, {
				if ( !checkApp(worker) ) return;
				if ( args.length() < 1 || !args[0]->isObject() || args[0]->isNull() ) {
					Js_Throw("NativeCSS.create(Object K/V) Bad argument.");
				}
				Js_Handle_Scope();

				auto arg = args[0]->template as<JSObject>();
				auto names = arg->getPropertyNames(worker);
				if (names->length()) {
					shared_app()->lockAllRenderThreads(Cb([worker,names,arg](auto& e) {
						create(worker, names, arg);
					}));
				}
			});
		}
	};

	Js_Module(_css, NativeCSS);
}	}
