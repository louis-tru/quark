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

// @private

#ifndef __quark__view__spine__
#define __quark__view__spine__

#include "./sprite.h"

namespace spine {
	class Atlas;
	class SkeletonData;
	class TrackEntry;
	class EventData;
	class SkeletonClipping;
}

namespace qk {

	/**
	 * @class SkeletonData
	 * @brief Wrapper class for Spine skeleton definition data.
	 *
	 * This class owns the parsed Spine skeleton and atlas data,
	 * and manages their lifetime. It provides factory methods
	 * for loading from file paths or memory buffers, and hides
	 * the raw Spine runtime pointers.
	 *
	 * Typical usage:
	 * @code
	 * auto data = SkeletonData::Make("hero.skel", "hero.atlas", 1.0f);
	 * spineView->skeleton(data);
	 * @endcode
	 *
	 * @note Internally wraps Spine runtime classes:
	 * - spine::SkeletonData
	 * - spine::Atlas
	 * - spine::AtlasAttachmentLoader
	 *
	 * Lifetime is reference-counted via Reference/Sp<T>.
	 */
	class Qk_EXPORT SkeletonData: public Reference {
	public:
		/**
		 * @brief Creates SkeletonData from skeleton and atlas files.
		 * @param skelPath Path to skeleton binary file.
		 * @param atlasPath Path to atlas file. Optional, may be empty string.
		 * @param scale Scale factor applied to skeleton (default = 1.0f).
		 * @return may return nullptr if loading fails.
		 */
		static Sp<SkeletonData> Make(cString &skelPath,
																	cString &atlasPath = String(),
																	float scale = 1.0f) throw(Error);

		/**
		 * @brief Creates SkeletonData from skeleton buffer + atlas path.
		 * @param skelBuff In-memory buffer containing skeleton binary.
		 * @param atlasPath Path to atlas file.
		 * @param scale Scale factor applied to skeleton (default = 1.0f).
		 * @return may return nullptr if loading fails.
		 */
		static Sp<SkeletonData> Make(cBuffer &skelBuff,
																	cString &atlasPath,
																	float scale = 1.0f) throw(Error);

		/**
		 * @brief Creates SkeletonData from skeleton + atlas buffers.
		 * @param skelBuff In-memory buffer containing skeleton binary.
		 * @param atlasBuff In-memory buffer containing atlas data.
		 * @param dir Directory path used to resolve texture file paths in atlas.
		 * @param scale Scale factor applied to skeleton (default = 1.0f).
		 * @return may return nullptr if loading fails.
		 */
		static Sp<SkeletonData> Make(cBuffer &skelBuff,
																	cBuffer &atlasBuff,
																	cString &dir,
																	float scale = 1.0f) throw(Error);

		/// @brief Destructor. Releases internal Spine objects.
		~SkeletonData();

	private:
		class QkAtlasAttachmentLoader;
		/**
		 * @brief Private constructor used by factory methods.
		 * @param data Parsed Spine skeleton data.
		 * @param atlas Spine atlas (texture regions).
		 * @param loader Spine atlas attachment loader.
		 */
		SkeletonData(spine::SkeletonData* data,
									spine::Atlas* atlas,
									QkAtlasAttachmentLoader* loader);

		static SkeletonData* _Make(cBuffer &skel,
			cBuffer &atlas, cString &dir, float scale, bool json) throw(Error);

		spine::SkeletonData* _data;               ///< Spine skeleton definition.
		spine::Atlas* _atlas;                     ///< Spine texture atlas.
		QkAtlasAttachmentLoader* _atlasLoader; ///< Attachment loader bound to atlas.

		/// @brief Spine class is a friend (direct access for rendering).
		friend class Spine;
	};

	typedef SkeletonData* SkeletonDataPtr;

