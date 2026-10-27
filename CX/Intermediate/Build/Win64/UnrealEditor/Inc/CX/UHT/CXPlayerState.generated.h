// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "PlayerState/CXPlayerState.h"

#ifdef CX_CXPlayerState_generated_h
#error "CXPlayerState.generated.h already included, missing '#pragma once' in CXPlayerState.h"
#endif
#define CX_CXPlayerState_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class ACXPlayerState ***********************************************************
#define FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execGetPlayerStateInfo);


CX_API UClass* Z_Construct_UClass_ACXPlayerState_NoRegister();

#define FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesACXPlayerState(); \
	friend struct Z_Construct_UClass_ACXPlayerState_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend CX_API UClass* Z_Construct_UClass_ACXPlayerState_NoRegister(); \
public: \
	DECLARE_CLASS2(ACXPlayerState, APlayerState, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/CX"), Z_Construct_UClass_ACXPlayerState_NoRegister) \
	DECLARE_SERIALIZER(ACXPlayerState) \
	enum class ENetFields_Private : uint16 \
	{ \
		NETFIELD_REP_START=(uint16)((int32)Super::ENetFields_Private::NETFIELD_REP_END + (int32)1), \
		Coins=NETFIELD_REP_START, \
		BonusMaxSpeed, \
		NETFIELD_REP_END=BonusMaxSpeed	}; \
	DECLARE_VALIDATE_GENERATED_REP_ENUMS(NO_API)


#define FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API ACXPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	ACXPlayerState(ACXPlayerState&&) = delete; \
	ACXPlayerState(const ACXPlayerState&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ACXPlayerState); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ACXPlayerState); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ACXPlayerState) \
	NO_API virtual ~ACXPlayerState();


#define FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_7_PROLOG
#define FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_INCLASS_NO_PURE_DECLS \
	FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h_10_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class ACXPlayerState;

// ********** End Class ACXPlayerState *************************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_MIR_CH4_CX_Source_CX_PlayerState_CXPlayerState_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
