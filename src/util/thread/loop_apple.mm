
#import <Foundation/Foundation.h>

namespace qk {
	void autoreleasepool_call(void(*cb)(void*, void*), void* arg0, void* arg1) {
		@autoreleasepool {
			cb(arg0, arg1);
		}
	}
}