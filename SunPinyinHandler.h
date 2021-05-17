#ifndef __SUNPINYIN_BE_HANDLER_H__
#define __SUNPINYIN_BE_HANDLER_H__

#include "SunPinyinModule.h"

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

private:
	SunPinyinModule *fModule;
	bool started_sent;

	void		GenerateStartedMessage();
};


class _LOCAL SunPinyinMessageHandler : public BHandler
{
public:
	SunPinyinMessageHandler(SunPinyinModule *module);
	virtual ~SunPinyinMessageHandler();

	virtual void	MessageReceived(BMessage *msg);

private:
	SunPinyinModule *fModule;
};

#endif /* __SUNPINYIN_BE_HANDLER_H__ */

