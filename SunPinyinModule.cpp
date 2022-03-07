#include "SunPinyinModule.h"
#include "SunPinyinHandler.h"
#include "SunPinyinStatusWindow.h"


// NOTE:
// 	When the graphics engine of input server or application using the filters
// itself in it's input method implementation, such as preference dialog, &etc,
// using GUI inside Filter() might cause dead-lock issues.
// 	If the input server uses no filter like that, or the GUI operation is
// the business of other server, such as BeOS/HaikuOS, it won't be a matter.
// 	Anyway, keeping GUI out of locking will do no harm to anything.
#define GUI_OUT_OF_LOCKING


static unsigned char logo_icon[] = { // 16x16
0x21,0x21,0x21,0x21,0x21,0x21,0x21,0xff,0xff,0x21,0xff,0x21,0xff,0x21,0x21,0x21,
0x21,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x21,0xff,0x21,0xff,0x21,0xff,0x21,
0x21,0x21,0x21,0x21,0x21,0x21,0x21,0xff,0xff,0x21,0xff,0x21,0xff,0x21,0xff,0x21,
0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x21,0xff,0x00,0x00,0x00,
0x00,0x2a,0x2a,0x2a,0x2a,0x2a,0x2a,0x00,0xfa,0xfa,0xfa,0x00,0x00,0xfa,0xfa,0xfa,
0xff,0x00,0x2a,0x2a,0x00,0x00,0x2a,0x2a,0x00,0xfa,0xfa,0x00,0x00,0xfa,0xfa,0x00,
0x21,0x00,0x2a,0x2a,0x00,0x00,0x2a,0x2a,0x00,0x00,0xfa,0xfa,0x00,0xfa,0x00,0x21,
0xff,0x00,0x2a,0x2a,0x00,0x00,0x2a,0x2a,0x00,0x00,0xfa,0xfa,0xfa,0x00,0xff,0xff,
0xff,0x00,0x2a,0x2a,0x2a,0x2a,0x2a,0x00,0xff,0xff,0x00,0xfa,0xfa,0x00,0xff,0xff,
0x21,0x00,0x2a,0x2a,0x00,0x00,0x00,0xff,0xff,0x21,0x00,0xfa,0xfa,0x00,0x21,0x21,
0x21,0x00,0x2a,0x2a,0x00,0x00,0x21,0xff,0xff,0x21,0x00,0xfa,0xfa,0x00,0xff,0xff,
0x00,0x2a,0x2a,0x2a,0x2a,0x2a,0x00,0xff,0xff,0x00,0xfa,0xfa,0xfa,0xfa,0x00,0x21,
0x21,0x00,0x00,0x00,0x00,0x00,0x21,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,
0x21,0xff,0x21,0xff,0x21,0xff,0x21,0xff,0xff,0x21,0x21,0x21,0x21,0x21,0x21,0x21,
0x21,0xff,0x21,0xff,0x21,0xff,0x21,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x21,
0x21,0x21,0x21,0xff,0x21,0xff,0x21,0xff,0xff,0x21,0x21,0x21,0x21,0x21,0x21,0x21,
};


static unsigned char english_icon[] = { // 16x16
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0x1f,0x0b,0xff,0xff,0xff,0x1e,0x0b,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0x1e,0x00,0xff,0xff,0xff,0x1e,0x01,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0x01,0x01,0x01,0x00,0x01,0x01,0x01,0x01,0x00,0x01,0x01,0x03,0xff,0xff,
0xff,0xff,0xff,0xff,0x1e,0x00,0xff,0x16,0x18,0x1e,0x01,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0d,0x11,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0x0a,0x02,0x02,0x02,0x00,0x01,0x02,0x02,0x02,0x08,0xff,0xff,0xff,
0xff,0xff,0xff,0x09,0x17,0xff,0xff,0x0d,0x11,0xff,0xff,0x18,0xd6,0xff,0xff,0xff,
0xff,0xff,0xff,0x09,0x17,0xff,0xff,0xaf,0x11,0xff,0xff,0x18,0xd6,0xff,0xff,0xff,
0xff,0xff,0xff,0x09,0x17,0xff,0xff,0x0c,0x12,0xff,0xff,0x18,0xd6,0xff,0xff,0xff,
0xff,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1a,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0x16,0x03,0xd6,0x14,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0x14,0x02,0x17,0x1b,0x03,0x12,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0x1b,0x12,0x05,0x08,0x1a,0xff,0xff,0x1c,0x07,0x05,0x12,0x61,0x1f,0xff,
0xff,0x1b,0x06,0x0e,0x19,0xff,0xff,0xff,0xff,0xff,0xff,0x18,0xaf,0x05,0x1a,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
};


