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

#ifdef __LITE_BEAPI__
	virtual status_t		InitCheck() const;
#else
	virtual status_t		InitCheck();
#endif

	virtual status_t		MethodActivated(bool state);

	// NOTE: The description below came from the ETK++.
	// Filter():
	//	In order to use the filter in the appplication which implemented with
	// the same graphics engine, please use asynchronous message (sent without reply)
	// to notify other threads instead of calling the GUI stuff directly.
	virtual filter_result		Filter(BMessage *message, BList *outList);

	bool				Lock();
	void				Unlock();

	// communicate with SunPinyinHandler
	void				EmptyMessageOutList();
	void				AddMessageToOutList(BMessage *msg);
	const BMessenger&		GetHandlerMessenger();
	const BMessenger&		CurrentHandlerMessenger() const;
	void				ResetSunPinyin();

private:
	BLocker fLocker;
	BMenu *fMenu;
	BMessenger fMenuHandlerMsgr;
	int32 fCurrentMessageHandlerMsgr;
	BMessenger *fMessageHandlerMsgrs[COUNT_OF_MESSAGE_HANDLER_MESSENGERS];
	BMessenger fStatusWinMessenger;

	CIMIView *fIMView;
	SunPinyinHandler *fIMHandler;

	BList fMessageOutList;

	bool fShiftKeyToSwitch;
	bool fShiftKeyFollowingOthers;
	bool fEnabled;

	BString fErrorInfo;

	BMenu*		_GenerateMenu() const;
	status_t	_InitSunPinyin();
	void		_DeInitSunPinyin();
};

#endif /* __SUNPINYIN_BE_MODULES_H__ */

