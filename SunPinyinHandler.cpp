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
	fStatusStopped = true;
	fStatusResponded = false;
	fStatusMaxRows = 1;
	fStatusMaxColumns = 10;
	fStatusSupports = 0;
	fCandidates = NULL;
	fCandidatesOffset = 0;
	fBestWordsOffset = -1;
	fCandidatesRows = 1;
	fCandidatesColumns = 0;
	fCandidatesSelection = -1;
	fCaret = 0;
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
	fStatusStopped = true;
	fStatusResponded = false;
	fCandidates = NULL;
	fCandidatesOffset = 0;
	fBestWordsOffset = -1;
	fCandidatesRows = 1;
	fCandidatesColumns = 0;
	fCandidatesSelection = -1;
	fCaret = 0;

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
			if(fStatusStopped)
			{
				// NOTE: Crossed response happened.
				//    STARTED -> Server/Target
				//       |         /|
				//    STOPPED     / |
				//       |   \   /  |
				//   RESPONDED<-/   |
				//             \    |
				//              \->STOPPED
				break;
			}

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

		case E_INPUT_METHOD_STATUS_FOCUS_CHANGED:
			if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION)
			{
				int32 btns = -1;
				int32 row_index, column_index;
				if(msg->FindInt32("row_index", &row_index) != B_OK ||
				   msg->FindInt32("column_index", &column_index) != B_OK ||
				   row_index < 0 || column_index < 0) break;
				msg->FindInt32("buttons", &btns);

				if(IsStatusSelectionValid(row_index * fStatusMaxColumns + column_index))
				{
					fCandidatesSelection = row_index * fStatusMaxColumns + column_index;
					StatusSelectionChanged();

					if(btns >= 0)
					{
						CKeyEvent key(IM_VK_SPACE, ' ', 0);
						checkKeyEvent(key);
						fModule->EnqueueMessageOutList();
					}
				}
			}
			break;

		case E_INPUT_METHOD_STATUS_SHIFT_REQUEST:
			if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION)
			{
				int32 offset;
				if(msg->FindInt32("offset", &offset) == B_OK)
				{
					if(offset == fCandidatesRows * fCandidatesColumns)
					{
						CKeyEvent key(IM_VK_PAGE_DOWN, 0, 0);
						checkKeyEvent(key);
						fModule->EnqueueMessageOutList();
					}
					else if(offset == -fCandidatesRows * fCandidatesColumns)
					{
						CKeyEvent key(IM_VK_PAGE_UP, 0, 0);
						checkKeyEvent(key);
						fModule->EnqueueMessageOutList();
					}
					else // free offset by scrollbar, &etc.
					{
						// TODO
					}
				}
				else if((fCandidatesRows == 1 || fCandidatesSelection >= 0) &&
					(msg->HasInt32("row_index") || msg->HasInt32("column_index")))
				{
					int32 row_index = ((fCandidatesSelection >= 0) ?
								(fCandidatesSelection / fStatusMaxColumns) : 0);
					int32 column_index = ((fCandidatesSelection >= 0) ?
								(fCandidatesSelection % fStatusMaxColumns) : 0);
					msg->FindInt32("row_index", &row_index);
					msg->FindInt32("column_index", &column_index);
					if(row_index >= 0 && column_index >= 0)
					{
						fCandidatesSelection = row_index * fStatusMaxColumns + column_index;
						StatusSelectionChanged();
					}
				}
			}
			break;

		case E_INPUT_METHOD_STATUS_CONFIRM_STRING:
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

	if(fStatusResponded == false ||
	   fCandidates == NULL ||
	   fCandidates->size() <= fCandidatesOffset) return;

	fSpecificArea = BRect();

	for(int32 k = (msg_loc->HasPoint(IME_LOCATION_REPLY_DESC, fCaret) ? fCaret : 0);
	    (msg_loc->FindPoint(IME_LOCATION_REPLY_DESC, k, &where) == B_OK &&
	     msg_loc->FindFloat(IME_HEIGHT_REPLY_DESC, k, &h) == B_OK);
	    k++)
	{
		BRect r;
		r.SetLeftTop(where);
		r.SetRightBottom(where + BPoint(h, h));
		fSpecificArea |= r;
	}

	BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_CHANGED);
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

	int32 m = 0;
	BString bestWords;
	fBestWordsOffset = -1;
	for(int k = 0;
	    m < fCandidatesRows * fStatusMaxColumns &&
		fCandidatesOffset + k < fCandidates->size();
	    k++)
	{
		char *s = utf32_to_utf8(fCandidates->candiString(fCandidatesOffset + k));
		if(s)
		{
			if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_TIPS) &&
			   fCandidates->candiType(fCandidatesOffset + k) == ICandidateList::BEST_TAIL &&
			   bestWords.Length() == 0 &&
			   (fCandidates->size() > fCandidatesOffset + 1 || fCandidates->total() == 1))
			{
				bestWords.SetTo(s);
				fBestWordsOffset = k;
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

	// NOTE: libsunpinyin has BUGS, so we use "m" to make it right.
	int32_t total = fCandidates->total() - ((fBestWordsOffset >= 0) ? 1 : 0);
	if(m < fCandidatesRows * fStatusMaxColumns &&
	  fCandidates->total() > fCandidates->first() + fCandidatesOffset + m)
		total = fCandidates->first() + fCandidatesOffset + m;

	msg->AddInt32("total", total);
	msg->AddInt32("offset", fCandidates->first() + fCandidatesOffset);

	fCandidatesColumns = min_c(m, fStatusMaxColumns);
	msg->AddInt32("rows", fCandidatesRows);
	msg->AddInt32("columns", fCandidatesColumns);

	if(IsStatusSelectionValid(fCandidatesSelection) == false)
		fCandidatesSelection = -1;
	if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION)
		msg->AddInt32("selection", fCandidatesSelection);

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


