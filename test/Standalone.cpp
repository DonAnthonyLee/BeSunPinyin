#include <string.h>
#include <be/Be.h>

#include <sunpinyin.h>

#define MSG_SWITCH_CN		'mscn'
#define MSG_SWITCH_PUNC		'mspu'
#define MSG_SWITCH_SIMBOL	'mssb'


inline char* utf32_to_utf8(const TWCHAR* wstr)
{
#ifndef __LITE_BEAPI__
	if(wstr == NULL || *wstr == 0) return NULL;

	// NOTE: Here we use native conversion instead of iconv
	int32 uLen = (int32)wcslen((const wchar_t*)wstr);
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


inline int LENGTH_CONVERT_TO_UTF8(TWCHAR c)
{
	if(c < 0x80) return 1;
	if(c < 0x800) return 2;
	if(c < 0x10000) return 3;
	if(c < 0x200000) return 4;
	return(c < 0x4000000 ? 5 : 6);
}


class TWindow : public BWindow, public CIMIWinHandler
{
public:
	TWindow();
	virtual ~TWindow();

	status_t	InitCheck() const;

	// derived from BWindow
	virtual void	DispatchMessage(BMessage *msg, BHandler *handler);

	// derived from CIMIWinHandler
	virtual void	commit(const TWCHAR* wstr);
	virtual void	updatePreedit(const IPreeditString* ppd);
	virtual void	updateCandidates(const ICandidateList* pcl);
	virtual void	updateStatus(int key, int value);

private:
	CIMIView *fIMView;
};


TWindow::TWindow()
	: BWindow(BRect(100, 100, 600, 350), "SunPinyin Standalone", B_TITLED_WINDOW, B_QUIT_ON_WINDOW_CLOSE),
	  CIMIWinHandler(),
	  fIMView(NULL)
{
	CSunpinyinSessionFactory::getFactory().setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);

	fIMView = CSunpinyinSessionFactory::getFactory().createSession();
	fIMView->getIC()->setCharsetLevel(1); // GBK
	fIMView->attachWinHandler(this);

	BRect rect = Bounds();
	BView *topView = new BView(rect, NULL, B_FOLLOW_ALL, 0);
	topView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(topView);

	rect.bottom = 50;
	BBox *box = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT);
	box->SetLabel("Preedit");
	BTextView *textView = new BTextView(rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 15), "Preedit",
					    rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 15).OffsetToSelf(B_ORIGIN),
					    B_FOLLOW_ALL);
	textView->MakeEditable(false);
	textView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	box->AddChild(textView);
	topView->AddChild(box);

	rect.OffsetBy(0, 50);
	box = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT);
	box->SetLabel("Candidates");
	BStringView *strView = new BStringView(rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 15), "Candidates",
					       NULL, B_FOLLOW_ALL);
	box->AddChild(strView);
	topView->AddChild(box);

	rect.OffsetBy(0, 50);
	box = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT);
	box->SetLabel("Status");
	BRect r = rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 10);
	r.top += 10;
	r.right = r.left + 120;
	box->AddChild(new BButton(r, "Status::CN", "Chinese", new BMessage(MSG_SWITCH_CN), B_FOLLOW_LEFT));
	r = rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 10);
	r.top += 10;
	r.left += r.Width() / 2.f - 60;
	r.right -= r.left - 5;
	box->AddChild(new BButton(r, "Status::FULLPUNC", "Chinese Punc", new BMessage(MSG_SWITCH_PUNC), B_FOLLOW_H_CENTER));
	r = rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 10);
	r.top += 10;
	r.left = r.right - 120;
	box->AddChild(new BButton(r, "Status::FULLSYMBOL", "Full-width Symbol", new BMessage(MSG_SWITCH_SIMBOL), B_FOLLOW_RIGHT));
	topView->AddChild(box);

	rect.OffsetBy(0, 50);
	rect.bottom = Bounds().bottom;
	box = new BBox(rect, NULL, B_FOLLOW_ALL);
	box->SetLabel("Output");
	textView = new BTextView(rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 15), "Output",
						   rect.OffsetToCopy(B_ORIGIN).InsetBySelf(5, 15).OffsetToSelf(B_ORIGIN),
						   B_FOLLOW_ALL);
	textView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	box->AddChild(textView);
	topView->AddChild(box);

	fIMView->setStatusAttrValue(STATUS_ID_CN, 1);
	fIMView->setStatusAttrValue(STATUS_ID_FULLPUNC, 1);
	fIMView->setStatusAttrValue(STATUS_ID_FULLSYMBOL, 1);
}