SunPinyinModule::SunPinyinModule()
	: BInputServerMethod("Sun 拼音", logo_icon),
	  fMenu(NULL), fCurrentMessageHandlerMsgr(0),
	  fIMView(NULL), fIMHandler(NULL),
	  fShiftKeyToSwitch(true), fPageKeysGroupFlags(0x3),
	  fEnabled(true), fActivated(false)
{
	bzero(fMessageHandlerMsgrs, COUNT_OF_MESSAGE_HANDLER_MESSENGERS * sizeof(BMessenger*));

#ifndef INPUT_SERVER_MORE_SUPPORT
	BWindow *win = new SunPinyinStatusWindow();
	win->Lock();
#ifdef __LITE_BEAPI__
	// for reducing thread
	etk_app->Lock();
	win->ProxyBy(etk_app);
	etk_app->Unlock();
#else
	win->Run();
#endif
	fStatusWinMessenger = BMessenger(win);
	if(fStatusWinMessenger.IsValid() == false)
	{
		fErrorInfo << "Failed to initialize fStatusWinMessenger!\n";
		win->Quit();
		return;
	}
	win->Unlock();
#endif // !INPUT_SERVER_MORE_SUPPORT

	// TODO: load config

	if(_InitSunPinyin() != B_OK)
	{
		fErrorInfo << "_InitSunPinyin() failed!\n";
		return;
	}

	fMenu = _GenerateMenu();

	if(fEnabled == false)
		SetIcon(english_icon);

	BLooper *looper = new BLooper();
	looper->Lock();
	looper->Run();
	for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS + 1; k++)
	{
		SunPinyinMessageHandler *handler = new SunPinyinMessageHandler(this);
		if(handler == NULL) goto ErrorExit;

#ifndef INPUT_SERVER_MORE_SUPPORT
		handler->SetStatusWindowMessenger(fStatusWinMessenger);
#endif

		looper->AddHandler(handler);
		if(k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS)
		{
			fMessageHandlerMsgrs[k] = new BMessenger(handler);
			if(fMessageHandlerMsgrs[k] == NULL || fMessageHandlerMsgrs[k]->IsValid() == false)
			{
				fErrorInfo << "Failed to initialize fMessageHandlerMsgrs[" << k << "]!\n";
				looper->RemoveHandler(handler);
				delete handler;
				goto ErrorExit;
			}
		}
		else
		{
			fMenuHandlerMsgr = BMessenger(handler);
			if(fMenuHandlerMsgr.IsValid() == false)
			{
				fErrorInfo << "Failed to initialize fMenuHandlerMsg!\n";
				looper->RemoveHandler(handler);
				delete handler;
				goto ErrorExit;
			}
		}
	}

	looper->Unlock();
	return;

ErrorExit:
	// NOTE: it's unnecessary to remove handlers created.
	looper->Quit();
}


