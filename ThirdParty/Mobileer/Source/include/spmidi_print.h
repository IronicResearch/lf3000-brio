#ifndef _SPMIDI_PRINT_H
#define _SPMIDI_PRINT_H

/* $Id: spmidi_print.h,v 1.14 2005/05/03 22:04:00 philjmsl Exp $ */
/**
 *
 * @file spmidi_print.h
 * @brief Tools for printing debug messages.
 *
 * Define the basic printing macros.
 *
 * PRTMSG(msg) prints a string.
 *
 * PRTNUMD(n) prints a number using base 10 decimal format.
 *
 * PRTNUMH(n) prints a number using base 16 hexadecimal format.
 *
 * @author Phil Burk, Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#if defined(WIN32) || defined(MACOSX) || defined(SPMIDI_USE_PRINTF)

#include <stdio.h>
	#define PRTMSG( msg ) { printf( msg ); fflush( stdout ); }
	#define PRTNUMD( num ) { printf("%ld ", (long)(num)); fflush(stdout); }
	#define PRTNUMH( num ) { printf("0x%08lX ", (long)(num)); fflush(stdout); }

#elif defined(SPMIDI_USE_PRINT)

#include <stdio.h>
	#define PRTMSG( msg ) { print( msg ); fflush( stdout ); }
	#define PRTNUMD( num ) { print("%ld ", (long)(num)); fflush(stdout); }
	#define PRTNUMH( num ) { print("0x%08lX ", (long)(num)); fflush(stdout); }

#else

#ifndef NULL
/* NULL is normally defined in stdio.h so we define it here. */
#define NULL ((void *) 0)
	#endif

#ifdef SPMIDI_USE_DEBUG_TOOLS

#include "DebugTools.h"
		#define PRTMSG( msg )  Debug_PrintString( msg )
		#define PRTNUMD( num ) Debug_PrintDecimalNumber( num )
		#define PRTNUMH( num ) Debug_PrintHexNumber( num )

#else

#define PRTMSG( msg )
		#define PRTNUMD( num )
		#define PRTNUMH( num )

#endif

#endif

#define PRTMSGNUMD( msg, num ) { PRTMSG( msg ); PRTNUMD( num ); PRTMSG("\n"); }
#define PRTMSGNUMH( msg, num ) { PRTMSG( msg ); PRTNUMH( num ); PRTMSG("\n"); }

#endif /* _SPMIDI_PRINT_H */
