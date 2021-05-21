#include "SunPinyinModule.h"
#include "SunPinyinHandler.h"

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
	  fShiftKeyToSwitch(true),
	  fEnabled(true)
{
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
		win->Quit();
		return;
	}
	win->Unlock();

	if(_InitSunPinyin() != B_OK) return;

	// TODO: MenuMessenger, load preference, &etc.
	fMenu = _GenerateMenu();

	if(fEnabled == false)
		SetIcon(english_icon);

	// TODO: Test on BeOS/HaikuOS, if the locking blocks thread, maybe we should create looper for it.
	// NOTE: It's NOT RECOMMENDED to use "SunPinyinStatusWindow" as looper!
	bzero(fMessageHandlerMsgrs, COUNT_OF_MESSAGE_HANDLER_MESSENGERS * sizeof(BMessenger*));
	be_app->Lock();
	for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS; k++)
	{
		BHandler *handler = new SunPinyinMessageHandler(this, fStatusWinMessenger);
		if(handler == NULL) break;

		be_app->AddHandler(handler);
		fMessageHandlerMsgrs[k] = new BMessenger(handler);
		if(fMessageHandlerMsgrs[k] == NULL || fMessageHandlerMsgrs[k]->IsValid() == false)
		{
			be_app->RemoveHandler(handler);
			delete handler;
			break;
		}
	}
	be_app->Unlock();
}


SunPinyinModule::~SunPinyinModule()
{
	if(!(fMessageHandlerMsgrs[0] == NULL || fMessageHandlerMsgrs[0]->LockTarget() == false))
	{
		BLooper *looper = NULL;
		fMessageHandlerMsgrs[0]->Target(&looper);

		for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS; k++)
		{
			if(fMessageHandlerMsgrs[k] == NULL) break;
			BHandler *handler = fMessageHandlerMsgrs[k]->Target(NULL);
			if(handler)
			{
				looper->RemoveHandler(handler);
				delete handler;
			}
			delete fMessageHandlerMsgrs[k];
		}

		looper->Unlock();
	}

	if(fStatusWinMessenger.LockTarget())
	{
		BLooper *looper = NULL;
		fStatusWinMessenger.Target(&looper);
		looper->Quit();
	}

	_DeInitSunPinyin();
	EmptyMessageOutList();
	if(fMenu) delete fMenu;
}


status_t
SunPinyinModule::InitCheck() const
{
	for(int k = 0; k < COUNT_OF_MESSAGE_HANDLER_MESSENGERS; k++)
	{
		if(fMessageHandlerMsgrs[k] == NULL ||
		   fMessageHandlerMsgrs[k]->IsValid() == false) return B_ERROR;
	}
	return B_OK;
}


status_t
SunPinyinModule::MethodActivated(bool state)
{
	// NOTE:
	//	SunPinyinMessageHandler will probably access SunPinyinModule in other thread,
	// we should lock it before doing anything.
	if(fMessageHandlerMsgrs[0] == NULL ||
	   fMessageHandlerMsgrs[0]->LockTarget() == false) return B_ERROR;
	BLooper *looper = NULL;
	fMessageHandlerMsgrs[0]->Target(&looper);

	// TODO: menu, icon, &etc.
	SetMenu((state ? fMenu : NULL), fMenuHandlerMsgr);
	if(state == false)
		ResetSunPinyin();

	looper->Unlock();

#ifndef __LITE_BEAPI__
	// NOTE: BeOS won't send stopped message to view anyway.
	if(state == false)
	{
		// Send stopped message whatever it started
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

			// NOTE:
			//	SunPinyinMessageHandler will probably access SunPinyinModule in other thread,
			// we should lock it before doing anything.
			if(!(fMessageHandlerMsgrs[0] == NULL ||
			     fMessageHandlerMsgrs[0]->LockTarget() == false))
			{
				BLooper *looper = NULL;
				fMessageHandlerMsgrs[0]->Target(&looper);

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
						outList->AddItem(msg);
					}
					fEnabled = !fEnabled;
					SetIcon(fEnabled ? logo_icon : english_icon);
				}
				else if(modifiers != 0)
				{
					fShiftKeyFollowingOthers = true;
				}

				looper->Unlock();
			}
		}
	}
	else if((message->what == B_KEY_DOWN || message->what == B_KEY_UP) && fEnabled) do
	{
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
		if(message->what == B_KEY_UP) // filtered key, skip it
		{
			retVal = B_SKIP_MESSAGE;
			break;
		}

		if(modifiers & B_SHIFT_KEY) keyState |= IM_SHIFT_MASK;
		if(modifiers & B_CONTROL_KEY) keyState |= IM_CTRL_MASK;
		if(modifiers & B_COMMAND_KEY) keyState |= IM_ALT_MASK;
		if(modifiers & B_MENU_KEY) keyState |= IM_SUPER_MASK;

		// NOTE:
		//	SunPinyinMessageHandler will probably access SunPinyinModule in other thread,
		// we should lock it before doing anything.
		if(fMessageHandlerMsgrs[0] == NULL ||
		   fMessageHandlerMsgrs[0]->LockTarget() == false) break;
		BLooper *looper = NULL;
		fMessageHandlerMsgrs[0]->Target(&looper);

		fShiftKeyFollowingOthers = true;

		EmptyMessageOutList();

		// fIMView->onKeyEvent() will call the proper handling of SunPinyinHandler
		fIMView->onKeyEvent(CKeyEvent(keyCode, byte, keyState));

		if(fMessageOutList.CountItems() == 1 && fMessageOutList.ItemAt(0) == NULL)
		{
			retVal = B_SKIP_MESSAGE;
		}
		else if(fMessageOutList.CountItems() > 0)
		{
			outList->AddList(&fMessageOutList);
			fMessageOutList.MakeEmpty();
		}

		looper->Unlock();
	} while(false);

	return retVal;
}


BMenu*
SunPinyinModule::_GenerateMenu() const
{
	// TODO
	return NULL;
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
SunPinyinModule::AddMessageToOutList(BMessage *msg)
{
	if(fMessageOutList.AddItem((void*)msg) == false && msg != NULL) delete msg;
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
	if(fIMView) fIMView->clearIC();
	if(fIMHandler) fIMHandler->Reset();
}


status_t
SunPinyinModule::_InitSunPinyin()
{
	// TODO: preference
	CSunpinyinSessionFactory::getFactory().setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);

	if((fIMView = CSunpinyinSessionFactory::getFactory().createSession()) == NULL) return B_ERROR;
	if((fIMHandler = new SunPinyinHandler(this, fStatusWinMessenger)) == NULL) return B_NO_MEMORY;

	fIMView->getIC()->setCharsetLevel(1); // GBK
	fIMView->attachWinHandler(fIMHandler);

	// TODO: preference
	fIMView->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, 1);
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


extern "C" _EXPORT BInputServerMethod* instantiate_input_method()
{
	return new SunPinyinModule();
}

