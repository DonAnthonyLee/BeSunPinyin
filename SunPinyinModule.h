#ifndef __SUNPINYIN_BE_MODULES_H__
#define __SUNPINYIN_BE_MODULES_H__

#include "be-stuffs.h"
#include <sunpinyin.h>

class SunPinyinModule : public BInputServerMethod {
public:
	SunPinyinModule();
	virtual ~SunPinyinModule();

	virtual status_t		InitCheck() const;
	virtual status_t		MethodActivated(bool state);
	virtual filter_result		Filter(BMessage *message, BList *outList);

private:
	BMenu *fMenu;
	BMessenger fMessenger;
	bool fActivated;

	CIMIView *fIMView;
	CIMIWinHandler *fIMHandler;

	bool _InitSunPinyin();
	void _DeInitSunPinyin();
};

#endif /* __SUNPINYIN_BE_MODULES_H__ */