SunPinyinModule::~SunPinyinModule()
{
	if(fMenuHandlerMsgr.LockTarget())
	{
		BLooper *looper = NULL;
		fMenuHandlerMsgr.Target(&looper);
		looper->Quit();
	}

#ifndef INPUT_SERVER_MORE_SUPPORT
	if(fStatusWinMessenger.LockTarget())
	{
		BLooper *looper = NULL;
		fStatusWinMessenger.Target(&looper);
		looper->Quit();
	}
#endif

	for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS; k++)
	{
		if(fMessageHandlerMsgrs[k] != NULL)
			delete fMessageHandlerMsgrs[k];
	}

	_DeInitSunPinyin();
	EmptyMessageOutList();
	delete fMenu;
}


bool
SunPinyinModule::Lock()
{
	return fLocker.Lock();
}


void
SunPinyinModule::Unlock()
{
	fLocker.Unlock();
}


bool
SunPinyinModule::IsLocked()
{
#ifndef __LITE_BEAPI__
	return fLocker.IsLocked();
#else
	return fLocker.IsLockedByCurrentThread();
#endif
}


status_t
#ifdef __LITE_BEAPI__
SunPinyinModule::InitCheck() const
#else
SunPinyinModule::InitCheck()
#endif
{
	status_t retVal = B_OK;

	if(fMenuHandlerMsgr.IsValid() == false)
	{
		retVal = B_ERROR;
	}
	else for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS; k++)
	{
		if(fMessageHandlerMsgrs[k] == NULL ||
		   fMessageHandlerMsgrs[k]->IsValid() == false)
		{
			retVal = B_ERROR;
			break;
		}
	}

	if(retVal != B_OK)
	{
		BString strInfo;
		if(fErrorInfo.Length() > 0)
		{
			strInfo << "Error information:\n";
			strInfo.Append(fErrorInfo.String());
			strInfo << "\nOther information:\n";
		}

		strInfo << "Unable to initialize SunPinyin input method addon!!!";
		(new BAlert("SunPinyin",
			    strInfo.String(),
			    "OK"))->Go((BInvoker*)NULL);
	}

	return retVal;
}


status_t
SunPinyinModule::MethodActivated(bool state)
{
#ifndef __LITE_BEAPI__
	bool send_stopped_msg = false;
#endif

	// NOTE:
	//	SunPinyinMessageHandler will probably access SunPinyinModule in other thread,
	// we should lock itself before doing anything.
	Lock();

	// TODO: menu, icon, &etc.
	SetMenu((state ? fMenu : NULL), fMenuHandlerMsgr);
	if(state == false)
	{
#ifndef __LITE_BEAPI__
		// NOTE: BeOS won't send stopped message to view anyway.
		if(fIMView->getIC()->isEmpty() == false)
			send_stopped_msg = true;
#endif

		ResetSunPinyin();
	}

	fActivated = state;

	Unlock();

#ifndef __LITE_BEAPI__
	if(send_stopped_msg)
	{
		BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
		msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STOPPED);
		if(EnqueueMessage(msg) != B_OK) delete msg;
	}
#endif

	return B_OK;
}


