//============================================================================
// 
// good.h:
// 		Define the C++ layout of the "Data" dataset
// 
// Created on: Mon Oct 15 10:35:35 2007
// 
//============================================================================
#ifndef GOOD_HEADER_LDKDSU
#define GOOD_HEADER_LDKDSU

#define _Packed
#include <CoreTypes.h>

LF_USING_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
_Packed struct tMonsterStrength
{
	U32                 nAttackAndDefense;
	S32*                pAttackAndDefense;
};

//----------------------------------------------------------------------------
_Packed struct tMisc
{
	S32                 NumberOfPlayers;
	U32                 nRewards;
	char**              pRewards;
};

//----------------------------------------------------------------------------
_Packed struct tRoom
{
	S32                 squareFootage;
	char*               roomName;
};

//----------------------------------------------------------------------------
_Packed struct tSet
{
	S32                 Min_digit1;
	char*               sign1;
	U32                 ntoken_1;
	S32*                ptoken_1;
};

//----------------------------------------------------------------------------
_Packed struct tLevel
{
	U32                 nSet;
	tSet*               pSet;
};

//----------------------------------------------------------------------------
_Packed struct tMonsterAudio
{
	U32                 MonsterName;
	U32                 nMonsterSounds;
	U32*                pMonsterSounds;
};

//----------------------------------------------------------------------------
_Packed struct tData
{
	char*               Title;
	S32                 Version;
	U32                 nMonsters;
	char**              pMonsters;
	U32                 nMonsterStrength;
	tMonsterStrength*   pMonsterStrength;
	tMisc               Misc;
	U32                 nRoom;
	tRoom*              pRoom;
	U32                 nLevel;
	tLevel*             pLevel;
	U32                 nMonsterAudio;
	tMonsterAudio*      pMonsterAudio;
};


//----------------------------------------------------------------------------
// Utility function declarations
//----------------------------------------------------------------------------
void Dump_Data( const tData* pData );
tData* Rebase_Data( void* pBlob, U32 romLocation = 0 );

#endif
