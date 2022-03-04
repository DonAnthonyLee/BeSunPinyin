#include <wchar.h>
#include <stdlib.h>
#include "SunPinyinHandler.h"


static char* utf32_to_utf8(const TWCHAR *wstr)
{
#ifndef __LITE_BEAPI__
	if(wstr == NULL || *wstr == 0) return NULL;

	// NOTE: Here we use native conversion instead of iconv
	int32 uLen = (int32)WCSLEN(wstr);
	int32 len = uLen * 6 + 1;
	char *s = (char*)malloc((size_t)len);
	if(s)
	{
		int32 state = 0;
		bzero(s, len);

		uint16 *uStr = (uint16*)malloc((size_t)uLen * 2 + 1);
		if(uStr)
		{
			int32 sLen = 0;
			bzero(uStr, (size_t)uLen * 2 + 1);

			uint16 *p = uStr;
			for(int32 k = 0; k < uLen; k++)
			{
				uint32 c = (uint32)wstr[k];
				if(c > 0xffff && c <= 0x10ffff)
				{
					*p++ = B_HOST_TO_BENDIAN_INT16(0xd800 | ((c - 0x10000) >> 10));
					*p++ = B_HOST_TO_BENDIAN_INT16(0xdc00 | ((c - 0x10000) & 0x03ff));
					sLen += 4;
				}
				else
				{
					*p++ = B_HOST_TO_BENDIAN_INT16(c & 0xffff);
					sLen += 2;
				}
			}

			if(convert_to_utf8(B_UNICODE_CONVERSION,
					   (const char*)uStr, &sLen,
					   s, &len, &state) == B_OK)
			{
				free(uStr);
				return s;
			}

			free(uStr);
		}

		free(s);
	}
	return NULL;
#else
	return e_utf32_convert_to_utf8((const eunichar32*)wstr, -1);
#endif
}


SunPinyinHandler::SunPinyinHandler(SunPinyinModule *module)
	: CIMIWinHandler(), fModule(module), fPreeditStartedSent(false)
{
	// TODO
#ifdef INPUT_SERVER_MORE_SUPPORT
	fStatusResponded = false;
	fStatusMaxRows = 1;
	fStatusMaxColumns = 10;
	fStatusSupports = 0;
	fCandidates = NULL;
	fCandidatesOffset = 0;
	fCandidatesRows =  1;
	fCandidatesSelection = -1;
#endif
}


SunPinyinHandler::~SunPinyinHandler()
{
	// TODO
}


void
SunPinyinHandler::Reset()
{
	fPreeditStartedSent = false;

#ifdef INPUT_SERVER_MORE_SUPPORT
	fStatusResponded = false;
	fCandidates = NULL;
	fCandidatesOffset = 0;
	fCandidatesSelection = -1;

	// NOTE:
	//	Because the status' responding messenger might be different from the method's,
	// and module call this->Reset() when received E_INPUT_METHOD_STOPPED, we should inform
	// the status' responding messenger to stop.
	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_STOPPED);
	if(fModule->EnqueueMessage(msg) != B_OK) delete msg;
#else
	BMessage aMsg(B_INPUT_METHOD_EVENT);
	aMsg.AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STOPPED);
	fStatusWinMessenger.SendMessage(&aMsg);
#endif
}


#ifndef INPUT_SERVER_MORE_SUPPORT
void
SunPinyinHandler::SetStatusWindowMessenger(const BMessenger &msgr)
{
	fStatusWinMessenger = msgr;
}
#else // INPUT_SERVER_MORE_SUPPORT
void
SunPinyinHandler::StatusResponded(const BMessage *msg)
{
	int32 action;
	if(msg->FindInt32("action", &action) != B_OK) return;

	switch(action)
	{
		case E_INPUT_METHOD_STATUS_CONFIG:
			msg->FindInt32("max_rows", &fStatusMaxRows);
			if(fStatusMaxRows > STATUS_MAX_ROWS)
				fStatusMaxRows = STATUS_MAX_ROWS;
			msg->FindInt32("max_columns", &fStatusMaxColumns);
			if(fStatusMaxColumns > STATUS_MAX_COLUMNS)
				fStatusMaxColumns = STATUS_MAX_COLUMNS;
			msg->FindInt32("supports", &fStatusSupports);
			fStatusResponded = true;
			if(fCandidates != NULL)
			{
				updateCandidates(fCandidates);
				fModule->EnqueueMessageOutList();
			}
			break;

		case E_INPUT_METHOD_STATUS_CONFIRM_STRING:
			// TODO
			break;

		case E_INPUT_METHOD_STATUS_FOCUS_CHANGED:
			// TODO
			break;

		case E_INPUT_METHOD_STATUS_SHIFT_REQUEST:
			// TODO
			break;

		case E_INPUT_METHOD_STATUS_TIPS_HIDDEN:
			// TODO
			break;

		default:
			break;
	}
}

