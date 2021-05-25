#ifndef __SUNPINYIN_BE_HANDLER_H__
#define __SUNPINYIN_BE_HANDLER_H__

#include "SunPinyinModule.h"

#define MSG_MENU_SWITCH_EN_CN_BY_SHIFT_KEY	'msg1'
#define MSG_MENU_USE_PAGE_KEYS_GROUP1		'msg2'
#define MSG_MENU_USE_PAGE_KEYS_GROUP2		'msg3'
#define MSG_MENU_USE_PAGE_KEYS_GROUP3		'msg4'

class _LOCAL SunPinyinHandler : public CIMIWinHandler
{
public:
	SunPinyinHandler(SunPinyinModule *module, const BMessenger &status_msgr);
	virtual ~SunPinyinHandler();

	virtual void	commit(const TWCHAR* wstr);
	virtual void	updatePreedit(const IPreeditString* ppd);
	virtual void	updateCandidates(const ICandidateList* pcl);
	virtual void	updateStatus(int key, int value);

	void		Reset();

private:
	SunPinyinModule *fModule;
	bool started_sent;
	BMessenger fStatusWinMessenger;

	void		GenerateStartedMessage();
};


class _LOCAL SunPinyinMessageHandler : public BHandler
{
public:
	SunPinyinMessageHandler(SunPinyinModule *module, const BMessenger &status_msgr);
	virtual ~SunPinyinMessageHandler();

	virtual void	MessageReceived(BMessage *msg);

private:
	SunPinyinModule *fModule;
	BMessenger fStatusWinMessenger;
};


class _LOCAL SunPinyinStatusWindow : public BWindow
{
public:
	SunPinyinStatusWindow();
	virtual ~SunPinyinStatusWindow();

	virtual void DispatchMessage(BMessage *msg, BHandler *target);

private:
	BView *fCandidates;
	int32 fCaret;
};

#endif /* __SUNPINYIN_BE_HANDLER_H__ */