TWindow::~TWindow()
{
	if(fIMView)
	  CSunpinyinSessionFactory::getFactory().destroySession(fIMView);
}


status_t
TWindow::InitCheck() const
{
	return(fIMView ? B_OK : B_ERROR);
}


void
TWindow::DispatchMessage(BMessage *msg, BHandler *handler)
{
	const char *viewName = "";
	if(!(handler == NULL ||
		 !is_kind_of(handler, BView) ||
		 cast_as(handler, BView)->Name() == NULL)) viewName = cast_as(handler, BView)->Name();

	if(strcmp(viewName, "Output") != 0)
	{
		switch(msg->what)
		{
			case MSG_SWITCH_CN:
				fIMView->setStatusAttrValue(STATUS_ID_CN, fIMView->getStatusAttrValue(STATUS_ID_CN) == 0 ? 1 : 0);
				return;

			case MSG_SWITCH_PUNC:
				fIMView->setStatusAttrValue(STATUS_ID_FULLPUNC, fIMView->getStatusAttrValue(STATUS_ID_FULLPUNC) == 0 ? 1 : 0);
				return;

			case MSG_SWITCH_SIMBOL:
				fIMView->setStatusAttrValue(STATUS_ID_FULLSYMBOL, fIMView->getStatusAttrValue(STATUS_ID_FULLSYMBOL) == 0 ? 1 : 0);
				return;

			case B_KEY_UP:
				return;

			case B_KEY_DOWN:
				{
					int32 key, modifiers;
					int8 byte;
					const char *bytes = NULL;

					if(msg->FindInt32("key", &key) != B_OK) break;
					if(msg->FindInt32("modifiers", &modifiers) != B_OK) break;
					msg->FindString("bytes", &bytes);
					if(msg->FindInt8("byte", 1, &byte) == B_OK || msg->FindInt8("byte", 2, &byte) == B_OK) break;
					if(msg->FindInt8("byte", &byte) != B_OK) break;
					if(!(bytes == NULL || (strlen(bytes) == 1 && *bytes == byte))) break;

			unsigned int keyCode = 0, keyState = 0;
					switch(byte)
					{
						case B_ENTER: keyCode = IM_VK_ENTER; break;
						case B_BACKSPACE: keyCode = IM_VK_BACK_SPACE; break;
						case B_ESCAPE: keyCode = IM_VK_ESCAPE; break;
						case B_PAGE_UP: keyCode = IM_VK_PAGE_UP; break;
						case B_PAGE_DOWN: keyCode = IM_VK_PAGE_DOWN; break;
						case B_END: keyCode = IM_VK_END; break;
						case B_HOME: keyCode = IM_VK_HOME; break;
						case B_LEFT_ARROW: keyCode = IM_VK_LEFT; break;
						case B_RIGHT_ARROW: keyCode = IM_VK_RIGHT; break;
						case B_UP_ARROW: keyCode = IM_VK_UP; break;
						case B_DOWN_ARROW: keyCode = IM_VK_DOWN; break;
						case B_DELETE: keyCode = IM_VK_DELETE; break;
						default: keyCode = (unsigned)byte;
					}

					switch(byte)
					{
						case B_ENTER:
						case B_BACKSPACE:
						case B_ESCAPE:
						case B_PAGE_UP:
						case B_PAGE_DOWN:
						case B_END:
						case B_HOME:
						case B_LEFT_ARROW:
						case B_RIGHT_ARROW:
						case B_UP_ARROW:
						case B_DOWN_ARROW:
						case B_DELETE:
							if(cast_as(FindView("Preedit"), BTextView)->TextLength() == 0)
							{
#ifdef __LITE_BEAPI__
								FindView("Output")->MakeFocus();
#endif
								BWindow::DispatchMessage(msg, FindView("Output"));
								break;
							}

						default:
							if(modifiers & B_SHIFT_KEY) keyState |= IM_SHIFT_MASK;
							if(modifiers & B_CONTROL_KEY) keyState |= IM_CTRL_MASK;
							if(modifiers & B_COMMAND_KEY) keyState |= IM_ALT_MASK;
							if(modifiers & B_MENU_KEY) keyState |= IM_SUPER_MASK;
							fIMView->onKeyEvent(CKeyEvent(keyCode, byte, keyState));
					}
				}
				return;

			default:
				break;
		}
	}

	BWindow::DispatchMessage(msg, handler);
}


