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

#ifndef __ngui__view__
#define __ngui__view__

#include "base/array.h"
#include "base/string.h"
#include "base/string-builder.h"
#include "event.h"

/**
 * @ns gui
 */

XX_NS(ngui)

class DrawData;
class Draw;
class GLDraw;
class GLES2Draw;
class View;
class ViewController;
class PreRender;
class ViewXML;
class Texture;
class CSSViewClasss;
class StyleSheetsScope;
class BasicScroll;

#define XX_EACH_VIEWS(F)  \
  F(LAYOUT, Layout, layout) \
  F(BOX, Box, box)  \
  F(DIV, Div, div)  \
  F(SHADOW, Shadow, shadow)  \
  F(GRADIENT, Gradient, gradient)  \
  F(INDEP, Indep, indep)  \
  F(LIMIT, Limit, limit) \
  F(LIMIT_INDEP, LimitIndep, limit_indep) \
  F(IMAGE, Image, image)  \
  F(SCROLL, Scroll, scroll) \
  F(VIDEO, Video, video)  \
  F(ROOT, Root, root) \
  F(SPRITE, Sprite, sprite) \
  F(HYBRID, Hybrid, hybrid) \
  F(TEXT_NODE, TextNode, text_node) \
  F(SPAN, Span, span) \
  F(TEXT_FONT, TextFont, text_font) \
  F(TEXT_LAYOUT, TextLayout, text_layout) \
  F(LABEL, Label, label) \
  F(BUTTON, Button, button) \
  F(SELECT_PANEL, SelectPanel, select_panel) \
  F(TEXT, Text, text) \
  F(CLIP, Clip, clip) \
  F(INPUT, Input, input) \
  F(TEXTAREA, Textarea, textarea) \

#define XX_DEFINE_CLASS(enum, type, name) class type;
XX_EACH_VIEWS(XX_DEFINE_CLASS);
#undef XX_DEFINE_CLASS

#define XX_DEFINE_GUI_VIEW(enum, type, name) \
  public: \
  friend class GLDraw;  \
  friend class GLES2Draw; \
  virtual type* as_##name() { return this; }  \
  virtual ViewType view_type() const { return enum; }

/**
 * @interface Member
 */
class XX_EXPORT Member {
 public:
  
  /**
   * @func as_view
   */
  virtual View* as_view() { return nullptr; };
  
  /**
   * @func as_ctr
   */
  virtual ViewController* as_ctr() { return nullptr; };
  
  /**
   * @func id
   */
  virtual String id() const = 0;
  
};

/**
 * @class ViewController
 */
class XX_EXPORT ViewController: public Reference, public Member {
 public:
  
  ViewController();
  
  /**
   * @destructor
   */
  virtual ~ViewController();
  
  /**
   * @func load_view
   */
  virtual void load_view(ViewXML* vx = nullptr);
  
  /**
   * @func trigger_remove_view view被删除时
   */
  virtual void trigger_remove_view(View* view) { }
  
  /**
   * @func as_ctr
   */
  virtual ViewController* as_ctr() { return this; };
  
  /**
   * @func parent
   */
  ViewController* parent();
  
  /**
   * @func view
   */
  inline View* view() { return m_view; }
  
  /**
   * @func view  set
   */
  void view(View* view) throw(Error);
  
  /**
   * @overwrite
   */
  virtual String id() const;
  
  /**
   * @func id set id
   */
  void set_id(cString& value) throw(Error);
  
  /**
   * @func find # 可通过ID查询控制器下的成员
   */
  virtual Member* find(cString& id);
  
  /**
   * @func set_member # 可通过ID设置控制器下的成员
   */
  virtual void set_member(cString& id, Member* member) throw(Error);
  
  /**
   * @func del_member # 可通过ID删除控制器下的成员
   * 注意: 只会删除弱引用并不从内存移除
   */
  virtual void del_member(cString& id, Member* member = nullptr);
  
