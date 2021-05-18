#ifndef __SUNPINYIN_BE_MODULES_H__
#define __SUNPINYIN_BE_MODULES_H__

#include "be-stuffs.h"
#include <sunpinyin.h>

class _LOCAL SunPinyinHandler;

class _LOCAL SunPinyinModule : public BInputServerMethod {
public:
	SunPinyinModule();
	virtual ~SunPinyinModule();

	virtual status_t		InitCheck() const;
	virtual status_t		MethodActivated(bool state);
	virtual filter_result		Filter(BMessage *message, BList *outList);

	// communicate with SunPinyinHandler
	void				EmptyMessageOutList();
	void				AddMessageToOutList(BMessage *msg);
	const BMessenger&		HandlerMessenger() const;
	void				ResetSunPinyin();

private:
	BMenu *fMenu;
	BMessenger fMessenger;
	BMessenger fStatusWinMessenger;

	CIMIView *fIMView;
	SunPinyinHandler *fIMHandler;

	BList fMessageOutList;

	BMenu*		_GenerateMenu() const;
	status_t	_InitSunPinyin();
	void		_DeInitSunPinyin();
};

#endif /* __SUNPINYIN_BE_MODULES_H__ */