bool
SunPinyinHandler::IsStatusSelectionValid(int32 selection) const
{
	if(fCandidates == NULL && selection >= 0)
	{
		return false;
	}
	else if(fCandidates != NULL && selection >= 0)
	{
		if(fCandidates->size() <= fCandidatesOffset + selection + ((fBestWordsOffset < 0) ? 0 : 1) ||
		   selection >= fCandidatesRows * fStatusMaxColumns)
			return false;
	}

	return true;
}


void
SunPinyinHandler::StatusSelectionChanged()
{
	if(IsStatusSelectionValid(fCandidatesSelection) == false)
		fCandidatesSelection = -1;

	if(fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION)
	{
		BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_CHANGED);
		msg->AddInt32("selection", fCandidatesSelection);
		if(fModule->EnqueueMessage(msg) != B_OK) delete msg;
	}
}


bool
SunPinyinHandler::checkKeyEvent(CKeyEvent &key)
{
	bool retVal = false;
	if(fStatusResponded == false || fCandidates == NULL) return false;

	int32 candidates_size = fCandidates->size();
	if(fBestWordsOffset >= 0) candidates_size--;
	if(candidates_size > STATUS_MAX_ROWS * STATUS_MAX_COLUMNS)
		candidates_size = STATUS_MAX_ROWS * STATUS_MAX_COLUMNS;
	if(candidates_size <= 0) return false;

	switch(key.code)
	{
		case IM_VK_ESCAPE:
			if(fCandidatesSelection >= 0 && fBestWordsOffset >= 0)
			{
				fCandidatesSelection = -1;
				StatusSelectionChanged();
				retVal = true;
			}
			break;

		case IM_VK_SPACE:
			if(fCandidatesSelection >= 0)
			{
				unsigned mask = CIMIClassicView::CANDIDATE_MASK;
				unsigned id = fCandidatesOffset + fCandidatesSelection +
						((fBestWordsOffset <= fCandidatesSelection && fBestWordsOffset >= 0) ? 1 : 0);
				CIMIClassicView *im_view = cast_as(fModule->IMView(), CIMIClassicView);

				fCandidatesOffset = 0;
				fCandidatesSelection = -1; // no sense to keep the previous selection
				im_view->makeSelection(id, mask);
				fModule->IMView()->getHotkeyProfile()->rememberLastKey(key);
				im_view->updateWindows(mask);
				retVal = true;
			}
			break;

		case IM_VK_DOWN:
			if(fCandidatesRows != min_c(STATUS_MAX_ROWS, fStatusMaxRows) && key.modifiers == 0)
			{
				fCandidatesRows = min_c(STATUS_MAX_ROWS, fStatusMaxRows);

				int32 new_offset = (fCandidatesOffset / (fCandidatesRows * fStatusMaxColumns));
				new_offset *= (fCandidatesRows * fStatusMaxColumns);

				if(fCandidatesSelection >= 0)
					fCandidatesSelection += (fCandidatesOffset - new_offset);
				fCandidatesOffset = new_offset;

				updateCandidates(fCandidates);
			}
			else if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION) && key.modifiers != 0)
			{
				if(!(fCandidatesSelection >= 0 &&
				     (candidates_size <= fCandidatesOffset + fCandidatesSelection + fStatusMaxColumns ||
					fCandidatesSelection + fStatusMaxColumns >= fCandidatesRows * fStatusMaxColumns)))
				{
					if(fCandidatesSelection < 0)
						fCandidatesSelection = 0;
					else
						fCandidatesSelection += fStatusMaxColumns;

					StatusSelectionChanged();
				}
				retVal = true;
			}
			if(key.modifiers == 0) // to prevent ruining
				retVal = true;
			break;

		case IM_VK_UP:
			if(fCandidatesRows != 1 && key.modifiers == 0)
			{
				fCandidatesRows = 1;
				updateCandidates(fCandidates);
			}
			else if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION) && key.modifiers != 0)
			{
				if(fCandidatesSelection >= fStatusMaxColumns)
				{
					fCandidatesSelection -= fStatusMaxColumns;
					StatusSelectionChanged();
				}
				retVal = true;
			}
			if(key.modifiers == 0) // to prevent ruining
				retVal = true;
			break;

		case IM_VK_RIGHT:
			if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION) && key.modifiers != 0)
			{
				if(!(fCandidatesSelection >= 0 &&
				     (candidates_size <= fCandidatesOffset + fCandidatesSelection + 1 ||
					(fCandidatesSelection % fStatusMaxColumns) + 1 >= fStatusMaxColumns)))
				{
					if(fCandidatesSelection < 0)
						fCandidatesSelection = 0;
					else
						fCandidatesSelection += 1;

					StatusSelectionChanged();
				}
				retVal = true;
			}
			break;

		case IM_VK_LEFT:
			if((fStatusSupports & E_INPUT_METHOD_STATUS_SUPPORT_SELECTION) && key.modifiers != 0)
			{
				if((fCandidatesSelection % fStatusMaxColumns) > 0)
				{
					fCandidatesSelection--;
					StatusSelectionChanged();
				}
				retVal = true;
			}
			break;

		default:
			if(key.value >= ((fStatusMaxColumns < 10) ? '1' : '0') &&
			   key.value <= ((fStatusMaxColumns < 10) ? ('0' + fStatusMaxColumns) : '9') &&
			   key.modifiers == 0)
			{
				if(fCandidatesRows == 1 || fCandidatesSelection >= 0)
				{
					unsigned id = (unsigned)((key.value == '0') ? 9 : (key.value - '1'));
					if(fCandidatesSelection >= 0)
						id += (fCandidatesSelection / fStatusMaxColumns) * fStatusMaxColumns;
					if(fBestWordsOffset <= id && fBestWordsOffset >= 0) id++;
					id += fCandidatesOffset;

					if(id < candidates_size)
					{
						unsigned mask = CIMIClassicView::CANDIDATE_MASK;
						CIMIClassicView *im_view = cast_as(fModule->IMView(), CIMIClassicView);

						fCandidatesOffset = 0;
						im_view->makeSelection(id, mask);
						fModule->IMView()->getHotkeyProfile()->rememberLastKey(key);
						im_view->updateWindows(mask);
					}
				}

				retVal = true;
			}
			else
			{
				CHotkeyProfile *profile = fModule->IMView()->getHotkeyProfile();
				if((key.code == IM_VK_PAGE_DOWN && key.modifiers == 0) || profile->isPageDownKey(key))
				{
					if(fCandidates->total() >=
						fCandidates->first() + fCandidatesOffset +
						fCandidatesRows * fStatusMaxColumns)
					{
						if(candidates_size > fCandidatesOffset + fCandidatesRows * fStatusMaxColumns)
						{
							fCandidatesOffset += fCandidatesRows * fStatusMaxColumns;
							updateCandidates(fCandidates);
						}
						else
						{
							CIMIClassicView *im_view = cast_as(fModule->IMView(), CIMIClassicView);
							int pgno = fCandidates->first() / (STATUS_MAX_ROWS * STATUS_MAX_COLUMNS) + 1;

							fCandidatesOffset = 0;
							im_view->setCandiWindowSize(STATUS_MAX_ROWS * STATUS_MAX_COLUMNS);
							im_view->onCandidatePageRequest(pgno, false);
							im_view->setCandiWindowSize(STATUS_MAX_ROWS * STATUS_MAX_COLUMNS + 1);
							im_view->updateWindows(CIMIClassicView::CANDIDATE_MASK);
						}
					}

					retVal = true;
				}
				else if((key.code == IM_VK_PAGE_UP && key.modifiers == 0) || profile->isPageUpKey(key))
				{
					if(!(fCandidates->first() == 0 && fCandidatesOffset == 0))
					{
						if(fCandidatesOffset > 0)
						{
							fCandidatesOffset = max_c(0, fCandidatesOffset - fCandidatesRows * fStatusMaxColumns);
							updateCandidates(fCandidates);
						}
						else
						{
							CIMIClassicView *im_view = cast_as(fModule->IMView(), CIMIClassicView);
							int pgno = fCandidates->first() / (STATUS_MAX_ROWS * STATUS_MAX_COLUMNS);
							if(pgno > 0) pgno--;

							fCandidatesOffset = STATUS_MAX_ROWS * STATUS_MAX_COLUMNS - fCandidatesRows * fStatusMaxColumns;
							im_view->setCandiWindowSize(STATUS_MAX_ROWS * STATUS_MAX_COLUMNS);
							im_view->onCandidatePageRequest(pgno, false);
							im_view->setCandiWindowSize(STATUS_MAX_ROWS * STATUS_MAX_COLUMNS + 1);
							im_view->updateWindows(CIMIClassicView::CANDIDATE_MASK);
						}
					}

					retVal = true;
				}
			}
	}

	return retVal;
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
#if 0
	// E_INPUT_METHOD_STATUS_RESPONDED will use same timestamp,
	// we can use this to detect whether it's crossed.
	msg->AddInt64("when", system_time());