void
SunPinyinHandler::LocationReplied(const BMessage *msg_loc)
{
	BPoint where;
	float h;

	fSpecificArea = BRect();

	for(int32 k = 0;
	    (msg_loc->FindPoint(IME_LOCATION_REPLY_DESC, k, &where) == B_OK &&
	     msg_loc->FindFloat(IME_HEIGHT_REPLY_DESC, k, &h) == B_OK);
	    k++)
	{
		BRect r;
		r.SetLeftTop(where);
		r.SetRightBottom(where + BPoint(h, h));
		fSpecificArea |= r;
	}

	if(fStatusResponded == false || fCandidates == NULL) return;

	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_CHANGED);
	msg->AddInt32("rows", fCandidatesRows);
	msg->AddInt32("columns", min_c(fCandidates->total() - fCandidates->first() - fCandidatesOffset, fStatusMaxColumns));
	if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_LABELS)
	{
		// TODO: row_label
		for(int32 k = 0; k < fStatusMaxColumns; k++)
		{
			BString aStr;
			aStr << ((k == 9) ? 0 : (k + 1));

			msg->AddString("column_label", aStr.String());
		}
	}

	msg->AddInt32("total", fCandidates->total());
	msg->AddInt32("offset", fCandidates->first() + fCandidatesOffset);

	if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION) &&
	   fCandidatesSelection >= 0)
		msg->AddInt32("selection", fCandidatesSelection);

	BString bestWords;
	for(int k = 0, m = 0;
	    m < min_c(fCandidates->total() - fCandidates->first() - fCandidatesOffset,
		      fCandidatesRows * fStatusMaxColumns) &&
		fCandidatesOffset + k < fCandidates->size();
	    k++)
	{
		char *s = utf32_to_utf8(fCandidates->candiString(fCandidatesOffset + k));
		if(s)
		{
			if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_TIPS) &&
			   fCandidates->candiType(fCandidatesOffset + k) == ICandidateList::BEST_TAIL &&
			   bestWords.Length() == 0)
			{
				bestWords.SetTo(s);
			}
			else
			{
				if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_LABELS) == 0)
				{
					BString aStr;
					aStr << (((k % fStatusMaxColumns) == 9) ? 0 : ((k % fStatusMaxColumns) + 1));
					aStr << ". " << s;
					msg->AddString("string", aStr.String());
				}
				else
				{
					msg->AddString("string", s);
				}
				m++;
			}
			free(s);
		}
	}

	if(fSpecificArea.IsValid())
		msg->AddRect("specific_area", fSpecificArea);

	fModule->AddMessageToOutList(msg);

	// TODO
	if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_TIPS)
	{
		msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_TIPS_CHANGED);
		if(bestWords.Length() > 0)
			msg->AddString("string", bestWords.String());
		fModule->AddMessageToOutList(msg);
	}

	fModule->EnqueueMessageOutList();
}
#endif // !INPUT_SERVER_MORE_SUPPORT


void
SunPinyinHandler::GeneratePreeditStartedMessage()
{
	if(fPreeditStartedSent) return;

	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STARTED);
	msg->AddMessenger(IME_REPLY_DESC, fModule->GetHandlerMessenger());
#ifndef INPUT_SERVER_MORE_SUPPORT
	fStatusWinMessenger.SendMessage(msg);
#endif
	fModule->AddMessageToOutList(msg);

	fPreeditStartedSent = true;

#ifdef INPUT_SERVER_MORE_SUPPORT
	GenerateStatusStartedMessage();
#endif
}


