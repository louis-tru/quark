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

#import <UIKit/UIKit.h>
typedef UIEvent MacUIEvent;
#import "../../app.h"
#import "../../event.h"
#import "./ios_app.h"

using namespace qk;

extern QkApplicationDelegate *qkappdelegate;

@implementation QkRootViewController

	-(BOOL)shouldAutorotate {
		return YES;
	}

	-(UIInterfaceOrientationMask)supportedInterfaceOrientations {
		switch ( qkappdelegate.setting_orientation ) {
			case Orientation::ORIENTATION_PORTRAIT:
				return UIInterfaceOrientationMaskPortrait;
			case Orientation::ORIENTATION_LANDSCAPE:
				return UIInterfaceOrientationMaskLandscapeRight;
			case Orientation::ORIENTATION_REVERSE_PORTRAIT:
				return UIInterfaceOrientationMaskPortraitUpsideDown;
			case Orientation::ORIENTATION_REVERSE_LANDSCAPE:
				return UIInterfaceOrientationMaskLandscapeLeft;
			case Orientation::ORIENTATION_USER: default:
				return UIInterfaceOrientationMaskAll;
			case Orientation::ORIENTATION_USER_PORTRAIT:
				return UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
			case Orientation::ORIENTATION_USER_LANDSCAPE:
				return UIInterfaceOrientationMaskLandscape;
			case Orientation::ORIENTATION_USER_LOCKED: {
				switch (qkappdelegate.current_orientation) {
					default:
					case Orientation::ORIENTATION_INVALID:
						return UIInterfaceOrientationMaskAll;
					case Orientation::ORIENTATION_PORTRAIT:
						return UIInterfaceOrientationMaskPortrait;
					case Orientation::ORIENTATION_LANDSCAPE:
						return UIInterfaceOrientationMaskLandscapeRight;
					case Orientation::ORIENTATION_REVERSE_PORTRAIT:
						return UIInterfaceOrientationMaskPortraitUpsideDown;
					case Orientation::ORIENTATION_REVERSE_LANDSCAPE:
						return UIInterfaceOrientationMaskLandscapeLeft;
				}
			}
		}
		return UIInterfaceOrientationMaskAll;
	}

	-(void)viewWillTransitionToSize:(CGSize)size
				withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
	{
		[coordinator animateAlongsideTransition:^(id context) {
			qkappdelegate.render->render()->reload();

			Orientation orient = qkappdelegate.host->display()->orientation();
			if (orient != qkappdelegate.current_orientation) {
				qkappdelegate.current_orientation = orient;
				qkappdelegate.host->loop()->post(Cb([](Cb::Data& e) {
					qkappdelegate.host->display()->Qk_Trigger(Orientation);
				}));
			}
		} completion:nil];
		[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	}

	-(UIStatusBarStyle)preferredStatusBarStyle {
		return qkappdelegate.status_bar_style;
	}

	-(UIStatusBarAnimation)preferredStatusBarUpdateAnimation {
		// UIApplicationWillChangeStatusBarFrameNotification
		return UIStatusBarAnimationSlide;
	}

	-(BOOL)prefersStatusBarHidden {
		return !qkappdelegate.visible_status_bar;
	}

	-(List<TouchPoint>)touchsList:(NSSet<UITouch*>*)touches {
		List<TouchPoint> rv;

		Vec2 size = qkappdelegate.host->display()->size();

		float scale_x = size.x() /  qkappdelegate.surface_view.frame.size.width;
		float scale_y = size.y() /  qkappdelegate.surface_view.frame.size.height;

		for (UITouch* touch in [touches objectEnumerator]) {
			CGPoint point = [touch locationInView:touch.view];
			CGFloat force = touch.force;
			// CGFloat angle = touch.altitudeAngle;
			// CGFloat max_force = touch.maximumPossibleForce;
			rv.pushBack({
				uint32_t((size_t)touch % Uint32::limit_max), 0, 0,
				float(point.x * scale_x), float(point.y * scale_y),
				float(force), false, nullptr,
			});
		}

		return rv;
	}

	-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
		qkappdelegate.host->dispatch()->onTouchstart( [self touchsList:touches] );
	}

	-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
		// Qk_DLog("touchesMoved, count: %d", touches.count);
		qkappdelegate.host->dispatch()->onTouchmove( [self touchsList:touches] );
	}

	-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event{
		qkappdelegate.host->dispatch()->onTouchend( [self touchsList:touches] );
	}

	-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable MacUIEvent *)event {
		qkappdelegate.host->dispatch()->onTouchcancel( [self touchsList:touches] );
	}

@end
