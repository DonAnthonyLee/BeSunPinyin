#ifndef __SUNPINYIN_BE_MODULES_H__
#define __SUNPINYIN_BE_MODULES_H__

#include "be-stuff.h"
#include <sunpinyin.h>

#define COUNT_OF_MESSAGE_HANDLER_MESSENGERS	2

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
	const BMessenger&		GetHandlerMessenger();
	const BMessenger&		CurrentHandlerMessenger() const;
	void				ResetSunPinyin();

private:
	BMenu *fMenu;
	BMessenger fMenuHandlerMsgr;
	int32 fCurrentMessageHandlerMsgr;
	BMessenger *fMessageHandlerMsgrs[COUNT_OF_MESSAGE_HANDLER_MESSENGERS];
	BMessenger fStatusWinMessenger;

	CIMIView *fIMView;
	SunPinyinHandler *fIMHandler;

	BList fMessageOutList;

	BMenu*		_GenerateMenu() const;
	status_t	_InitSunPinyin();
	void		_DeInitSunPinyin();
};

#endif /* __SUNPINYIN_BE_MODULES_H__ */

