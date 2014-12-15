//============================================================================
// 
// Actor.h:
// 		Define the C++ layout of the "ActorData" dataset
// 
// Created on: Thu Nov 29 13:25:43 2007
// 
//============================================================================
#ifndef ACTOR_HEADER_LDKDSU
#define ACTOR_HEADER_LDKDSU

#define _Packed
#include <CoreTypes.h>

//----------------------------------------------------------------------------
_Packed struct tActorGraphicElementData
{
	S32                             ActorPositionX;
	S32                             ActorPositionY;
	S32                             ActorPositionZ;
	S32                             ActorRotation;
	S32                             ActorScaleX;
	S32                             ActorScaleY;
	S32                             ActorRegX;
	S32                             ActorRegY;
	S32                             ActorColor;
};

//----------------------------------------------------------------------------
_Packed struct tOperationData
{
	S32                             OperationOpcode;
	S32                             OperationOparg;
	char*                           OperationOpargString1;
	char*                           OperationOpargString2;
};

//----------------------------------------------------------------------------
_Packed struct tBillboardGraphicElementData
{
	S32                             BillboardPositionX;
	S32                             BillboardPositionY;
	S32                             BillboardPositionZ;
	S32                             BillboardRotation;
	S32                             BillboardScaleX;
	S32                             BillboardScaleY;
	S32                             BillboardRegX;
	S32                             BillboardRegY;
	S32                             BillboardColor;
};

//----------------------------------------------------------------------------
_Packed struct tBillboardData
{
	S32                             BillboardID;
	char*                           BillboardURL;
	tBillboardGraphicElementData    BillboardGraphicElementData;
};

//----------------------------------------------------------------------------
_Packed struct tSpriteCelData
{
	char*                           SpriteCelLabel;
	S32                             SpriteCelID;
	S32                             SpriteCelDuration;
	U32                             nOperationData;
	tOperationData*                 pOperationData;
	U32                             nBillboardData;
	tBillboardData*                 pBillboardData;
};

//----------------------------------------------------------------------------
_Packed struct tActorData
{
	tActorGraphicElementData        ActorGraphicElementData;
	U32                             nSpriteCelData;
	tSpriteCelData*                 pSpriteCelData;
};


//----------------------------------------------------------------------------
// Utility function declarations
//----------------------------------------------------------------------------
void Dump_ActorData( const tActorData* pData );
tActorData* Rebase_ActorData( void* pBlob, U32 romLocation = 0 );

#endif