  /**
   * @func remove
   */
  void remove();
  
 private:
  View*  m_view;
  String m_id;
  Map<String, Member*> m_members;
  
  XX_DEFINE_INLINE_CLASS(Inl);
};

/**
 * @class View
 */
class XX_EXPORT View: public Notification<GUIEvent, GUIEventName, Reference>, public Member {
  XX_HIDDEN_ALL_COPY(View);
 public:
  
  View();
  
  /**
   * @destructor
   */
  virtual ~View();
  
  /**
   * @enum ViewType
   */
  enum ViewType {
#define xx_def_view_type(enum, type, name) enum,
    INVALID = 0,
    VIEW,
    XX_EACH_VIEWS(xx_def_view_type)
#undef xx_def_view_type
  };

  /**
   * @func view_type 获取视图类型
   */
  virtual ViewType view_type() const { return VIEW; }
  
#define xx_def_view_type(enum, type, name)  \
  inline bool is_##name() const { return const_cast<View*>(this)->as_##name(); }  \
  inline const type* as_##name() const { return const_cast<View*>(this)->as_##name(); } \
  virtual type* as_##name() { return nullptr; }

  // as type / is type
  XX_EACH_VIEWS(xx_def_view_type)
  
#undef xx_def_view_type
  
  /**
   * @overwrite
   */
  virtual View* as_view() { return this; };
  
  /**
   * @func is_clip
   */
  virtual bool is_clip() { return false; }
  
  /**
   * @enum MarkValue
   */
  enum : uint {
    M_NONE                  = 0,          /* 没有任何标记 */
    M_BASIC_MATRIX          = (1 << 0),   /* 基础矩阵变化, */
    M_TRANSFORM             = (1 << 1),   /* 矩阵变换（需要更新变换矩阵）这个标记具有继承性质 */
    M_MATRIX                = (M_BASIC_MATRIX | M_TRANSFORM),
    M_SHAPE                 = (1 << 2),   /* 形状变化,外形尺寸、原点位置都属于形状变化 */
    M_OPACITY               = (1 << 3),   /* 透明度 */
    M_VISIBLE               = (1 << 4),   /* 显示与隐藏 */
    M_INHERIT               = (M_TRANSFORM | M_OPACITY), /* 具有继承性质的变换标记 */
    M_LAYOUT                = (1 << 5),   // 第一次从外到内,初始盒子尺寸,如果有明确尺寸将不会触发三次布局
    M_SIZE_HORIZONTAL       = (1 << 6),   // 设置水平尺寸
    M_SIZE_VERTICAL         = (1 << 7),   // 设置垂直尺寸
    M_CONTENT_OFFSET        = (1 << 8),   // 第二次从内到外,设置内容偏移并挤压无明确尺寸的父盒子
    M_LAYOUT_THREE_TIMES    = (1 << 9),   // 第三次从外到内,设置无明确尺寸盒子,非所有无明确尺寸盒子都有三次
    M_BORDER                = (1 << 10),  // 边框
    M_BORDER_RADIUS         = (1 << 11),  // 边框圆角变化
    M_BACKGROUND_COLOR      = (1 << 12),  // 颜色变化
    M_TEXTURE               = (1 << 13),  // 纹理相关
    M_BOX_SHADOW            = (1 << 14),  // 阴影变化
    M_SCROLL                = (1 << 15),  //
    M_SCROLL_BAR            = (1 << 16),  //
    M_TEXT_SIZE             = (1 << 17),  // 文本尺寸变化
    M_TEXT_FONT             = (1 << 18),  //
    M_INPUT_STATUS          = (1 << 19),  // 文本输入状态改变
    M_STYLE_CLASS           = (1 << 29),  /* 变化class引起的样式变化 */
    M_STYLE_FULL            = (1 << 30),  /* 所有后后代视图都受到影响 */
    M_STYLE                 = (M_STYLE_CLASS | M_STYLE_FULL),
    M_TRANSFORM_AND_OPACITY_CTX_DATA = (uint(1) << 31),   /* 更新数据 */
  };
  
