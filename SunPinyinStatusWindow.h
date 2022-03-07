#ifndef __SUNPINYIN_BE_STATUS_WINDOW_H__
#define __SUNPINYIN_BE_STATUS_WINDOW_H__

#include "SunPinyinModule.h"

#ifndef INPUT_SERVER_MORE_SUPPORT
class _LOCAL SunPinyinStatusWindow : public BWindow
{
public:
	SunPinyinStatusWindow();
	virtual ~SunPinyinStatusWindow();

	virtual void DispatchMessage(BMessage *msg, BHandler *target);

private:
	BView *fCandidates;
	int32 fCaret;
	BMessage fCandidatesMsg;
};
#endif

#endif /* __SUNPINYIN_BE_STATUS_WINDOW_H__ */