	/**
	 * @class SpineEvent
	 * @brief Base class for all Spine animation events, wrapping Spine runtime EventType.
	 *
	 * During animation playback, Spine runtime triggers different types of events,
	 * such as when an animation starts, ends, completes, is interrupted, disposed,
	 * or when a custom event is fired. This class provides a unified wrapper for
	 * those events so they can be dispatched through the engine's UIEvent system.
	 *
	 * Type enum maps to spine::EventType:
	 * - kStart_Type       : Triggered when an animation starts (EventType_Start).
	 * - kInterrupt_Type   : Triggered when an animation is interrupted by another (EventType_Interrupt).
	 * - kEnd_Type         : Triggered when the animation reaches its end position (EventType_End).
	 * - kComplete_Type    : Triggered when the animation completes a loop (EventType_Complete).
	 * - kDispose_Type     : Triggered when the TrackEntry is released by Spine runtime (EventType_Dispose).
	 * - kExtEvent_Type       : Triggered when a custom Spine event (Event) is fired (EventType_Event).
	 *
	 * Usage:
	 * - Listen for animation state changes (start/end/complete, etc.) to drive UI or game logic.
	 * - Listen for custom Spine events (keyframe events) to trigger sounds, effects, or gameplay actions.
	 */
	class Qk_EXPORT SpineEvent: public UIEvent {
	public:
		enum Type {
			kStart_Type,      ///< Animation started
			kInterrupt_Type,  ///< Animation interrupted
			kEnd_Type,        ///< Animation reached its end
			kComplete_Type,   ///< Animation completed one loop
			kDispose_Type,    ///< Animation (TrackEntry) disposed
			kExtEvent_Type,   ///< Custom extend Spine event
		};

		/// Constructor
		/// @param origin The originating View (usually the Spine animation view)
		/// @param type   The event type
		/// @param trackIndex The animation track index
		/// @param animationName The name of the animation
		/// @param trackTime The current time (in seconds) of the track
		SpineEvent(Spine* origin, Type type, int trackIndex, cString& animationName, float trackTime);

		Qk_DEFINE_PROP_GET(Type, type, Const); ///< The event type
		Qk_DEFINE_PROP_GET(int, trackIndex, Const); ///< The animation track index
		Qk_DEFINE_PROP_GET(String, animationName, Const); ///< The animation name
		Qk_DEFINE_PROP_GET(float, trackTime, Const); ///< The current time of the track
	};

	/**
	 * @class SpineExtEvent
	 * @brief Wrapper for Spine custom extend events (Event).
	 *
	 * In the Spine editor, designers can add custom Events to the timeline.
	 * These events are triggered at specific times during animation playback.
	 * SpineExtEvent encapsulates those runtime events and dispatches them
	 * through the engine's UIEvent system.
	 *
	 * Exposed data:
	 * - static_data : Static description of the event (EventData), includes name and default values.
	 * - time        : The time (in seconds) in the animation when the event was triggered.
	 * - int_value   : Integer payload value.
	 * - float_value : Floating-point payload value.
	 * - string_value: String payload value.
	 * - volume      : Volume (commonly used for audio events).
	 * - balance     : Audio balance (commonly used for audio events).
	 *
	 * Usage:
	 * - Trigger gameplay logic, sound effects, or particle effects at specific animation keyframes.
	 * - Provide a data bridge between Spine editor events and runtime game logic.
	 */
	class Qk_EXPORT SpineExtEvent: public SpineEvent {
	public:
		typedef const spine::EventData cEventData;

		/// Constructor
		/// @param origin      The originating View (Spine animation view)
		/// @param trackIndex  The animation track index
		/// @param animationName The name of the animation
		/// @param trackTime   The current time (in seconds) of the track
		/// @param staticData  Static event description (EventData)
		/// @param time        Trigger time (in seconds)
		/// @param intValue    Integer value
		/// @param floatValue  Float value
		/// @param stringValue String value
		/// @param volume      Audio volume
		/// @param balance     Audio balance
		SpineExtEvent(Spine* origin
			, int trackIndex, cString& animationName, float trackTime
			, cEventData* staticData, float time, int intValue
			, float floatValue, cString& stringValue, float volume, float balance
		);

