
/*
 * jerror.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains simple error-reporting and trace-message routines.
 * These are suitable for Unix-like systems and others where writing to
 * stderr is the right thing to do.  Many applications will want to replace
 * some or all of these routines.
 *
 * These routines are used by both the compression and decompression code.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jversion.h"
#include "jerror.h"

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif


/*
 * Create the message string table.
 * We do this from the master message list in jerror.h by re-reading
 * jerror.h with a suitable definition for macro JMESSAGE.
 * The message table is made an external symbol just in case any applications
 * want to refer to it directly.
 */

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_std_message_table	jMsgTable
#endif

#define JMESSAGE(code,string)	string ,

#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
#if	(defined( _CONSOLE ) || defined( DJPEG_DLL ))
#pragma message( "Including jpeg_std_message_table data here..." )
#undef	stderr
#define	stderr		stdout

const char MLPTR jpeg_std_message_table[] = {
#include "jerror.h"
  NULL
};

#else	// !_CONSOLE or DJPEG_DLL

extern	void	SetError2( int );
extern	int		bErrFlag2;

// MAYBE No ERROR DATA in DLL, or define table
#ifdef	ADDERRDATA
#pragma message( "Including message table data here..." )
const char MLPTR jpeg_std_message_table[] = {
#include "jerror.h"
  NULL
};
#endif	// ADDERRDATA

#endif	// DJPEG_DLL y/n
#else	// !_INC_WIND

const char MLPTR const jpeg_std_message_table[] = {
#include "jerror.h"
  NULL
};

#endif	// _INC_WINDOWS or WJPEG6 y/n

#ifdef WJPEG6
extern	BOOL	GotTraceOut( void );
extern	void	ExtTraceOut( LPSTR lpt );
extern	void	ExtDebugStop( int code );
#endif	// WJPEG6 y/n

/*
 * Error exit handler: must not return to caller.
 *
 * Applications may override this if they want to get control back after
 * an error.  Typically one would longjmp somewhere instead of exiting.
 * The setjmp buffer can be made a private field within an expanded error
 * handler object.  Note that the info needed to generate an error message
 * is stored in the error object, so you can generate the message now or
 * later, at your convenience.
 * You should make sure that the JPEG object is cleaned up (with jpeg_abort
 * or jpeg_destroy) at some point.
 */

METHODDEF(void)
error_exit (j_common_ptr cinfo)
{
  /* Always display the message */
  (*cinfo->err->output_message) (cinfo);

  /* Let the memory manager delete any temp files before we die */
  jpeg_destroy(cinfo);

#ifdef	WJPEG6

#if	(defined( DJPEG_DLL ) || defined( _CONSOLE ))
  exit(EXIT_FAILURE);
#else	// !DJPEG_DLL

#pragma message( "ADVICE: Making WJPG32_2 Library." )
  SetError2( bErrFlag2 );
#endif	// DJPEG_DLL y/n
#else	// !WJPEG6
  exit(EXIT_FAILURE);
#endif	// WJPEG6 y/n

}


/*
 * Actual output of an error or trace message.
 * Applications may override this method to send JPEG messages somewhere
 * other than stderr.
 */

METHODDEF(void)
output_message (j_common_ptr cinfo)
{

#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
	// Put message into hgInfo buffer (if supplied)
	struct jpeg_error_mgr MLPTR err = cinfo->err;
	HGLOBAL		hg;
	LPSTR		lpb;
	if( ( hg = err->hgErr ) &&
		( lpb = GlobalLock( hg ) ) )
	{
		// Create the message
		(*cinfo->err->format_message) (cinfo, lpb );
#if	(defined( DJPEG_DLL ) || defined( _CONSOLE ))
		fprintf(stderr, "%s\n", lpb );
#endif	// _DJPEG_DLL
		GlobalUnlock( hg );

	}

#else	// !_INC_WINDOWS or WJPEG6

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);
	/* Send it to stderr, adding a newline */
	fprintf(stderr, "%s\n", buffer);

#endif	// _INC_WINDOWS or WJPEG6 y/n

}