#endif
	msg->AddInt32(IME_OPCODE_DESC, E_INPUT_METHOD_STATUS_STARTED);
	msg->AddMessenger(IME_REPLY_DESC, fModule->CurrentHandlerMessenger());
	//msg->AddString("tab_label", "待选字词");
	fModule->AddMessageToOutList(msg);

	// expecting response
	fStatusStopped = false;
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

	BString aStr(str);
	if(str) free(str);

	if(aStr.Length() == 0)
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

	msg->AddInt32(IME_CLAUSE_START_DESC, 0);
	msg->AddInt32(IME_CLAUSE_END_DESC, aStr.Length());

	// NOTE: on Win32, wchar_t is in unicode, not UTF32, so we use the WCSLEN() of SunPinyin
	size_t len = WCSLEN(wstr);
	int32 caret;
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
	msg->AddBool(IME_CONFIRMED_DESC, false);
	fModule->AddMessageToOutList(msg);

#ifndef INPUT_SERVER_MORE_SUPPORT
	// Notify StatusWindow that the position of caret changed
	fStatusWinMessenger.SendMessage(msg);
#else
	aStr.Truncate(caret);
	fCaret = aStr.CountChars();

	msg = new BMessage(B_INPUT_METHOD_EVENT);
	msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_LOCATION_REQUEST);
	fModule->AddMessageToOutList(msg);
#endif
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

		fStatusStopped = true;
		fStatusResponded = false;
		fCandidates = NULL;
		fCandidatesOffset = 0;
		fBestWordsOffset = -1;
		fCandidatesRows = 1;
		fCandidatesColumns = 0;
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

