/*
 * File:		io.h
 * Purpose:		Serial Input/Output routines
 *
 */

#ifndef _IO_H
#define _IO_H

/********************************************************************/

char	
in_char(void);

void
out_char(char);

int
char_present(void);

int		
printf_kinetis(const char *, ... );

int
sprintf_kinetis(char *, const char *, ... );

/********************************************************************/

#endif
