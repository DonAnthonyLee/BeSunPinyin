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
	SunPinyinHandler(SunPinyinModule *module);
	virtual ~SunPinyinHandler();

	virtual void	commit(const TWCHAR* wstr);
	virtual void	updatePreedit(const IPreeditString* ppd);
	virtual void	updateCandidates(const ICandidateList* pcl);
	virtual void	updateStatus(int key, int value);

	void		Reset();
#ifndef INPUT_SERVER_MORE_SUPPORT
	void		SetStatusWindowMessenger(const BMessenger &msgr);
#else
	void		StatusResponded(const BMessage *msg);
	void		LocationReplied(const BMessage *msg);
	bool		checkKeyEvent(CKeyEvent &key);
#endif

private:
	SunPinyinModule *fModule;
	bool fPreeditStartedSent;
#ifdef INPUT_SERVER_MORE_SUPPORT
	bool fStatusStopped;
	bool fStatusResponded;
	int32 fStatusMaxRows;
	int32 fStatusMaxColumns;
	int32 fStatusSupports;
	BRect fSpecificArea;

	const ICandidateList* fCandidates;
	int32 fCandidatesOffset;
	int32 fBestWordsOffset;
	int32 fCandidatesSelection;
	int32 fCandidatesRows;
	int32 fCaret;
#else
	BMessenger fStatusWinMessenger;
#endif

	void		GeneratePreeditStartedMessage();
#ifdef INPUT_SERVER_MORE_SUPPORT
	void		GenerateStatusStartedMessage();
	bool		IsStatusSelectionValid(int32 selection) const;
	void		StatusSelectionChanged();
#endif
};


class _LOCAL SunPinyinMessageHandler : public BHandler
{
public:
	SunPinyinMessageHandler(SunPinyinModule *module);
	virtual ~SunPinyinMessageHandler();

#ifndef INPUT_SERVER_MORE_SUPPORT
	void		SetStatusWindowMessenger(const BMessenger &msgr);
#endif

	virtual void	MessageReceived(BMessage *msg);

private:
	SunPinyinModule *fModule;
#ifndef INPUT_SERVER_MORE_SUPPORT
	BMessenger fStatusWinMessenger;
#endif
};

#endif /* __SUNPINYIN_BE_HANDLER_H__ */