  /**
   & @get inner_text {Ucs2String}
   */
  Ucs2String inner_text() const;
  
  /**
   * @set inner_text {Ucs2String}
   */
  void inner_text(cUcs2String& str) throw(Error);
  
  /**
   * @func prepend # 前置元素
   * @arg child {View*} # 要前置的元素
   */
  virtual void prepend(View* child) throw(Error);
  
  /**
   * @func append # 追加视图至结尾
   * @arg child {View*} # 要追加的元素
   */
  virtual void append(View* child) throw(Error);
  
  /**
   * @func append_text # 追加文本到结尾
   */
  virtual View* append_text(cUcs2String& str) throw(Error);
  
  /**
   * @func append_to # 追加自身至父视图结尾
   * @arg parent {View*} # 父视图
   */
  virtual void append_to(View* parent) throw(Error);
  
  /**
   * @func before # 插入前
   * @arg view {View*} # 要插入的元素
   */
  void before(View* view) throw(Error);
  
  /**
   * @func after # 插入后
   * @arg view {View*} # 要插入的元素
   */
  void after(View* view) throw(Error);
  
  /**
   * @func move_to_before # 移动视图到上一个视图上面
   */
  void move_to_before();
  
  /**
   * @func down # 移动视图到下一个视图下面
   */
  void move_to_after();
  
  /**
   * @func move_to_first # 移动视图到所有视图的上面
   */
  void move_to_first();
  
  /**
   * @func move_to_last # 移动视图到所有视图的下面
   */
  void move_to_last();
  
  /**
   * #func remove # 删除当前视图,从内存清除
   */
  virtual void remove();
  
  /**
   * #func remove_all_child # 删除所有子视图
   */
  virtual void remove_all_child();
  
  /**
   * @func children
   */
  View* children(uint index);
  
  /**
   * @func children_count
   */
  uint children_count();
  
  /**
   * @func id 获取当前视图id
   */
  virtual String id() const;
  
  /**
   * @func set_id 设置视图ID,会同时设置top控制器查询句柄.
   */
  void set_id(cString& value) throw(Error);
  
  /**
   * @func controller 视图控制器
   */
  inline ViewController* controller() { return m_ctr; }
  
  /**
   * @func top 该视图的上层控制器视图
   */
  inline View* top() { return m_top; }
  
  /**
   * @func top_ctr 该视图的上层控制器
   */
  inline ViewController* top_ctr() { 
    return m_top ? m_top->m_ctr : nullptr; 
  }
  
  /**
   * @func parent # 父级视图
   */
  inline View* parent() { return m_parent; }
  
  /**
   * @func prev # 上一个兄弟视图
   */
  inline View* prev() { return m_prev; }
  
  /**
   * @func next # 下一个兄视图
   */
  inline View* next() { return m_next; }
  
  /**
   * @func first # 第一个子视图
   */
  inline View* first() { return m_first; }
  
  /**
   * @func last # 最后一个子视图
   */
  inline View* last() { return m_last; }
  
  /**
   * @func action
   */
  inline Action* action() { return m_action; }
  
  /**
   * @func action
   */
  void action(Action* action) throw(Error);
  
  /**
   * @func x # x轴位移
   */
  inline float x() const { return m_translate.x(); }
  
  /**
   * @func y # y轴位移
   */
  inline float y() const { return m_translate.y(); }
  
  /**
   * @func scale_x # x轴缩放
   */
  inline float scale_x() const { return m_scale.x(); }

  /**
   * @func scale_y # y轴缩放
   */
  inline float scale_y() const { return m_scale.y(); }

  /**
   * @func rotate_z # 旋转角度
   */
  inline float rotate_z() const { return m_rotate_z; }

  /**
   * @func skew_x # x轴斜歪角度
   */
  inline float skew_x() const { return m_skew.x(); }

