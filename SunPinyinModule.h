#ifndef __SUNPINYIN_BE_MODULES_H__
#define __SUNPINYIN_BE_MODULES_H__

#include "be-stuff.h"
#include <sunpinyin.h>

#ifdef __LITE_BEAPI__
#define INPUT_SERVER_MORE_SUPPORT
#define STATUS_MAX_ROWS				6
#define STATUS_MAX_COLUMNS			10
#endif

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
	//	In order to use the filter in the application which implemented with
	// the same graphics engine, please use asynchronous message (sent without reply)
	// to notify other threads instead of calling the GUI stuff directly.
	virtual filter_result		Filter(BMessage *message, BList *outList);

	bool				Lock();
	void				Unlock();
	bool				IsLocked();

	CIMIView*			IMView();
	SunPinyinHandler*		IMHandler();

	// communicate with SunPinyinHandler
	void				EmptyMessageOutList();
	void				EnqueueMessageOutList();
	void				AddMessageToOutList(BMessage *msg);
	const BMessenger&		GetHandlerMessenger();
	const BMessenger&		CurrentHandlerMessenger() const;
	void				ResetSunPinyin();

	// for fMenuHandlerMsgr
	void				SwitchShiftKeyUsing();
	void				SwitchPageKeysGroup(uint8 bit);

private:
	BLocker fLocker;
	BMenu *fMenu;
	BMessenger fMenuHandlerMsgr;
	int32 fCurrentMessageHandlerMsgr;
	BMessenger *fMessageHandlerMsgrs[COUNT_OF_MESSAGE_HANDLER_MESSENGERS];

#ifndef INPUT_SERVER_MORE_SUPPORT
	BMessenger fStatusWinMessenger;
#endif

	CIMIView *fIMView;
	SunPinyinHandler *fIMHandler;

	BList fMessageOutList;

	bool fShiftKeyToSwitch;
	uint8 fPageKeysGroupFlags;

	bool fShiftKeyFollowingOthers;
	bool fEnabled;
	bool fActivated;

	BString fErrorInfo;
	BMessenger fAlertMessenger;

	BMenu*		_GenerateMenu();
	void		_RegenMenu();
	status_t	_InitSunPinyin();
	void		_DeInitSunPinyin();
};

#endif /* __SUNPINYIN_BE_MODULES_H__ */

