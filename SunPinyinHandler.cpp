#include <wchar.h>
#include <stdlib.h>
#include "SunPinyinHandler.h"


static char* utf32_to_utf8(const TWCHAR *wstr)
{
#ifndef __LITE_BEAPI__
	// BeOS always use UTF-8
	size_t len = wcslen(wstr) * 6 + 1;
	char *s = (char*)malloc(len);
	if(s)
	{
		memset(s, 0, len);
		wcstombs(s, wstr, len);
	}
	return s;
#else
	return e_utf32_convert_to_utf8(wstr, -1);
#endif
}


SunPinyinHandler::SunPinyinHandler(SunPinyinModule *module)
	: CIMIWinHandler(), fModule(module), started_sent(false)
{
	// TODO
}


SunPinyinHandler::~SunPinyinHandler()
{
	// TODO
}


void
SunPinyinHandler::Reset()
{
	started_sent = false;
}


void
SunPinyinHandler::GenerateStartedMessage()
{
	if(started_sent) return;

	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STARTED);
	msg->AddMessenger(IME_REPLY_DESC, fModule->HandlerMessenger());
	fModule->AddMessageToOutList(msg);

	started_sent = true;
}


void
SunPinyinHandler::commit(const TWCHAR* wstr)
{
	BMessage *msg;

	GenerateStartedMessage();

	char *str = utf32_to_utf8(wstr);
	if(str)
	{
		msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_CHANGED);
		msg->AddString(IME_STRING_DESC, str);
		msg->AddBool(IME_CONFIRMED_DESC, true);
		fModule->AddMessageToOutList(msg);
		free(str);
	}

	msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STOPPED);
	fModule->AddMessageToOutList(msg);

	Reset();
}


void
SunPinyinHandler::updatePreedit(const IPreeditString* ppd)
{
	const TWCHAR *wstr = ppd->string();
	char *str = utf32_to_utf8(wstr);

	GenerateStartedMessage();

	// TODO: ppd->caret(), &etc.
	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_CHANGED);
	if(str)
	{
		msg->AddInt32(IME_CLAUSE_START_DESC, 0);
		msg->AddInt32(IME_CLAUSE_END_DESC, strlen(str));
		msg->AddString(IME_STRING_DESC, str);
	}
	msg->AddBool(IME_CONFIRMED_DESC, false);

	fModule->AddMessageToOutList(msg);

	if(str) free(str);
}


void
SunPinyinHandler::updateCandidates(const ICandidateList* pcl)
{
	// TODO
}


void
SunPinyinHandler::updateStatus(int key, int value)
{
	// TODO
}


SunPinyinMessageHandler::SunPinyinMessageHandler(SunPinyinModule *module)
	: BHandler(), fModule(module)
{
	// TODO
}


SunPinyinMessageHandler::~SunPinyinMessageHandler()
{
	// TODO
}


void
SunPinyinMessageHandler::MessageReceived(BMessage *msg)
{
	if(msg->what == B_INPUT_METHOD_EVENT)
	{
		int32 opcode = 0;
		msg->FindInt32(IME_OPCODE_DESC, &opcode);

		// TODO
		switch(opcode)
		{
			case B_INPUT_METHOD_STOPPED:
				fModule->ResetSunPinyin();
				break;

			default:
				break;
		}
	}
}