  /**
   * @func skew_y # y轴斜歪角度
   */
  inline float skew_y() const { return m_skew.y(); }

  /**
   * @func opacity  # 可影响子视图的透明度值
   *                  Can affect the transparency value of sub view
   */
  inline float opacity() const { return m_opacity; }

  /**
   * @func visible  # 是否显示视图,包括子视图
   *                  Whether to display the view, including the sub view
   */
  inline bool visible() const { return m_visible; }
  
  /**
   * @get final_visible {bool} # 最终是否显示会受父视图的影响
   */
  inline bool final_visible() const { return m_final_visible; }
  
  /**
   * @get translate
   */
  inline Vec2 translate() const { return m_translate; }

  /**
   * @func scale
   */
  inline Vec2 scale() const { return m_scale; }
  
  /**
   * @func skew
   */
  inline Vec2 skew() const { return m_skew; }

  /**
   * @func origin_x # x轴原点,以该点 位移,缩放,旋转,歪斜
   *                  To the view of displacement, rotate, scale, skew
   */
  inline float origin_x() const { return m_origin.x(); }

  /**
   * @func origin_y # y轴原点,以该点 位移,缩放,旋转,歪斜
   *                  To the view of displacement, rotate, scale, skew
   */
  inline float origin_y() const { return m_origin.y(); }

  /**
   * @func origin # 原点,以该点 位移,缩放,旋转,歪斜
   *                To the view of displacement, rotate, scale, skew
   */
  inline Vec2 origin() const { return m_origin; }
  
  /**
   * @func set_x
   */
  void set_x(float value);
  
  /**
   * @func set_y
   */
  void set_y(float value);
  
  /**
   * @func set_scale_x
   */
  void set_scale_x(float value);
  
  /**
   * @func set_scale_y
   */
  void set_scale_y(float value);
  
  /**
   * @func set_rotate_z
   */
  void set_rotate_z(float value);
  
  /**
   * @func set_skew_x
   */
  void set_skew_x(float value);
  
  /**
   * @func set_skew_y
   */
  void set_skew_y(float value);
  
  /**
   * @func set_opacity
   */
  void set_opacity(float value);
  
  /**
   * @func set_visible_0
   */
  inline void set_visible_0(bool value) { set_visible(value); }
  
  /**
   * @func set_visible
   */
  virtual void set_visible(bool value);
  
  /**
   * @func set_translate
   */
  void set_translate(Vec2 value);
  
  /**
   * @func set_scale
   */
  void set_scale(Vec2 value);
  
  /**
   * @func set_skew
   */
  void set_skew(Vec2 skew);
  
  /**
   * @func set_origin_x
   */
  void set_origin_x(float value);
  
  /**
   * @func set_origin_y
   */
  void set_origin_y(float value);
  
  /**
   * @func set_origin
   */
  void set_origin(Vec2 value);
    
  /**
   * @func need_draw
   */
  inline bool need_draw() const { return m_need_draw; }
  
  /**
   * @func set_need_draw
   */
  void set_need_draw(bool value);

  /**
   * @func level # 视图在整个视图树中所处的层级
   *               0表示还没有加入到GUIApplication唯一的视图树中,根视图为1
   */
  inline uint level() const { return m_level; }
  
  /**
   * @func transform # 设置视图变换
   * @arg translate {Vec2}
   * @arg scale {Vec2}
   * @arg [rotate_z = 0] {float}
   * @arg [skew = Vec2()] {Vec2}
   */
  void transform(Vec2 translate, Vec2 scale, float rotate_z = 0, Vec2 skew = Vec2());

  /**
   * @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
   */
  virtual bool overlap_test(Vec2 point);
  
  /**
   * @func overlap_test_from_convex_quadrilateral
   */
  static bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
  
