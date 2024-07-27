#pragma once
#include "f4se/ScaleformLoader.h"
#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/CustomMenu.h"
#include "f4se/PapyrusScaleformAdapter.h"
#include "f4se/PapyrusEvents.h"
#include "f4se/GameInput.h"
#include "f4se/InputMap.h"
#include "MSF_Shared.h"
#include "MSF_Data.h"
#include "MSF_Base.h"

namespace MSF_Scaleform
{
	void ReceiveKeyEvents();
	bool RegisterScaleformCallback(GFxMovieView* view, GFxValue* f4se_root);
	bool RegisterScaleformTest(GFxMovieView * view, GFxValue * f4se_root);
	void RegisterMSFScaleformFuncs(GFxValue* codeObj, GFxMovieRoot* movieRoot);
	bool StartWidgetHideCountdown(UInt32 delayTime);
	bool UpdateWidgetData(TESObjectWEAP::InstanceData* instanceData = nullptr);
	bool UpdateWidgetSettings();
	UInt32 GetInterfaceVersion(); 
	GFxMovieRoot* HandleToggleMenu(ModSelectionMenu* selectMenu);
	bool ToggleSelectionMenu(ModSelectionMenu* selectMenu, ModData* mods);
	bool ToggleGlobalMenu(ModSelectionMenu* selectMenu);
	bool ToggleAmmoMenu(ModSelectionMenu* selectMenu);
	bool ToggleModMenu(ModSelectionMenu* selectMenu, ModData* mods);
}

class MSFWidgetMenu : public GameMenuBase
{
public:
	MSFWidgetMenu() : GameMenuBase()
	{
		enum
		{
			//Confirmed
			kFlag_PauseGame = 0x01,
			kFlag_DoNotDeleteOnClose = 0x02,
			kFlag_ShowCursor = 0x04,
			kFlag_EnableMenuControl = 0x08, // 1, 2
			kFlag_ShaderdWorld = 0x20,
			kFlag_Open = 0x40,//set it after open.
			kFlag_DoNotPreventGameSave = 0x800,
			kFlag_ApplyDropDownFilter = 0x8000, //
			kFlag_BlurBackground = 0x400000,

			//Unconfirmed
			kFlag_Modal = 0x10,
			kFlag_PreventGameLoad = 0x80,
			kFlag_Unk0100 = 0x100,
			kFlag_HideOther = 0x200,
			kFlag_DisableInteractive = 0x4000,
			kFlag_UpdateCursorOnPlatformChange = 0x400,
			kFlag_Unk1000 = 0x1000,
			kFlag_ItemMenu = 0x2000,
			kFlag_Unk10000 = 0x10000,	// mouse cursor
			kFlag_Unk800000 = 0x800000
		};
		flags = kFlag_AllowSaving | kFlag_AlwaysOpen | kFlag_DontHideCursorWhenTopmost;// | kFlag_ApplyDropDownFilter | kFlag_Unk800000;// | 0x40000 | 0x80000 | 0x200000;
		depth = 0x6;
		if (CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(this, this->movie, "MSFwidget", "root1", 0))
		{

			_MESSAGE("MSF widget loaded.");

			CreateBaseShaderTarget(this->filterHolder, this->stage);

			//inherit_colors
			this->filterHolder->SetFilterColor(false);
			(*g_colorUpdateDispatcher)->eventDispatcher.AddEventSink(this->filterHolder);

			if (flags & kFlag_ApplyDropDownFilter)
				this->shaderFXObjects.Push(this->filterHolder);
		}
	}
	virtual void	Invoke(Args * args) final
	{
		switch (args->optionID)
		{
		case 0:
			break;
		default:
			break;
		}
	}

	virtual void	RegisterFunctions() final
	{
		_MESSAGE("RegisterFunctions");
	}

	virtual UInt32	ProcessMessage(UIMessage * msg) final
	{
		GFxMovieRoot * root = movie->movieRoot;
		GFxValue ValueToSet;
		//_DEBUG("message proc!!");
		return this->GameMenuBase::ProcessMessage(msg);
	};

	virtual void	DrawNextFrame(float unk0, void * unk1) final
	{
		return this->GameMenuBase::AdvanceMovie(unk0, unk1);
	};

	static IMenu * CreateMSFMenu()
	{
		return new MSFWidgetMenu();
	}

	static void OpenMenu()
	{
		static BSFixedString menuName("MSFwidget");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
	}

	static void CloseMenu()
	{
		static BSFixedString menuName("MSFwidget");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Close);
	}

	static void RegisterMenu()
	{
		static BSFixedString menuName("MSFwidget");
		if ((*g_ui) != nullptr && !(*g_ui)->IsMenuRegistered(menuName))
		{
			(*g_ui)->Register("MSFwidget", CreateMSFMenu);
		}
		_MESSAGE("MSFwidget %s", (*g_ui)->IsMenuRegistered(menuName) ? "registered" : "not registered");
	}
};

class MSFMenu : public GameMenuBase
{
public:

	MSFMenu() : GameMenuBase()
	{
		flags = kFlag_AllowSaving | kFlag_AlwaysOpen;// | kFlag_ApplyDropDownFilter | kFlag_Unk800000 | kFlag_DisableInteractive;// | 0x40000 | 0x80000 | 0x200000;
		depth = 0x6;
		if (CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(this, this->movie, "MSFMenu", "root1", 0))
		{

			_MESSAGE("MSF Menu loaded.");

			CreateBaseShaderTarget(this->filterHolder, this->stage);

			//inherit_colors
			this->filterHolder->SetFilterColor(false);
			(*g_colorUpdateDispatcher)->eventDispatcher.AddEventSink(this->filterHolder);

			if (flags & kFlag_CustomRendering)
				this->shaderFXObjects.Push(this->filterHolder);
		}
	}