#ifdef WJPEG6
METHODDEF(void)
format_trace (j_common_ptr cinfo, char MLPTR buffer)
{
	struct jpeg_error_mgr MLPTR	err;
	int							msg_code;
	const char MLPTR			msgtext;
	const char MLPTR			msgptr;
	char						ch;
	boolean						isstring;

	err = cinfo->err;
	msg_code = err->msg_code;
	msgtext = NULL;

	/* Look up message string in proper table */
	if( ( msg_code > 0 ) &&
		( msg_code <= err->last_jpeg_message ) )
	{
		msgtext = err->jpeg_message_table[msg_code];
	}
	else if( (err->addon_message_table != NULL) &&
		( msg_code >= err->first_addon_message ) &&
		( msg_code <= err->last_addon_message ) )
	{
		msgtext = err->addon_message_table[msg_code - err->first_addon_message];
	}

	/* Defend against bogus message number */
	if( msgtext == NULL )
	{
		err->msg_parm.i[0] = msg_code;
		msgtext = err->jpeg_message_table[0];
	}

	/* Check for string parameter,
		as indicated by %s in the message text */
	isstring = FALSE;
	msgptr = msgtext;
	while( (ch = *msgptr++) != '\0' )
	{
		if( ch == '%' )
		{
			if( *msgptr == 's' )
				isstring = TRUE;
			break;
		}
	}

	/* Format the message into the passed buffer */
	if( isstring )
	{
		wsprintf( buffer, msgtext, err->msg_parm.s );
	}
	else
	{
		wsprintf(buffer,
			msgtext,
			err->msg_parm.i[0], err->msg_parm.i[1],
			err->msg_parm.i[2], err->msg_parm.i[3],
			err->msg_parm.i[4], err->msg_parm.i[5],
			err->msg_parm.i[6], err->msg_parm.i[7] );
	}
}

METHODDEF(void)
output_trace (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];
	if( GotTraceOut() )
	{
		//ExtDebugStop( 7 );
		/* Create the message */
		format_trace( cinfo, buffer );
		/* Send it to stderr, adding a newline */
		//fprintf(stderr, "%s\n", buffer);
		ExtTraceOut( &buffer[0] );
	}
}
#endif	// WJPEG6

/*
 * Decide whether to emit a trace or warning message.
 * msg_level is one of:
 *   -1: recoverable corrupt-data warning, may want to abort.
 *    0: important advisory messages (always display to user).
 *    1: first level of tracing detail.
 *    2,3,...: successively more detailed tracing messages.
 * An application might override this method if it wanted to abort on warnings
 * or change the policy about which messages to display.
 */