  /**
   * @func screen_rect_from_convex_quadrilateral
   */
  static CGRect screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);
  
  /**
   * @func screen_region_from_convex_quadrilateral
   */
  static Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);
  
  /**
   * @func layout_offset #  获取布局偏移值
   */
  virtual Vec2 layout_offset();
  
  /**
   * @func layout_in_offset 获取布局内部偏移值
   */
  virtual Vec2 layout_in_offset();
  
  /**
   * @func layout_offset_from
   */
  Vec2 layout_offset_from(View* parents);
  
  /**
   * @func screen_rect
   */
  virtual CGRect screen_rect();
  
  /**
   * @func matrix 基础矩阵,通过计算从父视图矩阵开始的位移,缩放,旋转,歪斜得到的矩阵。
   */
  const Mat& matrix();
  
  /**
   * @func final_matrix
   */
  const Mat& final_matrix();
  
  /**
   * @func final_opacity
   */
  float final_opacity();
  
  /**
   * @func position
   */
  Vec2 position();
  
  /**
   * @func classs
   */
  inline CSSViewClasss* classs() { return m_classs; }
  
  /**
   * "cls1 clas2 clas3"
   * @func classs
   */
  void classs(cString& names);
  
  /**
   * @func classs
   */
  void classs(const Array<String>& names);
  
  /**
   * @func add_class
   */
  void add_class(cString& names);
  
  /**
   * @func remove_class
   */
  void remove_class(cString& names);
  
  /**
   * @func toggle_class
   */
  void toggle_class(cString& names);
  
  /**
   * @func receive()
   */
  inline bool receive() const { return m_receive; }
  
  /**
   * @func receive()
   */
  inline void set_receive(bool value) { m_receive = value; }
  
  /**
   * @func focus();
   */
  bool focus();
  
  /**
   * @func blur();
   */
  bool blur();
  
  /**
   * @func is_focus();
   */
  bool is_focus() const;
  
  /**
   * @func set_is_focus()
   */
  void set_is_focus(bool value);
  
  /**
   * @func can_become_focus() 是否可以成为焦点
   */
  virtual bool can_become_focus();
  
  /**
   * @func trigger_listener_change()
   */
  virtual void trigger_listener_change(const Name& name, int count, int change);
  
  /**
   * @func first_button
   */
  Button* first_button();
  
  /**
   * @func has_child(child)
   */
  bool has_child(View* child);
  
 protected:
  
  /**
   * @func trigger()
   */
  ReturnValue& trigger(const Name& name, GUIEvent& evt, bool need_send = false);
  ReturnValue trigger(const Name& name, bool need_send = false);
  
  /**
   * @func refresh_styles
   */
  void refresh_styles(StyleSheetsScope* sss);
  
  /**
   * @func mark 标记该视图已经发生改变
   */
  void mark(uint value);
  
  /**
   * @func mark_pre 标记该视图已经发生改变并加入绘制前预处理
   */
  void mark_pre(uint value);
  
  /**
   * @func visit child draw
   */
  inline void visit(Draw* draw) {
    visit(draw, mark_value & M_INHERIT, m_need_draw);
  }
  
  /**
   * @func visit child draw
   */
  void visit(Draw* draw, uint inherit_mark, bool need_draw = false);
  
  /**
   * @func solve transform
   */
  void solve();
  
  /**
   * @func set_visible_draw
   */
  virtual void set_visible_draw();
  
  /**
   * @func accept_text
   */
  virtual void accept_text(Ucs2StringBuilder& out) const;
  
  /**
   * @func set_parent 设置父视图
   */
  virtual void set_parent(View* parent) throw(Error);
  
  /**
   * @func draw 绘制视图,通过这个函数向GUP发送绘图命令
   */
  virtual void draw(Draw* draw);
  
 private:

  View*   m_top;        /* Top视图/Top视图一定有 ViewController */
  View*   m_parent;     /* 父视图 */
  View*   m_prev;       /* 上一个兄弟视图,通过这些属性组成了一个双向链表 */
  View*   m_next;       /* 下一个兄视图 */
  View*   m_first;      /* 第一个子视图 */
  View*   m_last;       /* 最后一个子视图 */
  String  m_id;         /* 视图ID/可以通过ID快速索引视图 */
  Vec2    m_translate;  /* 平移向量 */
  Vec2    m_scale;      /* 缩放向量 */
  Vec2    m_skew;       /* 倾斜向量 */
  float   m_rotate_z;   /* z轴旋转角度值 */
  float   m_opacity;    /* 可影响子视图的透明度值
                         * Can affect the transparency value of sub view */
  /*
   *  不可能同时所有视图都会发生改变,如果视图树很庞大的时候,
   *  如果涉及到布局时为了跟踪其中一个视图的变化就需要遍历整颗视图树,为了避免这种情况
   *  把标记的视图独立到视图外部按视图等级进行分类以双向环形链表形式存储(PreRender)
   *  这样可以避免访问那些没有发生改变的视图并可以根据视图等级顺序访问.
   */
  View* m_prev_pre_mark;      /* 上一个标记的预处理标记视图 */
  View* m_next_pre_mark;      /* 下一个标记的预处理标记视图 */
  
  CSSViewClasss* m_classs;
  
 protected:
  
  uint m_level;         /* 视图在整个视图树中所处的层级
                         * 0表示还没有加入到GUIApplication唯一的视图树中,根视图为1 */
  Mat  m_matrix;        /* 基础矩阵,从父视图矩阵开始的矩阵,通过translate/scale/skew/rotate得到 */
  Vec2 m_origin;        /* 以该点 位移,缩放,旋转,歪斜
                         * To the view of displacement, rotate, scale, skew */
  Mat  m_final_matrix;  /* 父视图矩阵乘以基础矩阵等于最终变换矩阵
                         * The parent view matrix * local matrix = final transformation matrix */
  float m_final_opacity;/* 最终的不透明值 */
  uint mark_value;      /* 这些标记后的视图会在开始帧绘制前进行更新.
                         * 需要这些标记的原因主要是为了最大程度的节省性能开销,
                         * 因为程序在运行过程中可能会频繁的更新视图局部属性也可能视图很少发生改变.
                         *
                         *  1.如果对每次更新如果都更新GPU中的数据那么对性能消耗那将是场灾难,
                         *  那么记录视图所有的局部变化,待到到需要真正帧渲染时统一进行更新.
                         * */
  bool m_visible;       /* 是否显示视图,包括子视图
                         * Whether to display the view, including the sub view */
  bool m_final_visible; /* 最终是否显示,受父视图m_visible影响 */
  bool m_visible_draw;  /* 该状态标识视图是否在绘图范围内,这个状态会忽略m_visible值
                         * Whether on the screen range inner */
  bool m_need_draw;             /* 忽略视图visible draw,强制绘制子视图 */
  bool m_child_change_flag;     /* 子视图有变化标记,调用draw后重置 */
  bool m_removing;              /* 删除中 */
  bool m_receive;               /* 是否接收事件 */
  ViewController* m_ctr;        /* ViewController */
  DrawData*       m_ctx_data;   /* 绘图上下文需要的数据 */
  Action*         m_action;
  Array<View*>*   m_children;   /* 子视图索引 */
  
  XX_DEFINE_INLINE_CLASS(Inl);
  XX_DEFINE_INLINE_CLASS(EventInl);
  XX_DEFINE_INLINE_CLASS(ActionInl);
  
  friend class TextLayout;
  friend class TextFont;
  friend class Layout;
  friend class PreRender;
  friend class ViewController;
  friend class GLDraw;
  friend class GLES2Draw;
  friend class GUIEventDispatch;
  friend class DisplayPort;
  friend class TextNode;
  friend class Label;
  friend class Span;
  friend class CSSViewClasss;
  friend class GUIResponder;
  friend class BasicScroll;
};

XX_END
#endif