	virtual void	Invoke(Args * args) final
	{
		switch (args->optionID)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		default:
			break;
		}
	}

	virtual void	RegisterFunctions() final
	{
		_MESSAGE("RegisterFunctions");
	}

	virtual UInt32	ProcessMessage(UIMessage * msg) final
	{
		GFxMovieRoot * root = movie->movieRoot;
		GFxValue ValueToSet;
		//_DEBUG("message proc!!");
		/*
		switch (msg->type)
		{
		case kMessage_UpdateModSettings:
		{
			//_DEBUG("UpdateModSettings");
			if (isHMO)
			{
				GFxValue arr;
				root->GetVariable(&arr, "root.Menu_mc.trans_food_HMO");
				root->SetVariable("root.Menu_mc.trans_food", &arr);

				ValueToSet.SetInt(isHMO ? 1 : 0);
				root->SetVariable("root.Menu_mc.isHMO", &ValueToSet);
			}

			ValueToSet.SetInt(iColored);
			root->SetVariable("root.Menu_mc.iColored", &ValueToSet);

			ValueToSet.SetInt(iFoodBarVisible);
			root->SetVariable("root.Menu_mc.iFoodBarVisible", &ValueToSet);

			root->Invoke("root.Menu_mc.UpdateModSettings", nullptr, nullptr, 0);
			break;
		}
		case kMessage_UpdateValues:
		{
			//_DEBUG("UpdateValues");

			ValueToSet.SetInt(iFoodPool);
			root->SetVariable("root.Menu_mc.iFoodPool", &ValueToSet);

			if (isHMO)
			{
				ValueToSet.SetInt(iSleepPool);
			}
			else
			{
				ValueToSet.SetInt(-(SInt32)(*g_player)->actorValueOwner.GetValue(HC_SleepEffect));
			}
			root->SetVariable("root.Menu_mc.iSleepPool", &ValueToSet);

			ValueToSet.SetInt((SInt32)(*g_player)->actorValueOwner.GetValue(HC_HungerEffect));
			root->SetVariable("root.Menu_mc.iFoodStatus", &ValueToSet);

			ValueToSet.SetInt((SInt32)(*g_player)->actorValueOwner.GetValue(HC_ThirstEffect));
			root->SetVariable("root.Menu_mc.iDrinkStatus", &ValueToSet);

			ValueToSet.SetInt((SInt32)(*g_player)->actorValueOwner.GetValue(HC_SleepEffect));
			root->SetVariable("root.Menu_mc.iSleepStatus", &ValueToSet);

			root->Invoke("root.Menu_mc.UpdateValues", nullptr, nullptr, 0);
			break;
		}
		case kMessage_UpdateAmounts:
		{
			//_DEBUG("UpdateAmounts");

			ValueToSet.SetInt(iSleepPoolIncapacitatedAmount);
			root->SetVariable("root.Menu_mc.iSleepPoolIncapacitatedAmount", &ValueToSet);

			root->Invoke("root.Menu_mc.UpdateAmounts", nullptr, nullptr, 0);
		}
		default:
			break;
		}
		*/
		return this->GameMenuBase::ProcessMessage(msg);
	};

	virtual void	DrawNextFrame(float unk0, void * unk1) final
	{
		//_DMESSAGE("DrawNextFrame");
		return this->GameMenuBase::AdvanceMovie(unk0, unk1);
	};

	static IMenu * CreateMSFMenu()
	{
		return new MSFMenu();
	}

	static void OpenMenu()
	{
		static BSFixedString menuName("MSFMenu");
		//_DEBUG("opening menu");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Open);
	}

	static void CloseMenu()
	{
		static BSFixedString menuName("MSFMenu");
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(menuName, kMessage_Close);
	}

	static void RegisterMenu()
	{
		static BSFixedString menuName("MSFMenu");
		if ((*g_ui) != nullptr && !(*g_ui)->IsMenuRegistered(menuName))
		{
			(*g_ui)->Register("MSFMenu", CreateMSFMenu);
		}
		_MESSAGE("MSFMenu %s", (*g_ui)->IsMenuRegistered(menuName) ? "registered" : "not registered");
	}
};


		//case VMValue::kType_Struct:
		//{
		//	VMStructTypeInfo * structType = static_cast<VMStructTypeInfo*>(src->GetComplexType());
		//	VMValue::StructData * structData = src->data.strct;
		//	if (structType && structData) {
		//		root->CreateObject(dest);
		//		GFxValue gStructObject;
		//		root->CreateObject(&gStructObject);
		//		dest->SetMember("__struct__", &gStructObject);

		//		GFxValue gStructName;
		//		root->CreateString(&gStructName, structType->m_typeName.c_str());
		//		gStructObject.SetMember("__type__", &gStructName);

		//		GFxValue gStructPairs;
		//		root->CreateObject(&gStructPairs);
		//		gStructObject.SetMember("__data__", &gStructPairs);

		//		structType->m_members.ForEach([&gStructPairs, &structData, &root](VMStructTypeInfo::MemberItem * item)
		//		{
		//			GFxValue value;
		//			ConvertPapyrusValue(&value, &structData->GetStruct()[item->index], root);
		//			gStructPairs.SetMember(item->name.c_str(), &value);
		//			return true;
		//		});
		//		return true;
		//	}
		//}
		//break;