#ifdef INPUT_SERVER_MORE_SUPPORT
void
SunPinyinHandler::GenerateStatusStartedMessage()
{
	if(fStatusResponded) return;

	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_STARTED);
	msg->AddMessenger(IME_REPLY_DESC, fModule->CurrentHandlerMessenger());
	//msg->AddString("tab_label", "待选字词");
	fModule->AddMessageToOutList(msg);
}
#endif


void
SunPinyinHandler::commit(const TWCHAR* wstr)
{
	BMessage *msg;

	GeneratePreeditStartedMessage();

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


inline int LENGTH_CONVERT_TO_UTF8(TWCHAR c)
{
	if(c < 0x80) return 1;
	if(c < 0x800) return 2;
	if(c < 0x10000) return 3;
	if(c < 0x200000) return 4;
	return(c < 0x4000000 ? 5 : 6);
}


void
SunPinyinHandler::updatePreedit(const IPreeditString* ppd)
{
	const TWCHAR *wstr = ppd->string();
	char *str = utf32_to_utf8(wstr);
	BMessage *msg;

	if(str == NULL)
	{
		msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STOPPED);
		fModule->AddMessageToOutList(msg);

		Reset();
		return;
	}

	GeneratePreeditStartedMessage();

	msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_CHANGED);
	if(str)
	{
		BString aStr(str);
		free(str);

		msg->AddInt32(IME_CLAUSE_START_DESC, 0);
		msg->AddInt32(IME_CLAUSE_END_DESC, aStr.Length());

		// NOTE: on Win32, wchar_t is in unicode, not UTF32, so we use the WCSLEN() of SunPinyin
		int32 caret;
		size_t len = WCSLEN(wstr);
		if(ppd->caret() < (int)len)
		{
			caret = 0;
			for(int k = 0; k < ppd->caret(); k++)
				caret += LENGTH_CONVERT_TO_UTF8(wstr[k]);
		}
		else
		{
			caret = aStr.Length();
		}

#ifdef __LITE_BEAPI__
		msg->AddInt32(IME_SELECTION_DESC, caret);
		msg->AddInt32(IME_SELECTION_DESC, caret);
#else
		if(caret == aStr.Length()) aStr.Append(" ");
		msg->AddInt32(IME_SELECTION_DESC, caret);
		msg->AddInt32(IME_SELECTION_DESC, caret + 1);
#endif

		msg->AddString(IME_STRING_DESC, aStr.String());
	}
	msg->AddBool(IME_CONFIRMED_DESC, false);
#ifndef INPUT_SERVER_MORE_SUPPORT
	// Notify StatusWindow that the position of caret changed
	fStatusWinMessenger.SendMessage(msg);
#endif
	fModule->AddMessageToOutList(msg);
}


void
SunPinyinHandler::updateCandidates(const ICandidateList* pcl)
{
#ifndef INPUT_SERVER_MORE_SUPPORT
	BString aStr;

	// TODO: do more than simple string
	for(int k = 0; k < min_c(pcl->size(), 10); k++)
	{
		char *s = utf32_to_utf8(pcl->candiString(k));
		if(s)
		{
			if(aStr.Length() > 0) aStr << "  ";
			aStr << ((k == 9) ? 0 : (k + 1)) << ". " << s;
			free(s);
		}
	}

	BMessage aMsg(B_INPUT_METHOD_EVENT);
	// TODO: find another way, it's NOT GOOD!
	aMsg.AddInt32(IME_OPCODE_DESC, 1234); // custom opcode
	if(aStr.Length() > 0)
		aMsg.AddString("candidates", aStr.String());
	fStatusWinMessenger.SendMessage(&aMsg);

	if(aStr.Length() > 0)
	{
		BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_LOCATION_REQUEST);
		fModule->AddMessageToOutList(msg);
	}
#else // INPUT_SERVER_MORE_SUPPORT
	BMessage *msg;

	if(pcl->size() == 0)
	{
		if(fStatusResponded == false) return;

		fStatusResponded = false;
		fCandidates = NULL;
		fCandidatesOffset = 0;
		fCandidatesSelection = -1;

		msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_STOPPED);
		fModule->AddMessageToOutList(msg);
	}
	else
	{
		msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_LOCATION_REQUEST);
		fModule->AddMessageToOutList(msg);

		fCandidates = pcl;
		if(fStatusResponded == false)
			GenerateStatusStartedMessage();
	}