METHODDEF(void)
emit_message (j_common_ptr cinfo, int msg_level)
{
	struct jpeg_error_mgr MLPTR err = cinfo->err;

	if( msg_level < 0 )
	{
		/* It's a warning message.  Since corrupt files may generate many warnings,
		 * the policy implemented here is to show only the first warning,
		 * unless trace_level >= 3.
		 */
		if( ( err->num_warnings == 0 ) ||
			( err->trace_level >= 3 ) )
		{
			(*err->output_message) (cinfo);
		}
		/* Always count warnings in num_warnings. */
		err->num_warnings++;
	}
	else
	{
		/* It's a trace message.
			Show it if trace_level >= msg_level. */
		// OR if the EXTRACEOUT() is passed
#ifdef WJPEG6
		if( GotTraceOut() )
			output_trace( cinfo );
#else	// !WJPEG6
		if( ( err->trace_level >= msg_level )
			(*err->output_message) (cinfo);
#endif	// WJPEG6 y/n
	}
}


/*
 * Format a message string for the most recent JPEG error or message.
 * The message is stored into buffer, which should be at least JMSG_LENGTH_MAX
 * characters.  Note that no '\n' character is added to the string.
 * Few applications should need to override this method.
 */
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
char	szErrMsg[] = "ERROR: Code = %u\n";
#endif	// _INC_WINDOWS or WJPEG6

METHODDEF(void)
format_message (j_common_ptr cinfo, char MLPTR buffer)
{
	int		rcount;
	struct jpeg_error_mgr MLPTR err = cinfo->err;
	int msg_code = err->msg_code;
	const char MLPTR msgtext = NULL;
	const char MLPTR msgptr;
	char ch;
	boolean isstring;

	rcount = 0;
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
	bErrFlag2 = msg_code;	// SET the error code
#if	(defined( _CONSOLE ) || defined( ADDERRDATA ))

	/* Look up message string in proper table */
	if( ( msg_code > 0 ) &&
		( msg_code <= err->last_jpeg_message ) )
	{
		msgtext = err->jpeg_message_table[msg_code];
	}
	else if( (err->addon_message_table != NULL) &&
		( msg_code >= err->first_addon_message ) &&
		( msg_code <= err->last_addon_message ) )
	{
		msgtext = err->addon_message_table[msg_code - err->first_addon_message];
	}

	/* Defend against bogus message number */
	if( msgtext == NULL )
	{
		err->msg_parm.i[0] = msg_code;
		msgtext = err->jpeg_message_table[0];
	}

	/* Check for string parameter,
		as indicated by %s in the message text */
	isstring = FALSE;
	msgptr = msgtext;
	while( (ch = *msgptr++) != '\0' )
	{
		if( ch == '%' )
		{
			if( *msgptr == 's' )
				isstring = TRUE;
			break;
		}
	}

	/* Format the message into the passed buffer */
	if( isstring )
	{
		wsprintf( buffer, msgtext, err->msg_parm.s );
	}
	else
	{
		wsprintf(buffer, msgtext,
	    err->msg_parm.i[0], err->msg_parm.i[1],
	    err->msg_parm.i[2], err->msg_parm.i[3],
	    err->msg_parm.i[4], err->msg_parm.i[5],
	    err->msg_parm.i[6], err->msg_parm.i[7] );
	}

#else	// !_CONSOLE
	if( buffer )
	{
		isstring = TRUE;
		msgtext = &szErrMsg[0];
		msgptr = msgtext;
		wsprintf( buffer, msgtext, msg_code );

		while( ch = *msgptr++ )
		{
			rcount++;
		}
		bErrFlag2 = msg_code;
	}

#endif	// _CONSOLE
#else	// !(_INC_WINDOWS or WJPEG6)

	/* Look up message string in proper table */
	if( ( msg_code > 0 ) &&
		( msg_code <= err->last_jpeg_message ) )
	{
		msgtext = err->jpeg_message_table[msg_code];
	}
	else if( ( err->addon_message_table != NULL ) &&
		( msg_code >= err->first_addon_message ) &&
		( msg_code <= err->last_addon_message ) )
	{
		msgtext = err->addon_message_table[msg_code - err->first_addon_message];
	}

	/* Defend against bogus message number */
	if( msgtext == NULL )
	{
		err->msg_parm.i[0] = msg_code;
		msgtext = err->jpeg_message_table[0];
	}

	/* Check for string parameter,
		as indicated by %s in the message text */
	isstring = FALSE;
	msgptr = msgtext;
	while( (ch = *msgptr++) != '\0' )
	{
		if( ch == '%' )
		{
			if( *msgptr == 's' )
				isstring = TRUE;
			break;
		}
	}

	/* Format the message into the passed buffer */
	if( isstring )
	{
		sprintf(buffer, msgtext, err->msg_parm.s );
	}
	else
	{
		sprintf( buffer,
			msgtext,
			err->msg_parm.i[0], err->msg_parm.i[1],
			err->msg_parm.i[2], err->msg_parm.i[3],
			err->msg_parm.i[4], err->msg_parm.i[5],
			err->msg_parm.i[6], err->msg_parm.i[7] );
	}

#endif	// _INC_WINDOWS or WJPEG6

}


/*
 * Reset error state variables at start of a new image.
 * This is called during compression startup to reset trace/error
 * processing to default state, without losing any application-specific
 * method pointers.  An application might possibly want to override
 * this method if it has additional error processing state.
 */

METHODDEF(void)
reset_error_mgr (j_common_ptr cinfo)
{
  cinfo->err->num_warnings = 0;
  /* trace_level is not reset since it is an application-supplied parameter */
  cinfo->err->msg_code = 0;	/* may be useful as a flag for "no error" */
}


/*
 * Fill in the standard error-handling methods in a jpeg_error_mgr object.
 * Typical call is:
 *	struct jpeg_compress_struct cinfo;
 *	struct jpeg_error_mgr err;
 *
 *	cinfo.err = jpeg_std_error(&err);
 * after which the application may override some of the methods.
 */

GLOBAL(struct jpeg_error_mgr MLPTR )
jpeg_std_error (struct jpeg_error_mgr MLPTR err)
{
	err->error_exit = error_exit;
	err->emit_message = emit_message;
	err->output_message = output_message;
	err->format_message = format_message;
	err->reset_error_mgr = reset_error_mgr;

	err->trace_level = 0;		/* default = no tracing */
	err->num_warnings = 0;	/* no warnings emitted yet */
	err->msg_code = 0;		/* may be useful as a flag for "no error" */

	/* Initialize message table pointers */
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
#if	(defined( _CONSOLE ) || defined( DJPEG_DLL ))
	err->jpeg_message_table = jpeg_std_message_table;
#else	// !_any of above
#ifdef	ADDERRDATA
	err->jpeg_message_table = jpeg_std_message_table;
#else	// !ADDERRDATA
	// Data is NOT maintained in the DLL - For SIZE reasons
	// It can be placed in a resource, but really, if needed,
	// then the application should supply this POINTER
	err->jpeg_message_table = 0;
#endif	// ADDERRDATA
#endif	// _CONSOLE or DJPEG_DLL
#else	// !_INC_WINDOWS or WJPEG6
	err->jpeg_message_table = jpeg_std_message_table;
#endif	// _INC_WINDOWS or WJPEG6 - y/n

	err->last_jpeg_message = (int) JMSG_LASTMSGCODE - 1;

	err->addon_message_table = NULL;
	err->first_addon_message = 0;	/* for safety */
	err->last_addon_message = 0;

	return err;
}

// eof - Jerror.c
