// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "CXUI.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeCXUI() {}

// ********** Begin Cross Module References ********************************************************
CX_API UClass* Z_Construct_UClass_UChatUI_NoRegister();
CX_API UClass* Z_Construct_UClass_UCXUI();
CX_API UClass* Z_Construct_UClass_UCXUI_NoRegister();
UMG_API UClass* Z_Construct_UClass_UUserWidget();
UPackage* Z_Construct_UPackage__Script_CX();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UCXUI Function GetGameStateInfoText **************************************
struct Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics
{
	struct CXUI_eventGetGameStateInfoText_Parms
	{
		FText ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Game State" },
		{ "ModuleRelativePath", "CXUI.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FTextPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FTextPropertyParams Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(CXUI_eventGetGameStateInfoText_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UCXUI, nullptr, "GetGameStateInfoText", Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::PropPointers), sizeof(Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::CXUI_eventGetGameStateInfoText_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x54020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::Function_MetaDataParams), Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::CXUI_eventGetGameStateInfoText_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UCXUI_GetGameStateInfoText()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UCXUI_GetGameStateInfoText_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UCXUI::execGetGameStateInfoText)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FText*)Z_Param__Result=P_THIS->GetGameStateInfoText();
	P_NATIVE_END;
}
// ********** End Class UCXUI Function GetGameStateInfoText ****************************************

// ********** Begin Class UCXUI Function GetPlayerStateInfoText ************************************
struct Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics
{
	struct CXUI_eventGetPlayerStateInfoText_Parms
	{
		FText ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Player State" },
		{ "ModuleRelativePath", "CXUI.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FTextPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FTextPropertyParams Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(CXUI_eventGetPlayerStateInfoText_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UCXUI, nullptr, "GetPlayerStateInfoText", Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::PropPointers), sizeof(Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::CXUI_eventGetPlayerStateInfoText_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x54020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::Function_MetaDataParams), Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::CXUI_eventGetPlayerStateInfoText_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UCXUI::execGetPlayerStateInfoText)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FText*)Z_Param__Result=P_THIS->GetPlayerStateInfoText();
	P_NATIVE_END;
}
// ********** End Class UCXUI Function GetPlayerStateInfoText **************************************

// ********** Begin Class UCXUI Function OnGearUpdate **********************************************
struct CXUI_eventOnGearUpdate_Parms
{
	int32 NewGear;
};
static FName NAME_UCXUI_OnGearUpdate = FName(TEXT("OnGearUpdate"));
void UCXUI::OnGearUpdate(int32 NewGear)
{
	CXUI_eventOnGearUpdate_Parms Parms;
	Parms.NewGear=NewGear;
	UFunction* Func = FindFunctionChecked(NAME_UCXUI_OnGearUpdate);
	ProcessEvent(Func,&Parms);
}
struct Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Vehicle" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Implemented in Blueprint to display the new gear */" },
#endif
		{ "ModuleRelativePath", "CXUI.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Implemented in Blueprint to display the new gear" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewGear;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::NewProp_NewGear = { "NewGear", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(CXUI_eventOnGearUpdate_Parms, NewGear), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::NewProp_NewGear,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UCXUI, nullptr, "OnGearUpdate", Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::PropPointers), sizeof(CXUI_eventOnGearUpdate_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x08080800, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::Function_MetaDataParams), Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(CXUI_eventOnGearUpdate_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UCXUI_OnGearUpdate()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UCXUI_OnGearUpdate_Statics::FuncParams);
	}
	return ReturnFunction;
}
// ********** End Class UCXUI Function OnGearUpdate ************************************************

// ********** Begin Class UCXUI Function OnSpeedUpdate *********************************************
struct CXUI_eventOnSpeedUpdate_Parms
{
	float NewSpeed;
};
static FName NAME_UCXUI_OnSpeedUpdate = FName(TEXT("OnSpeedUpdate"));
void UCXUI::OnSpeedUpdate(float NewSpeed)
{
	CXUI_eventOnSpeedUpdate_Parms Parms;
	Parms.NewSpeed=NewSpeed;
	UFunction* Func = FindFunctionChecked(NAME_UCXUI_OnSpeedUpdate);
	ProcessEvent(Func,&Parms);
}
struct Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Vehicle" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Implemented in Blueprint to display the new speed */" },
#endif
		{ "ModuleRelativePath", "CXUI.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Implemented in Blueprint to display the new speed" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFloatPropertyParams NewProp_NewSpeed;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::NewProp_NewSpeed = { "NewSpeed", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(CXUI_eventOnSpeedUpdate_Parms, NewSpeed), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::NewProp_NewSpeed,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UCXUI, nullptr, "OnSpeedUpdate", Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::PropPointers), sizeof(CXUI_eventOnSpeedUpdate_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x08080800, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::Function_MetaDataParams), Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(CXUI_eventOnSpeedUpdate_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UCXUI_OnSpeedUpdate()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UCXUI_OnSpeedUpdate_Statics::FuncParams);
	}
	return ReturnFunction;
}
// ********** End Class UCXUI Function OnSpeedUpdate ***********************************************