#endif
}


void
SunPinyinHandler::updateStatus(int key, int value)
{
	// TODO: use BInputServerMethod::SetIcon()
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


#ifndef INPUT_SERVER_MORE_SUPPORT
void
SunPinyinMessageHandler::SetStatusWindowMessenger(const BMessenger &msgr)
{
	fStatusWinMessenger = msgr;
}
#endif


void
SunPinyinMessageHandler::MessageReceived(BMessage *msg)
{
	uint8 bit = 0;

	if(msg->what == B_INPUT_METHOD_EVENT)
	{
		if(fModule->Lock() == false) return;
		BMessenger msgr = fModule->CurrentHandlerMessenger();
		fModule->Unlock();
		if(msgr.Target(NULL) != this) return;

		int32 opcode = 0;
		msg->FindInt32(IME_OPCODE_DESC, &opcode);

		// TODO
		switch(opcode)
		{
#ifndef INPUT_SERVER_MORE_SUPPORT
			case B_INPUT_METHOD_LOCATION_REQUEST:
				fStatusWinMessenger.SendMessage(msg);
				break;
#else // INPUT_SERVER_MORE_SUPPORT
			case B_INPUT_METHOD_LOCATION_REQUEST:
				if(fModule->Lock() == false) break;
				fModule->IMHandler()->LocationReplied(msg);
				fModule->Unlock();
				break;

			case E_INPUT_METHOD_STATUS_RESPONDED:
				if(fModule->Lock() == false) break;
				fModule->IMHandler()->StatusResponded(msg);
				fModule->Unlock();
				break;
#endif // !INPUT_SERVER_MORE_SUPPORT

			case B_INPUT_METHOD_STOPPED:
				if(fModule->Lock() == false) break;
				fModule->ResetSunPinyin();
				fModule->Unlock();
				break;

			default:
				break;
		}
	}
	else switch(msg->what) // as menu handler
	{
		case MSG_MENU_SWITCH_EN_CN_BY_SHIFT_KEY:
			fModule->Lock();
			fModule->SwitchShiftKeyUsing();
			fModule->Unlock();
			break;

		case MSG_MENU_USE_PAGE_KEYS_GROUP3:
			bit = 2;
		case MSG_MENU_USE_PAGE_KEYS_GROUP2:
			if(bit == 0) bit = 1;
		case MSG_MENU_USE_PAGE_KEYS_GROUP1:
			fModule->Lock();
			fModule->SwitchPageKeysGroup(bit);
			fModule->Unlock();
			break;

		default:
			break;
	}
}


#ifndef INPUT_SERVER_MORE_SUPPORT
SunPinyinStatusWindow::SunPinyinStatusWindow()
	: BWindow(BRect(0, 0, 100, 30), NULL, B_NO_BORDER_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL, B_AVOID_FOCUS),
	  fCaret(-1)
{
	BView *topView = new BView(Bounds(), NULL, B_FOLLOW_ALL, 0);
	topView->SetViewColor(0, 0, 0); // like border
	AddChild(topView);

	// TODO: do more than this simple string view
	fCandidates = new BStringView(Bounds().InsetBySelf(1, 1), NULL, NULL, B_FOLLOW_ALL);
	cast_as(fCandidates, BStringView)->SetAlignment(B_ALIGN_CENTER);
	fCandidates->SetFontSize(be_plain_font->Size() * 1.2f);
	topView->AddChild(fCandidates);

	// NOTE:
	//	On HaikuOS, SetViewColor() only make effection when view added to it's parent view,
	// otherwise, it's same as parent view.
	fCandidates->SetViewColor(255, 220, 0);
}


SunPinyinStatusWindow::~SunPinyinStatusWindow()
{
	// TODO
}


void
SunPinyinStatusWindow::DispatchMessage(BMessage *msg, BHandler *target)
{
	if(msg->what == B_INPUT_METHOD_EVENT)
	{
		int32 opcode = 0;
		msg->FindInt32(IME_OPCODE_DESC, &opcode);

		switch(opcode)
		{
			case B_INPUT_METHOD_CHANGED:
				{
					BString aStr;
					int32 index = -1;
					if(msg->FindString(IME_STRING_DESC, &aStr) != B_OK) break;

					// for now, we use selection to simulate caret position
					msg->FindInt32(IME_SELECTION_DESC, &index);

					// convert bytes offset to chars offset
					fCaret = -1;
					if(index >= 0 && index <= aStr.Length())
					{
						aStr.Truncate(index);
						fCaret = aStr.CountChars();
					}
				}
				break;

			case B_INPUT_METHOD_LOCATION_REQUEST:
				{
					// NOTE:
					//	The location reply maybe delay after current status.
					// Message handled like below:
					// The 1st. round (request)
					//	SunPinyinModule -> Input Server -> View
					//			-> SunPinyinStatusWindow (store current state through SunPinyinHandler)
					// The 2nd. round (reply)
					//		View -> Input Server -> SunPinyinMessageHandler -> SunPinyinStatusWindow
					//
					// So, probably the location reply just match the previous request, event such before last STOPPED.
					// The way to resolve it is:
					//	1. Filling up different BMessenger to IME_REPLY_DESC at each STARTED;
					//	2. Checking whether the reply is fit with current status, like using count, &etc.

					BString aStr;
					fCandidatesMsg.FindString("candidates", &aStr);
					if(aStr.Length() == 0) break;

					float w = 0, h = 0;
					cast_as(fCandidates, BStringView)->SetText(aStr.String());
					fCandidates->GetPreferredSize(&w, &h);
					ResizeTo(w + 2, h + 2);

					int32 pos = ((fCaret < 0) ? 0 : fCaret);
					BPoint where;

					if(!(msg->FindPoint(IME_LOCATION_REPLY_DESC, pos, &where) == B_OK &&
					     msg->FindFloat(IME_HEIGHT_REPLY_DESC, pos, &h) == B_OK))
					{
						if(pos == 0) break;
						// some platforms like EIME-XIM, the location reply is single when using XIMPreeditCallback style
					   	if(!(msg->FindPoint(IME_LOCATION_REPLY_DESC, 0, &where) == B_OK &&
					   	     msg->FindFloat(IME_HEIGHT_REPLY_DESC, 0, &h) == B_OK)) break;
					}

					// adjust the postion
					BScreen screen(this);
					BRect scrRect = screen.Frame().OffsetToSelf(B_ORIGIN);
					BRect rect = Frame().OffsetToSelf(where + BPoint(0, h + 2));
					if(scrRect.Contains(rect) == false)
					{
						if(rect.bottom > scrRect.bottom)
						{
							if(h > 0.f)
								rect.OffsetBy(0, where.y - rect.Height() - 2 - rect.top);
							else // some platforms like EIME-XIM, the height reply maybe set to 0
								rect.OffsetBy(0, where.y - 2 * rect.Height() - 2 - rect.top);
							if(rect.bottom > scrRect.bottom)
								rect.OffsetBy(0, scrRect.bottom - rect.bottom);
						}
						if(rect.left < 0)
							rect.OffsetBy(-rect.left, 0);
						else if(rect.right > scrRect.right && rect.Width() < scrRect.Width())
							rect.OffsetBy(scrRect.right - rect.right, 0);
					}
					MoveTo(rect.LeftTop());

					if(IsHidden()) Show();
					SendBehind(NULL);
				}
				break;

			case B_INPUT_METHOD_STARTED:
				break;

			case B_INPUT_METHOD_STOPPED:
				// NOTE: On HaikuOS, hide a hidden window cause issues.
				if(!IsHidden()) Hide();
				cast_as(fCandidates, BStringView)->SetText("");
				break;

			case 1234: // custom message for candidates list
				if(msg->HasString("candidates") == false)
				{
					fCandidatesMsg.MakeEmpty();

					// NOTE: On HaikuOS, hide a hidden window cause issues.
					if(!IsHidden()) Hide();
				}
				else
				{
					fCandidatesMsg = *msg;
				}
				break;

			default:
				break;
		}
	}
	else
	{
		BWindow::DispatchMessage(msg, target);
	}
}
#endif // !INPUT_SERVER_MORE_SUPPORT

