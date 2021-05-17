#ifndef __BE_STUFFS_H__
#define __BE_STUFFS_H__

#include <be/Be.h>

#ifdef __LITE_BEAPI__
#include <etkxx/etk/add-ons/input/InputServerMethod.h>
#define BInputServerMethod			EInputServerMethod
#define B_INPUT_METHOD_EVENT			E_INPUT_METHOD_EVENT
#define B_INPUT_METHOD_STARTED			E_INPUT_METHOD_STARTED
#define B_INPUT_METHOD_STOPPED			E_INPUT_METHOD_STOPPED
#define B_INPUT_METHOD_CHANGED			E_INPUT_METHOD_CHANGED
#define B_INPUT_METHOD_LOCATION_REQUEST		E_INPUT_METHOD_LOCATION_REQUEST
#define IME_OPCODE_DESC				"etk:opcode"
#define IME_REPLY_DESC				"etk:reply_to"
#define IME_STRING_DESC				"etk:string"
#define IME_SELECTION_DESC			"etk:selection"
#define IME_CLAUSE_START_DESC			"etk:clause_start"
#define IME_CLAUSE_END_DESC			"etk:clause_end"
#define IME_LOCATION_REPLY_DESC			"etk:location_reply"
#define IME_HEIGHT_REPLY_DESC			"etk:height_reply"
#define IME_CONFIRMED_DESC			"etk:confirmed"
#else // !__LITE_BEAPI__
#define IME_OPCODE_DESC				"be:opcode"
#define IME_REPLY_DESC				"be:reply_to"
#define IME_STRING_DESC				"be:string"
#define IME_SELECTION_DESC			"be:selection"
#define IME_CLAUSE_START_DESC			"be:clause_start"
#define IME_CLAUSE_END_DESC			"be:clause_end"
#define IME_LOCATION_REPLY_DESC			"be:location_reply"
#define IME_HEIGHT_REPLY_DESC			"be:height_reply"
#define IME_CONFIRMED_DESC			"be:confirmed"
#endif // __LITE_BEAPI__

#ifndef _LOCAL
#define _LOCAL
#endif

#endif /* __BE_STUFFS_H__ */

