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

****************************************************************/

#ifndef lint
static const char cvsid[] = "$Id: message.c,v 1.1 1999/09/09 14:29:50 phelps Exp $";
#endif /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"


#ifdef DEBUG
static long message_count;
#endif /* DEBUG */

struct message
{
    /* The receiver's reference count */
    int ref_count;

    /* The Subscription which was matched to generate the receiver */
    void *info;

    /* The receiver's group */
    char *group;

    /* The receiver's user */
    char *user;

    /* The receiver's string (tickertext) */
    char *string;

    /* The receiver's MIME type information */
    char *mime_type;

    /* The receiver's MIME arg */
    char *mime_args;

    /* The lifetime of the receiver in seconds */
    unsigned long timeout;

    /* The identifier for this message */
    char *id;

    /* The identifier for the message for which this is a reply */
    char *reply_id;
};


/* Creates and returns a new message */
message_t message_alloc(
    void *info,
    char *group,
    char *user,
    char *string,
    unsigned int timeout,
    char *mime_type,
    char *mime_args,
    char *id,
    char *reply_id)
{
    message_t self = (message_t) malloc(sizeof(struct message));

    self -> ref_count = 1;
    self -> info = info;
    self -> group = strdup(group);
    self -> user = strdup(user);
    self -> string = strdup(string);

#ifdef DEBUG
    printf("allocated message_t %p (%ld)\n", self, ++message_count);
#endif /* DEBUG */

    if (mime_type == NULL)
    {
	self -> mime_type = NULL;
    }
    else
    {
	self -> mime_type = strdup(mime_type);
    }

    if (mime_args == NULL)
    {
	self -> mime_args = NULL;
    }
    else
    {
	self -> mime_args = strdup(mime_args);
    }

    self -> timeout = timeout;

    if (id != NULL)
    {
	self -> id = strdup(id);
    }
    else
    {
	self -> id = NULL;
    }

    if (reply_id != NULL)
    {
	self -> reply_id = strdup(reply_id);
    }
    else
    {
	self -> reply_id = NULL;
    }

    return self;
}

/* Allocates another reference to the message_t */
message_t message_alloc_reference(message_t self)
{
    self -> ref_count++;
    return self;
}

/* Frees the memory used by the receiver */
void message_free(message_t self)
{
    /* Decrement the reference count */
    if (--self -> ref_count > 0)
    {
	return;
    }

#ifdef DEBUG
    printf("freeing message_t %p (%ld):\n", self, --message_count);
    message_debug(self);
#endif /* DEBUG */

    /* Out of references -- release the hounds! */
    if (self -> group != NULL)
    {
	free(self -> group);
	self -> group = NULL;
    }

    if (self -> user != NULL)
    {
	free(self -> user);
	self -> user = NULL;
    }

    if (self -> string != NULL)
    {
	free(self -> string);
	self -> string = NULL;
    }

    if (self -> mime_type != NULL)
    {
	free(self -> mime_type);
	self -> mime_type = NULL;
    }

    if (self -> mime_args != NULL)
    {
	free(self -> mime_args);
	self -> mime_args = NULL;
    }

    if (self -> id != NULL)
    {
	free(self -> id);
	self -> id = NULL;
    }

    if (self -> reply_id != NULL)
    {
	free(self -> reply_id);
	self -> reply_id = NULL;
    }

    free(self);
}


/* Prints debugging information */
void message_debug(message_t self)
{
    printf("message_t (%p)\n", self);
    printf("  info = 0x%p\n", self -> info);
    printf("  group = \"%s\"\n", self -> group);
    printf("  user = \"%s\"\n", self -> user);
    printf("  string = \"%s\"\n", self -> string);
    printf("  mime_type = \"%s\"\n", (self -> mime_type == NULL) ? "<null>" : self -> mime_type);
    printf("  mime_args = \"%s\"\n", (self -> mime_args == NULL) ? "<null>" : self -> mime_args);
    printf("  timeout = %ld\n", self -> timeout);
    printf("  id = \"%s\"\n", (self -> id == NULL) ? "<null>" : self -> id);
    printf("  reply_id = \"%s\"\n", (self -> reply_id == NULL) ? "<null>" : self -> reply_id);
}



/* Answers the Subscription matched to generate the receiver */
void *message_get_info(message_t self)
{
    return self -> info;
}

/* Answers the receiver's group */
char *message_get_group(message_t self)
{
    return self -> group;
}

/* Answers the receiver's user */
char *message_get_user(message_t self)
{
    return self -> user;
}


/* Answers the receiver's string */
char *message_get_string(message_t self)
{
    return self -> string;
}

/* Answers the receiver's timout */
unsigned long message_get_timeout(message_t self)
{
    return self -> timeout;
}

/* Sets the receiver's timeout */
void message_set_timeout(message_t self, unsigned long timeout)
{
    self -> timeout = timeout;
}

/* Answers non-zero if the receiver has a MIME attachment */
int message_has_attachment(message_t self)
{
    return (self -> mime_args != NULL);
}

/* Answers the receiver's MIME-type string */
char *message_get_mime_type(message_t self)
{
    return self -> mime_type;
}

/* Answers the receiver's MIME arguments */
char *message_get_mime_args(message_t self)
{
    return self -> mime_args;
}

/* Answers the receiver's id */
char *message_get_id(message_t self)
{
    return self -> id;
}

/* Answers the id of the message_t for which this is a reply */
char *message_get_reply_id(message_t self)
{
    return self -> reply_id;
}