		Qk_DEFINE_PROP_GET(cEventData*, static_data, Const); ///< Static event description
		Qk_DEFINE_PROP_GET(float, time, Const);              ///< Trigger time (seconds)
		Qk_DEFINE_PROP_GET(int, int_value, Const);           ///< Integer payload
		Qk_DEFINE_PROP_GET(float, float_value, Const);       ///< Float payload
		Qk_DEFINE_PROP_GET(String, string_value, Const);     ///< String payload
		Qk_DEFINE_PROP_GET(float, volume, Const);            ///< Volume
		Qk_DEFINE_PROP_GET(float, balance, Const);           ///< Audio balance
	};

	/**
	 * @class Spine
	 * @brief View class for rendering and controlling Spine skeleton animations.
	 *
	 * This class integrates Spine runtime (Skeleton, AnimationState, etc.) into the
	 * rendering system, providing control over skeleton pose, slots, skins, animation
	 * playback and transitions, while hiding the low-level Spine API details from the user.
	 *
	 * Inherits from:
	 * - Agent: Enables entity behavior (position, velocity, etc.).
	 * - PreRender::Task: Enables per-frame update tasks (e.g., advancing animation state).
	 */
	class Qk_EXPORT Spine: public Agent, public PreRender::Task {
	public:
		/**
		 * @brief Constructor. Initializes an empty Spine view.
		 */
		Spine();

		/**
		 * @brief Accessor for the engine's opaque SkeletonData wrapper.
		 *
		 * This accessor returns a pointer to the engine-level wrapper type `SkeletonData`.
		 * The actual Spine runtime types (for example `spine::SkeletonData`, `spine::Atlas`,
		 * etc.) are intentionally hidden inside the implementation and are NOT exposed by this API.
		 *
		 * @note
		 * - `SkeletonData` is an opaque wrapper whose implementation details live in the .cpp (PIMPL).
		 * - Callers should treat the returned pointer as the engine wrapper only and must not
		 *   assume access to any `spine::` symbols or internals.
		 * - Lifetime of the returned object is managed by the engine's reference/Sp<T> system.
		 */
		Qk_DEFINE_ACCESSOR(SkeletonData*, skel);

		/**
		 * @brief Sets the active skin.
		 *
		 * The skin is used to look up attachments not found in the SkeletonData defaultSkin.
		 * Attachments from the new skin are attached if the corresponding attachment
		 * from the old skin was attached.
		 *
		 * @param skin Skin name (empty string "" = no skin).
		 */
		Qk_DEFINE_ACCESSOR(String, skin, Const);

		/**
		 * @brief Time scale factor for animations.
		 *
		 * - Normal speed = 1.0
		 * - Slow motion < 1.0
		 * - Fast forward > 1.0
		 * - Valid range: [0.01, 1e2]
		 */
		Qk_DEFINE_PROPERTY(float, speed, Const);

		/**
		 * @brief Default mix duration (seconds) when blending between two animations.
		 *
		 * If no specific mix is defined via set_mix(), this duration is used as the blend time.
		 */
		Qk_DEFINE_PROPERTY(float, default_mix, Const);

		/**
		 * @brief Current animation name for track 0 (base track).
		 *  Set track 0 animation name and loop
		*/
		Qk_DEFINE_ACCESSOR(String, animation, Const);

		/**
		 * @brief Destroys the Spine object, releasing associated resources.
		 */
		void destroy() override;

		/// @brief Returns the type of this view (Spine).
		ViewType viewType() const override;

		/// @brief Returns the logical client rect of the view (depends on skeleton bounds).
		Vec2 client_size() override;

		/// @brief Returns the logical client region of the view.
		Region client_region() override;

		/// @brief Draws the skeleton using the provided Painter.
		void draw(Painter *painter) override;

		/// @brief Called when the view is activated (e.g., the visible is changed).
		void onActivate() override;

		/**
		 * @brief Runs the per-frame update task.
		 * @param time Absolute time in ms.
		 * @param delta Time since last frame in ms.
		 * @return true if task continues, false to stop.
		 */
		bool run_task(int64_t time, int64_t delta) override;