void
TWindow::commit(const TWCHAR* wstr)
{
	char *s = utf32_to_utf8(wstr);
	BTextView *textView = cast_as(FindView("Output"), BTextView);
#ifndef __LITE_BEAPI__
	textView->Insert(s);
#else
	int32 pos = textView->Position();
	textView->Insert(pos, s, strlen(s));
	textView->SetPosition(pos + strlen(s));
	textView->Invalidate();
#endif
	free(s);
}


void
TWindow::updatePreedit(const IPreeditString* ppd)
{
	BTextView *textView = cast_as(FindView("Preedit"), BTextView);

	const TWCHAR *pwcs = ppd->string();
	char *s = utf32_to_utf8(pwcs);
	textView->SetText(s);
	free(s);

	size_t len = WCSLEN(pwcs);
	if(ppd->caret() < len)
	{
		int32 pos = 0;
		for(size_t k = 0; k < len && k != ppd->caret(); k++)
		{
			pos += LENGTH_CONVERT_TO_UTF8(pwcs[k]);
		}

#ifndef __LITE_BEAPI__
		textView->Highlight(pos, pos + LENGTH_CONVERT_TO_UTF8(pwcs[ppd->caret()]));
#else
		textView->Select(pos, pos + LENGTH_CONVERT_TO_UTF8(pwcs[ppd->caret()]));
#endif
	}
}


void
TWindow::updateCandidates(const ICandidateList* pcl)
{
	BString str;

	for(int i = 0; i < pcl->size(); i++)
	{
		char *s = utf32_to_utf8(pcl->candiString(i));
		str << i+1 << ". " << s << " ";
	free(s);
	}

	cast_as(FindView("Candidates"), BStringView)->SetText(str.String());
}


void
TWindow::updateStatus(int key, int value)
{
	BButton *statusCN = cast_as(FindView("Status::CN"), BButton);
	BButton *statusFullPunc = cast_as(FindView("Status::FULLPUNC"), BButton);
	BButton *statusFullSimbol = cast_as(FindView("Status::FULLSYMBOL"), BButton);

	if(key == STATUS_ID_CN)
		statusCN->SetLabel(value == 0 ? "English" : "Chinese");
	else if(key == STATUS_ID_FULLPUNC)
		statusFullPunc->SetLabel(value == 0 ? "Normal Punc" : "Chinese Punc");
	else if(key == STATUS_ID_FULLSYMBOL)
		statusFullSimbol->SetLabel(value == 0 ? "Half-width Symbol" : "Full-width Symbol");
}


int main(int argc, char **argv)
{
	new BApplication("application/x-vnd.lee-sunpinyin-app");

	TWindow *win = new TWindow();
	if(win->InitCheck() != B_OK)
	{
		BAlert *alert = new BAlert("Sun Pinyin Standalone",
					   "Unable to initalize Sun Pinyin!!!",
					   "Quit", NULL, NULL,
					   B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Lock();
		alert->SetFlags(B_QUIT_ON_WINDOW_CLOSE);
		alert->Unlock();
		alert->Go((BInvoker*)NULL);
	}
	else
	{
		win->Lock();
		win->Show();
		win->Unlock();
	}

	be_app->Run();

	be_app->Lock();
	be_app->Quit();

	return 0;
}

