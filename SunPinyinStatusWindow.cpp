#include "SunPinyinStatusWindow.h"

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

					// some platforms like EIME-XIM, the height reply maybe set to 0
					if(h <= 0)
						h = Frame().Height();

					// adjust the postion
					if(IsHidden())
						MoveTo(where + BPoint(0, h + 2));
					BScreen screen(this);
					BRect scrRect = screen.Frame();
					BRect rect = Frame().OffsetToSelf(where + BPoint(0, h + 2));
					if(rect.bottom > scrRect.bottom && where.y - rect.Height() - 2 > scrRect.top)
						rect.OffsetBy(0, where.y - rect.Height() - 2 - rect.top);
					if(rect.left < scrRect.left)
						rect.OffsetBy(scrRect.left - rect.left, 0);
					else if(rect.right > scrRect.right && rect.Width() < scrRect.Width())
						rect.OffsetBy(scrRect.right - rect.right, 0);
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