// ********** Begin Class UCXUI ********************************************************************
void UCXUI::StaticRegisterNativesUCXUI()
{
	UClass* Class = UCXUI::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GetGameStateInfoText", &UCXUI::execGetGameStateInfoText },
		{ "GetPlayerStateInfoText", &UCXUI::execGetPlayerStateInfoText },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
FClassRegistrationInfo Z_Registration_Info_UClass_UCXUI;
UClass* UCXUI::GetPrivateStaticClass()
{
	using TClass = UCXUI;
	if (!Z_Registration_Info_UClass_UCXUI.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("CXUI"),
			Z_Registration_Info_UClass_UCXUI.InnerSingleton,
			StaticRegisterNativesUCXUI,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_UCXUI.InnerSingleton;
}
UClass* Z_Construct_UClass_UCXUI_NoRegister()
{
	return UCXUI::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UCXUI_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n *  Simple Vehicle HUD class\n *  Displays the current speed and gear.\n *  Widget setup is handled in a Blueprint subclass.\n */" },
#endif
		{ "IncludePath", "CXUI.h" },
		{ "ModuleRelativePath", "CXUI.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Simple Vehicle HUD class\nDisplays the current speed and gear.\nWidget setup is handled in a Blueprint subclass." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChatUI_MetaData[] = {
		{ "BindWidget", "" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "CXUI.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bIsMPH_MetaData[] = {
		{ "Category", "Vehicle" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Controls the display of speed in Km/h or MPH */" },
#endif
		{ "ModuleRelativePath", "CXUI.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Controls the display of speed in Km/h or MPH" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ChatUI;
	static void NewProp_bIsMPH_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bIsMPH;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UCXUI_GetGameStateInfoText, "GetGameStateInfoText" }, // 3858059399
		{ &Z_Construct_UFunction_UCXUI_GetPlayerStateInfoText, "GetPlayerStateInfoText" }, // 3586937822
		{ &Z_Construct_UFunction_UCXUI_OnGearUpdate, "OnGearUpdate" }, // 135629682
		{ &Z_Construct_UFunction_UCXUI_OnSpeedUpdate, "OnSpeedUpdate" }, // 903518731
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UCXUI>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UCXUI_Statics::NewProp_ChatUI = { "ChatUI", nullptr, (EPropertyFlags)0x0124080000080008, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UCXUI, ChatUI), Z_Construct_UClass_UChatUI_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChatUI_MetaData), NewProp_ChatUI_MetaData) };
void Z_Construct_UClass_UCXUI_Statics::NewProp_bIsMPH_SetBit(void* Obj)
{
	((UCXUI*)Obj)->bIsMPH = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UCXUI_Statics::NewProp_bIsMPH = { "bIsMPH", nullptr, (EPropertyFlags)0x0020080000000015, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UCXUI), &Z_Construct_UClass_UCXUI_Statics::NewProp_bIsMPH_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bIsMPH_MetaData), NewProp_bIsMPH_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UCXUI_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCXUI_Statics::NewProp_ChatUI,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCXUI_Statics::NewProp_bIsMPH,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UCXUI_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UCXUI_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UUserWidget,
	(UObject* (*)())Z_Construct_UPackage__Script_CX,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UCXUI_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UCXUI_Statics::ClassParams = {
	&UCXUI::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UCXUI_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UCXUI_Statics::PropPointers),
	0,
	0x00B010A1u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UCXUI_Statics::Class_MetaDataParams), Z_Construct_UClass_UCXUI_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UCXUI()
{
	if (!Z_Registration_Info_UClass_UCXUI.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UCXUI.OuterSingleton, Z_Construct_UClass_UCXUI_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UCXUI.OuterSingleton;
}
UCXUI::UCXUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UCXUI);
UCXUI::~UCXUI() {}
// ********** End Class UCXUI **********************************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_MIR_CH4_CX_Source_CX_CXUI_h__Script_CX_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UCXUI, UCXUI::StaticClass, TEXT("UCXUI"), &Z_Registration_Info_UClass_UCXUI, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UCXUI), 3346139066U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_MIR_CH4_CX_Source_CX_CXUI_h__Script_CX_557092069(TEXT("/Script/CX"),
	Z_CompiledInDeferFile_FID_MIR_CH4_CX_Source_CX_CXUI_h__Script_CX_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_MIR_CH4_CX_Source_CX_CXUI_h__Script_CX_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