filter_result
SunPinyinModule::Filter(BMessage *message, BList *outList)
{
	filter_result retVal = B_DISPATCH_MESSAGE;

	// NOTE:
	//	SunPinyinMessageHandler will probably access SunPinyinModule in other thread,
	// we should lock itself before doing anything.
	Lock();

	// TODO
	if(fShiftKeyToSwitch && message->what == B_MODIFIERS_CHANGED)
	{
		int32 modifiers, old_modifiers;

		if(message->FindInt32("modifiers", &modifiers) == B_OK &&
		   message->FindInt32(OLD_MODIFIERS_DESC, &old_modifiers) == B_OK)
		{
			int32 mask = (B_CAPS_LOCK | B_SCROLL_LOCK | B_NUM_LOCK |
				      B_LEFT_SHIFT_KEY | B_RIGHT_SHIFT_KEY |
				      B_LEFT_CONTROL_KEY | B_RIGHT_CONTROL_KEY |
				      B_LEFT_OPTION_KEY | B_RIGHT_OPTION_KEY);
			modifiers &= ~mask;
			old_modifiers &= ~mask;

			if(modifiers == B_SHIFT_KEY && old_modifiers == 0)
			{
				fShiftKeyFollowingOthers = false;
			}
			else if(modifiers == 0 && old_modifiers == B_SHIFT_KEY &&
				fShiftKeyFollowingOthers == false)
			{
				if(fEnabled)
				{
					ResetSunPinyin();

					BMessage *msg = new BMessage(B_INPUT_METHOD_EVENT);
					msg->AddInt32(IME_OPCODE_DESC, B_INPUT_METHOD_STOPPED);

					// we won't skip the current message
					if(EnqueueMessage(msg) != B_OK) delete msg;
				}
				fEnabled = !fEnabled;
				SetIcon(fEnabled ? logo_icon : english_icon);
			}
			else if(modifiers != 0)
			{
				fShiftKeyFollowingOthers = true;
			}
		}
	}
	else if(message->what == B_KEY_DOWN || message->what == B_KEY_UP) do
	{
		fShiftKeyFollowingOthers = true;
		if(fEnabled == false) break;

		int32 key, modifiers;
		int8 byte;
		const char *bytes = NULL;

		if(message->FindInt32("key", &key) != B_OK) break;
		if(message->FindInt32("modifiers", &modifiers) != B_OK) break;

		// to be sure it's a single key
		message->FindString("bytes", &bytes);
		if(message->FindInt8("byte", 1, &byte) == B_OK) break;
		if(message->FindInt8("byte", &byte) != B_OK) break;
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
			default: keyCode = (byte < 0x20 || byte > 0x7e) ? 0x10000 : (unsigned int)byte;
		}

		if(keyCode > 0xffff) break; // unknown key, let it be
		if(message->what == B_KEY_UP) // filtered key, skip it if we are handling something
		{
			if(fIMView->getIC()->isEmpty() == false)
				retVal = B_SKIP_MESSAGE;
			break;
		}

		if(modifiers & B_SHIFT_KEY) keyState |= IM_SHIFT_MASK;
		if(modifiers & B_CONTROL_KEY) keyState |= IM_CTRL_MASK;
		if(modifiers & B_COMMAND_KEY) keyState |= IM_ALT_MASK;
		if(modifiers & B_OPTION_KEY) keyState |= IM_SUPER_MASK;

		EmptyMessageOutList();

		// fIMView->onKeyEvent() will call the proper handling of SunPinyinHandler
		bool used = false;
		CKeyEvent ckey(keyCode, byte, keyState);
#ifdef INPUT_SERVER_MORE_SUPPORT
		used = fIMHandler->checkKeyEvent(ckey);
#endif
		if(used == false)
			used = fIMView->onKeyEvent(ckey);

		if(used && fMessageOutList.CountItems() == 0)
		{
			retVal = B_SKIP_MESSAGE;
		}
		else if(fMessageOutList.CountItems() > 0)
		{
			outList->AddList(&fMessageOutList);
			fMessageOutList.MakeEmpty();
		}
	} while(false);

	Unlock();
	return retVal;
}


BMenu*
SunPinyinModule::_GenerateMenu()
{
	bool shift_key_using = fShiftKeyToSwitch;
	uint8 page_keys_flags = fPageKeysGroupFlags;

#ifdef GUI_OUT_OF_LOCKING
	bool locked = IsLocked();
	if(locked) Unlock();
#endif

	BMenu *menu = new BMenu(NULL, B_ITEMS_IN_COLUMN);

	BMenuItem *item = new BMenuItem("启用 SHIFT 键切换中英文", new BMessage(MSG_MENU_SWITCH_EN_CN_BY_SHIFT_KEY));
	if(shift_key_using) item->SetMarked(true);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	BMenu *submenu = new BMenu("附加翻页键", B_ITEMS_IN_COLUMN);
	submenu->SetRadioMode(false);
	menu->AddItem(submenu);

	item = new BMenuItem("启用减号与等号", new BMessage(MSG_MENU_USE_PAGE_KEYS_GROUP1));
	if(page_keys_flags & 0x01) item->SetMarked(true);
	submenu->AddItem(item);

	item = new BMenuItem("启用逗号与句号", new BMessage(MSG_MENU_USE_PAGE_KEYS_GROUP2));
	if(page_keys_flags & 0x02) item->SetMarked(true);
	submenu->AddItem(item);

	item = new BMenuItem("启用中括号", new BMessage(MSG_MENU_USE_PAGE_KEYS_GROUP3));
	if(page_keys_flags & 0x04) item->SetMarked(true);
	submenu->AddItem(item);

#ifdef GUI_OUT_OF_LOCKING
	if(locked) Lock();
#endif

	return menu;
}


