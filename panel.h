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

#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#ifndef lint
static const char cvs_CONTROL_PANEL_H[] = "$Id: panel.h,v 1.1 1999/10/05 02:55:46 phelps Exp $";
#endif /* lint */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* The control_panel_t datatype */
typedef struct control_panel *control_panel_t;

#include "message.h"
#include "tickertape.h"

/* The control_panel_t callback type */
typedef void (*control_panel_callback_t)(void *context, message_t message);

/* Allocates and initializes a new control_panel_t */
control_panel_t control_panel_alloc(tickertape_t tickertape, Widget parent);

/* Releases the resources used by the receiver */
void control_panel_free(control_panel_t self);

/* Adds a subscription to the receiver.  Returns information which is
 * needed in order to later remove or re-index the subscription */
void *control_panel_add_subscription(
    control_panel_t self, char *title,
    control_panel_callback_t callback, void *rock);

/* Removes a subscription from the receiver */
void control_panel_remove_subscription(control_panel_t self, void *rock);

/* Changes the location of the subscription in the control panel's menu */
void control_panel_set_index(control_panel_t self, void *rock, int index);

/* Changes the title of a subscription */
void control_panel_retitle_subscription(control_panel_t self, void *rock, char *title);

/* Makes the control_panel visible and selects the given message in
 * the history list */
void control_panel_select(control_panel_t self, message_t message);

/* Makes the control_panel visible */
void control_panel_show(control_panel_t self);

/* Handle notifications */
void control_panel_handle_notify(control_panel_t self, Widget widget);

#endif /* CONTROLPANEL_H */