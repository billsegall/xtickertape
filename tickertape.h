/***************************************************************

  Copyright (C) DSTC Pty Ltd (ACN 052 372 577) 1995.
  Unpublished work.  All Rights Reserved.

  The software contained on this media is the property of the
  DSTC Pty Ltd.  Use of this software is strictly in accordance
  with the license agreement in the accompanying LICENSE.DOC
  file.  If your distribution of this software does not contain
  a LICENSE.DOC file then you have no rights to use this
  software in any manner and should contact DSTC at the address
  below to determine an appropriate licensing arrangement.

     DSTC Pty Ltd
     Level 7, Gehrmann Labs
     University of Queensland
     St Lucia, 4072
     Australia
     Tel: +61 7 3365 4310
     Fax: +61 7 3365 4311
     Email: enquiries@dstc.edu.au

  This software is being provided "AS IS" without warranty of
  any kind.  In no event shall DSTC Pty Ltd be liable for
  damage of any kind arising out of or in connection with
  the use or performance of this software.

   Description: 
             Represents a Scroller and ControlPanel and manages the
	     connection between them, the notification service and the 
	     user

****************************************************************/

#ifndef TICKERTAPE_H
#define TICKERTAPE_H

#ifndef lint
static const char cvs_TICKERTAPE_H[] = "$Id: tickertape.h,v 1.3 1999/08/19 04:37:51 phelps Exp $";
#endif /* lint */

typedef struct tickertape *tickertape_t;

#include <X11/Intrinsic.h>

/* Answers a new Tickertape for the given user using the given file as
 * her groups file and connecting to the notification service
 * specified by host and port */
tickertape_t tickertape_alloc(
    char *user, char *tickerDir,
    char *groupsFile, char *usenetFile,
    char *host, int port,
    Widget top);

/* Frees the resources used by the Tickertape */
void tickertape_free(tickertape_t self);

/* Prints out debugging information about the Tickertape */
void tickertape_debug(tickertape_t self);

/* Answers the tickertape's user name */
char *tickertape_user_name(tickertape_t self);

/* Reloads the tickertape's groups file */
void tickertape_reload_groups(tickertape_t self);

/* Reloads the tickertape's usenet file */
void tickertape_reload_usenet(tickertape_t self);

/* Quit the application */
void tickertape_quit(tickertape_t self);

#endif /* TICKERTAPE_H */