void
SunPinyinModule::_RegenMenu()
{
	BMenu *menu = _GenerateMenu();
	BMenu *old_menu = fMenu;
	fMenu = menu;

#ifdef GUI_OUT_OF_LOCKING
	bool locked = IsLocked();
	if(locked) Unlock();
#endif

	delete old_menu;

#ifdef GUI_OUT_OF_LOCKING
	if(locked) Lock();
#endif

	if(fActivated)
	{
		// NOTE:
		// 	SetMenu() convert menu to message by using BMenu::Archive(),
		// so it's unnecessary to keep it out of locking.
		SetMenu(fMenu, fMenuHandlerMsgr);
	}
}


void
SunPinyinModule::EmptyMessageOutList()
{
	for(int32 k = 0; k < fMessageOutList.CountItems(); k++)
	{
		BMessage *msg = (BMessage*)fMessageOutList.ItemAt(k);
		if(msg != NULL) delete msg;
	}
	fMessageOutList.MakeEmpty();
}


void
SunPinyinModule::EnqueueMessageOutList()
{
	for(int32 k = 0; k < fMessageOutList.CountItems(); k++)
	{
		BMessage *msg = (BMessage*)fMessageOutList.ItemAt(k);
		if(msg != NULL)
		{
			if(EnqueueMessage(msg) != B_OK) delete msg;
		}
	}
	fMessageOutList.MakeEmpty();
}


void
SunPinyinModule::AddMessageToOutList(BMessage *msg)
{
	if(fMessageOutList.AddItem(msg) == false && msg != NULL) delete msg;
}


const BMessenger&
SunPinyinModule::GetHandlerMessenger()
{
	fCurrentMessageHandlerMsgr++;
	if(fCurrentMessageHandlerMsgr >= COUNT_OF_MESSAGE_HANDLER_MESSENGERS)
		fCurrentMessageHandlerMsgr = 0;

	return(*(fMessageHandlerMsgrs[fCurrentMessageHandlerMsgr]));
}


const BMessenger&
SunPinyinModule::CurrentHandlerMessenger() const
{
	return(*(fMessageHandlerMsgrs[fCurrentMessageHandlerMsgr]));
}


void
SunPinyinModule::ResetSunPinyin()
{
	fIMView->clearIC();
	fIMHandler->Reset();
}


void
SunPinyinModule::SwitchShiftKeyUsing()
{
	fShiftKeyToSwitch = !fShiftKeyToSwitch;

	// TODO: save config

	_RegenMenu();
}


void
SunPinyinModule::SwitchPageKeysGroup(uint8 bit)
{
	char keys[2];
	switch(bit)
	{
		case 0:
			keys[0] = '-';
			keys[1] = '=';
			break;

		case 1:
			keys[0] = ',';
			keys[1] = '.';
			break;

		case 2:
			keys[0] = '[';
			keys[1] = ']';
			break;

		default:
			return;
	}

	CHotkeyProfile *profile = fIMView->getHotkeyProfile();
	if(fPageKeysGroupFlags & (0x1 << bit))
	{
		fPageKeysGroupFlags &= ~(0x1 << bit);
		profile->removePageUpKey(CKeyEvent((unsigned int)keys[0], (unsigned int)keys[0], 0));
		profile->removePageDownKey(CKeyEvent((unsigned int)keys[1], (unsigned int)keys[1], 0));
	}
	else
	{
		fPageKeysGroupFlags |= (0x1 << bit);
		profile->addPageUpKey(CKeyEvent((unsigned int)keys[0], (unsigned int)keys[0], 0));
		profile->addPageDownKey(CKeyEvent((unsigned int)keys[1], (unsigned int)keys[1], 0));
	}

	// TODO: save config

	_RegenMenu();
}


