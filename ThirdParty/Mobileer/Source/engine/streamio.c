/* $Id: streamio.c,v 1.5 2005/11/28 19:07:17 philjmsl Exp $ */
/**
 * Stream IO - emulate file I/O from in-memory char arrays.
 *
 * @author Phil Burk, Copyright 1997-2005 Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
 */

#include "streamio.h"

/******************************************************************
** Read from Stream
** Return number of bytes read.
*/
int Stream_Read( StreamIO *sio, char *buffer, int numBytes )
{
	return sio->read( sio, buffer, numBytes );
}

/******************************************************************
** Write to Stream
** Return number of bytes written.
*/
int Stream_Write( StreamIO *sio, char *buffer, int numBytes )
{
	return sio->read( sio, buffer, numBytes );
}

/******************************************************************
** Seek forwards or backwards in stream.
*/
int Stream_SetPosition( StreamIO *sio, int offset )
{
	return sio->setPosition( sio, offset );
}

/******************************************************************
** Report current position in stream.
*/
int Stream_GetPosition( StreamIO *sio )
{
	return sio->getPosition( sio);
}

/******************************************************************
** Close and free stream.
*/
void Stream_Close( StreamIO *sio )
{
	sio->close( sio );
}


