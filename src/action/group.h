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

#ifndef __noug__action__group__
#define __noug__action__group__

#include "./action.h"

N_NAMESPACE_START

/**
* @class GroupAction
*/
class N_EXPORT GroupAction: public Action {
public:
	/**
	* @func operator[]
	*/
	Action* operator[](uint32_t index);
	
	/**
	* @func length
	*/
	inline uint32_t length() const { return _actions.length(); }
	
	/**
	* @func append
	*/
	virtual void append(Action* action) throw(Error);
	
	/**
	* @func insert
	*/
	virtual void insert(uint32_t index, Action* action) throw(Error);
	
	/**
	* @func remove_child
	*/
	virtual void remove_child(uint32_t index);
	
	/**
	* @overwrite
	*/
	virtual void clear();

	/**
	* @
	*/
	virtual GroupAction* as_group() {
		return this;
	}
	
protected:
	/**
	* @destructor
	*/
	virtual ~GroupAction();
	
	/**
	* @overwrite
	*/
	virtual void bind_view(View* view);
	
	typedef List<Action*>::Iterator Iterator;
	
	List<Action*>  _actions;
	Array<Iterator>     _actions_index;
	
	friend class Action;
	
	N_DEFINE_INLINE_CLASS(Inl);
};

/**
* @class SpawnAction
*/
class N_EXPORT SpawnAction: public GroupAction {
public:
	/**
	* @func spawn
	*/
	inline Action* spawn(uint32_t index) { return (*this)[index]; }
	
	/**
	* @overwrite
	*/
	virtual SpawnAction* as_spawn() { return this; }
	virtual void append(Action* action) throw(Error);
	virtual void insert(uint32_t index, Action* action) throw(Error);
	virtual void remove_child(uint32_t index);

private:
	/**
	* @overwrite
	*/
	virtual uint64_t advance(uint64_t time_span, bool restart, Action* root);
	virtual void seek_time(uint64_t time, Action* root);
	virtual void seek_before(int64_t time, Action* child);
};

/**
* @class SequenceAction
*/
class N_EXPORT SequenceAction: public GroupAction {
public:
	/**
	* @func seq
	*/
	inline Action* seq(uint32_t index) { return (*this)[index]; }
	
	/**
	* @overwrite
	*/
	virtual SequenceAction* as_sequence() { return this; }
	virtual void append(Action* action) throw(Error);
	virtual void insert(uint32_t index, Action* action) throw(Error);
	virtual void remove_child(uint32_t index);
	virtual void clear();
	
private:
	/**
	* @overwrite
	*/
	virtual uint64_t advance(uint64_t time_span, bool restart, Action* root);
	virtual void seek_time(uint64_t time, Action* root);
	virtual void seek_before(int64_t time, Action* child);
	
	Iterator _action;
	
	friend class GroupAction::Inl;

};

N_NAMESPACE_END
#endif