status_t
SunPinyinModule::_InitSunPinyin()
{
	CSunpinyinSessionFactory &factory = CSunpinyinSessionFactory::getFactory();
	factory.setLanguage(CSunpinyinSessionFactory::SIMPLIFIED_CHINESE);
	factory.setInputStyle(CSunpinyinSessionFactory::CLASSIC_STYLE);
	factory.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
#ifndef INPUT_SERVER_MORE_SUPPORT
	factory.setCandiWindowSize(10);
#else
	factory.setCandiWindowSize(STATUS_MAX_ROWS * STATUS_MAX_COLUMNS + 1);
#endif

	if((fIMView = factory.createSession()) == NULL ||
	   fIMView->getIC() == NULL)
	{
		fErrorInfo << "Failed to initialize fIMView!\n";
		return B_ERROR;
	}

#ifdef INPUT_SERVER_MORE_SUPPORT
	if(!is_kind_of(fIMView, CIMIClassicView))
	{
		fErrorInfo << "Failed to initialize fIMView!\n";
		return B_ERROR;
	}
#endif

	if((fIMHandler = new SunPinyinHandler(this)) == NULL)
	{
		fErrorInfo << "Failed to allocate memory for fIMHandler!\n";
		return B_NO_MEMORY;
	}

#ifndef INPUT_SERVER_MORE_SUPPORT
	fIMHandler->SetStatusWindowMessenger(fStatusWinMessenger);
#endif

	fIMView->getIC()->setCharsetLevel(1); // GBK

#if 0
	fIMView->getIC()->setUserDict(NULL); // no user dict
	fIMView->getIC()->setHistoryMemory(NULL); // no history
#endif

	fIMView->attachWinHandler(fIMHandler);

	CHotkeyProfile *profile = fIMView->getHotkeyProfile();
	if(fPageKeysGroupFlags & 0x1)
	{
		profile->addPageUpKey(CKeyEvent((unsigned int)'-', (unsigned int)'-', 0));
		profile->addPageDownKey(CKeyEvent((unsigned int)'=', (unsigned int)'=', 0));
	}
	if(fPageKeysGroupFlags & 0x2)
	{
		profile->addPageUpKey(CKeyEvent((unsigned int)',', (unsigned int)',', 0));
		profile->addPageDownKey(CKeyEvent((unsigned int)'.', (unsigned int)'.', 0));
	}
	if(fPageKeysGroupFlags & 0x4)
	{
		profile->addPageUpKey(CKeyEvent((unsigned int)'[', (unsigned int)'[', 0));
		profile->addPageDownKey(CKeyEvent((unsigned int)']', (unsigned int)']', 0));
	}

	// TODO

#if 0
	// NOTE: we don't use STATUS_ID_CN or SwitchModeKey of libsunpinyin
	fIMView->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, 1);
#endif
	fIMView->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, 1);
	fIMView->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, 1);

	return B_OK;
}


void
SunPinyinModule::_DeInitSunPinyin()
{
	if(fIMView)
		CSunpinyinSessionFactory::getFactory().destroySession(fIMView);
	if(fIMHandler)
		delete fIMHandler;
}


CIMIView*
SunPinyinModule::IMView()
{
	return fIMView;
}


SunPinyinHandler*
SunPinyinModule::IMHandler()
{
	return fIMHandler;
}


extern "C" _EXPORT BInputServerMethod* instantiate_input_method()
{
	return new SunPinyinModule();
}