		/**
		 * @brief Resets skeleton to its full setup pose (bones + slots).
		 * Equivalent to calling both set_bones_to_setup_pose() and set_slots_to_setup_pose().
		 */
		void set_to_setup_pose();

		/**
		 * @brief Resets all bones to their setup pose.
		 * Does not affect slots or attachments.
		 */
		void set_bones_to_setup_pose();

		/**
		 * @brief Resets all slots (attachments + draw order) to their setup pose.
		 * Does not affect bones.
		 */
		void set_slots_to_setup_pose();

		/**
		 * @brief Sets an attachment for the specified slot.
		 *
		 * @param slotName Name of the slot.
		 * @param attachmentName Name of the attachment (empty string "" = no attachment).
		 */
		void set_attachment(cString &slotName, cString &attachmentName);

		/**
		 * @brief Sets a custom mix (crossfade) duration between two animations.
		 *
		 * @param fromName Name of the source animation.
		 * @param toName Name of the destination animation.
		 * @param duration Blend duration in seconds.
		 */
		void set_mix(cString &fromName, cString &toName, float duration);

		/**
		 * @brief Sets an animation on a given track, replacing any current animation.
		 *
		 * @param trackIndex Index of the track (0 = base track).
		 * @param name Animation name.
		 * @param loop Whether the animation should loop.
		 * @return A pointer to the created TrackEntry.
		 */
		spine::TrackEntry* set_animation(uint32_t trackIndex, cString &name, bool loop = false);

		/**
		 * @brief Queues an animation after the current one on the track.
		 *
		 * @param trackIndex Index of the track.
		 * @param name Animation name.
		 * @param loop Whether the animation should loop.
		 * @param delay Delay before starting, in seconds (0 = immediately after current).
		 * @return A pointer to the created TrackEntry.
		 */
		spine::TrackEntry* add_animation(uint32_t trackIndex, cString &name, bool loop = false, float delay = 0);

		/**
		 * @brief Sets an empty animation on a track, fading out the current animation.
		 *
		 * @param trackIndex Index of the track.
		 * @param mixDuration Crossfade duration (seconds).
		 * @return A pointer to the TrackEntry representing the empty animation.
		 */
		spine::TrackEntry* set_empty_animation(uint32_t trackIndex, float mixDuration);

		/**
		 * @brief Applies empty animations to all tracks, fading them out.
		 *
		 * @param mixDuration Crossfade duration (seconds).
		 */
		void set_empty_animations(float mixDuration);

		/**
		 * @brief Queues an empty animation after the current animation.
		 *
		 * @param trackIndex Index of the track.
		 * @param mixDuration Crossfade duration (seconds).
		 * @param delay Delay before starting, in seconds.
		 * @return A pointer to the TrackEntry representing the empty animation.
		 */
		spine::TrackEntry* add_empty_animation(uint32_t trackIndex, float mixDuration, float delay = 0);

		/**
		 * @brief Gets the currently playing animation on a track.
		 *
		 * @param trackIndex Index of the track (default 0).
		 * @return The TrackEntry of the active animation, or nullptr if none.
		 */
		spine::TrackEntry* get_current(uint32_t trackIndex = 0);

		/// @brief Clears all tracks (removes all animations).
		void clear_tracks();

		/// @brief Clears a specific track (removes animation at given index).
		void clear_track(uint32_t trackIndex = 0);

		/// @override
		void solve_marks(const Mat &mat, View *parent, uint32_t mark) override;

	private:
		/// @brief Internal wrapper around Spine skeleton and runtime objects (hidden implementation).
		Qk_DEFINE_INLINE_CLASS(SkeletonWrapper);

		mutable QkMutex _mutex; // protects _self and SkeletonSelf members
		Sp<spine::SkeletonClipping> _clipper; // clipping helper
		/// @brief Thread-safe wrapper pointer (atomic for concurrent safety).
		std::atomic<SkeletonWrapper*> _skel;
		Vec2 _skel_origin, _skel_size; // cached skeleton origin and size
		bool _firstDraw;
	};

} // namespace qk
#endif // __quark__view__spine__
