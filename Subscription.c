/* $Id: Subscription.c,v 1.15 1998/10/23 17:01:44 phelps Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "sanity.h"
#include "Subscription.h"
#include "FileStreamTokenizer.h"
#include "StringBuffer.h"

/* Sanity checking strings */
#ifdef SANITY
static char *sanity_value = "Subscription";
static char *sanity_freed = "Freed Subscription";
#endif /* SANITY */

/* The subscription data type */
struct Subscription_t
{
#ifdef SANITY
    char *sanity_check;
#endif /* SANITY */

    /* The name of the receiver's group */
    char *group;

    /* The receiver's subscription expression */
    char *expression;

    /* Non-zero if the receiver should appear in the groups menu */
    int inMenu;

    /* Non-zero if the receiver's mime-enabled messages should automatically appear */
    int hasNazi;

    /* The minimum number of minutes to display a message in the receiver's group */
    int minTime;

    /* The maximum number of minutes to display a message in the receiver's group */
    int maxTime;

    /* The receiver's ElvinConnection */
    ElvinConnection connection;

    /* The receiver's ElvinConnection information (for unsubscribing) */
    void *connectionInfo;

    /* The receiver's ControlPanel */ 
    ControlPanel controlPanel;

    /* The receiver's ControlPanel info (for unsubscribing/retitling) */
    void *controlPanelInfo;

    /* The receiver's callback and argument */
    SubscriptionCallback callback;
    void *context;
};



/*
 *
 * Static function headers
 *
 */
static void GetFromGroupFileLine(
    List list,
    FileStreamTokenizer tokenizer, char *firstToken,
    SubscriptionCallback callback, void *context);
static List GetFromGroupFile(
    FILE *file, SubscriptionCallback callback, void *context);
static void HandleNotify(Subscription self, en_notify_t notification);
static void SendMessage(Subscription self, Message message);


/*
 *
 * Static functions
 *
 */

/* Translate the 'menu' or 'no menu' token into non-zero or zero*/
static int TranslateMenu(char *token)
{
    if (token == NULL)
    {
	fprintf(stderr, "*** Unexpected end of groups file\n");
	exit(1);
    }

    switch (*token)
    {
	/* menu */
	case 'm':
	{
	    if (strcmp(token, "menu") == 0)
	    {
		return 1;
	    }
	    
	    break;
	}

	/* no menu */
	case 'n':
	{
	    if (strcmp(token, "no menu") == 0)
	    {
		return 0;
	    }

	    break;
	}
    }

    /* Invalid token */
    fprintf(stderr, "*** Expected \"menu\" or \"no menu\", got \"%s\"\n", token);
    exit(1);
}

/* Translate the 'manual' or 'auto' token into zero or non-zero */
static int TranslateMime(char *token)
{
    if (token == NULL)
    {
	fprintf(stderr, "*** Unexpected end of groups file\n");
	exit(1);
    }

    switch (*token)
    {
	/* auto */
	case 'a':
	{
	    if (strcmp(token, "auto") == 0)
	    {
		return 1;
	    }

	    break;
	}

	/* manual */
	case 'm':
	{
	    if (strcmp(token, "manual") == 0)
	    {
		return 0;
	    }

	    break;
	}
    }

    fprintf(stderr, "*** Expected \"manual\" or \"auto\", got \"%s\"\n", token);
    exit(1);
}


/* Translate a positive, integral token into an integer */
static int TranslateTime(char *token)
{
    char *pointer;
    int value = 0;

    for (pointer = token; *pointer != '\0'; pointer++)
    {
	int digit = *pointer - '0';

	if ((digit < 0) || (digit > 9))
	{
	    fprintf(stderr, "*** Expected numerical argument, got \"%s\"\n", token);
	    exit(1);
	}

	value = (value * 10) + digit;
    }

    return value;
}

/* Create a subscription from a line of the group file */
static void GetFromGroupFileLine(
    List list,
    FileStreamTokenizer tokenizer, char *firstToken,
    SubscriptionCallback callback, void *context)
{
    char *token;
    char *group;
    int inMenu, hasNazi, minTime, maxTime;
    Subscription subscription;

    /* Copy the group token */
    group = (char *)alloca(strlen(firstToken) + 1);
    strcpy(group, firstToken);

    /* 'menu' or 'no menu' */
    token = FileStreamTokenizer_next(tokenizer);
    inMenu = TranslateMenu(token);

    /* 'manual' or 'auto' mime */
    token = FileStreamTokenizer_next(tokenizer);
    hasNazi = TranslateMime(token);

    /* minimum time */
    token = FileStreamTokenizer_next(tokenizer);
    minTime = TranslateTime(token);

    /* maximum time */
    token = FileStreamTokenizer_next(tokenizer);
    maxTime = TranslateTime(token);

    /* optional subscription expression */
    FileStreamTokenizer_setWhitespace(tokenizer, "");
    token = FileStreamTokenizer_next(tokenizer);
    FileStreamTokenizer_setWhitespace(tokenizer, ":");

    if ((token == NULL) || (*token == '\n'))
    {
	StringBuffer buffer = StringBuffer_alloc();
	StringBuffer_append(buffer, "TICKERTAPE == \"");
	StringBuffer_append(buffer, group);
	StringBuffer_append(buffer, "\"");

	subscription = Subscription_alloc(
	    group, StringBuffer_getBuffer(buffer),
	    inMenu, hasNazi,
	    minTime, maxTime,
	    callback, context);
	StringBuffer_free(buffer);
    }
    else
    {
	subscription = Subscription_alloc(
	    group, token,
	    inMenu, hasNazi,
	    minTime, maxTime,
	    callback, context);
	FileStreamTokenizer_skipToEndOfLine(tokenizer);
    }

    List_addLast(list, subscription);
}


/* Read the next subscription from file (answers NULL if EOF) */
static List GetFromGroupFile(
    FILE *file,
    SubscriptionCallback callback, void *context)
{
    FileStreamTokenizer tokenizer = FileStreamTokenizer_alloc(file, ":", "\n");
    List list = List_alloc();
    char *token;

    /* Locate a non-empty line that doesn't begin with a '#' */
    while ((token = FileStreamTokenizer_next(tokenizer)) != NULL)
    {
	/* Comment line? */
	if (*token == '#')
	{
	    FileStreamTokenizer_skipToEndOfLine(tokenizer);
	}
	/* Useful line? */
	else if (*token != '\n')
	{
	    GetFromGroupFileLine(list, tokenizer, token, callback, context);
	}
    }

    FileStreamTokenizer_free(tokenizer);
    return list;
}


/* Delivers a notification which matches the receiver's subscription expression */
static void HandleNotify(Subscription self, en_notify_t notification)
{
    Message message;
    en_type_t type;
    char *user;
    char *text;
    int32 *timeout_p;
    int32 timeout;
    char *mimeType;
    char *mimeArgs;
    int32 *msg_id_p;
    int32 msg_id;
    int32 *thread_id_p;
    int32 thread_id;
    SANITY_CHECK(self);

    /* If we don't have a callback then just quit now */
    if (self -> callback == NULL)
    {
	return;
    }
    
    /* Get the user from the notification (if provided) */
    if ((en_search(notification, "USER", &type, (void **)&user) != 0) ||
	(type != EN_STRING))
    {
	user = "anonymous";
    }

    /* Get the text of the notification (if provided) */
    if ((en_search(notification, "TICKERTEXT", &type, (void **)&text) != 0) ||
	(type != EN_STRING))
    {
	text = "";
    }

    /* Get the timeout for the notification (if provided) */
    if ((en_search(notification, "TIMEOUT", &type, (void **)&timeout_p) != 0) ||
	(type != EN_INT32))
    {
	timeout_p = &timeout;
	timeout = 0;

	if (type == EN_STRING)
	{
	    char *timeoutString;
	    en_search(notification, "TIMEOUT", &type, (void **)&timeoutString);
	    timeout = atoi(timeoutString);
	    printf("timeout=%d\n", timeout);
	}

	timeout = (timeout == 0) ? 10 : timeout;
    }

    /* Make sure the timeout conforms */
    if (*timeout_p < self -> minTime)
    {
	*timeout_p = self -> minTime;
    }
    else if (*timeout_p > self -> maxTime)
    {
	*timeout_p = self -> maxTime;
    }

    /* Get the MIME type (if provided) */
    if ((en_search(notification, "MIME_TYPE", &type, (void **)&mimeType) != 0) ||
	(type != EN_STRING))
    {
	mimeType = NULL;
    }

    /* Get the MIME args (if provided) */
    if ((en_search(notification, "MIME_ARGS", &type, (void **)&mimeArgs) != 0) ||
	(type != EN_STRING))
    {
	mimeArgs = NULL;
    }

    /* Get the message id (if provided) */
    if ((en_search(notification, "message", &type, (void **)&msg_id_p) != 0) ||
	(type != EN_INT32))
    {
	msg_id = 0;
	msg_id_p = &msg_id;
    }

    /* Get the message id (if provided) */
    if ((en_search(notification, "thread", &type, (void **)&thread_id_p) != 0) ||
	(type != EN_INT32))
    {
	thread_id = 0;
	thread_id_p = &thread_id;
    }

    /* Construct a message */
    message = Message_alloc(
	self -> controlPanelInfo, self -> group,
	user, text, *timeout_p,
	mimeType, mimeArgs,
	*msg_id_p, *thread_id_p);

    /* Deliver the message */
    (*self -> callback)(self -> context, message);
}

/* Sends a Message via this Subscription */
static void SendMessage(Subscription self, Message message)
{
    en_notify_t notification;
    int32 timeout, msg_id, thread_id;
    char *mimeArgs, *mimeType;
    
    SANITY_CHECK(self);

    timeout = Message_getTimeout(message);
    msg_id = Message_getId(message);
    thread_id = Message_getThreadId(message);
    mimeArgs = Message_getMimeArgs(message);
    mimeType = Message_getMimeType(message);

    notification = en_new();
    en_add_string(notification, "TICKERTAPE", self -> group);
    en_add_string(notification, "USER", Message_getUser(message));
    en_add_int32(notification, "TIMEOUT", timeout);
    en_add_string(notification, "TICKERTEXT", Message_getString(message));
    en_add_int32(notification, "message", msg_id);
    en_add_int32(notification, "thread", thread_id);

    /* Add mime information if both mimeArgs and mimeType are provided */
    if ((mimeArgs != NULL) && (mimeType != NULL))
    {
	en_add_string(notification, "MIME_ARGS", mimeArgs);
	en_add_string(notification, "MIME_TYPE", mimeType);
    }

    ElvinConnection_send(self -> connection, notification);
    en_free(notification);
}


/*
 *
 * Exported functions
 *
 */

/* Read Subscriptions from the group file 'groups' and add them to 'list' */
List Subscription_readFromGroupFile(
    FILE *groups, SubscriptionCallback callback, void *context)
{
    return GetFromGroupFile(groups, callback, context);
}


/* Answers a new Subscription */
Subscription Subscription_alloc(
    char *group,
    char *expression,
    int inMenu,
    int autoMime,
    int minTime,
    int maxTime,
    SubscriptionCallback callback,
    void *context)
{
    Subscription self = (Subscription) malloc(sizeof(struct Subscription_t));
#ifdef SANITY
    self -> sanity_check = sanity_value;
#endif /* SANITY */
    self -> group = strdup(group);
    self -> expression = strdup(expression);
    self -> inMenu = inMenu;
    self -> hasNazi = autoMime;
    self -> minTime = minTime;
    self -> maxTime = maxTime;
    self -> controlPanel = NULL;
    self -> controlPanelInfo = NULL;
    self -> connection = NULL;
    self -> connectionInfo = NULL;
    self -> callback = callback;
    self -> context = context;

    return self;
}

/* Releases resources used by a Subscription */
void Subscription_free(Subscription self)
{
    SANITY_CHECK(self);

    if (self -> group)
    {
	free(self -> group);
    }

#ifdef SANITY
    self -> sanity_check = sanity_freed;
#endif /* SANITY */
    free(self);
}

/* Prints debugging information */
void Subscription_debug(Subscription self)
{
    SANITY_CHECK(self);
    printf("Subscription (%p)\n", self);
#ifdef SANITY
    printf("  sanity_check = \"%s\"\n", self -> sanity_check);
#endif /* SANITY */    
    printf("  group = \"%s\"\n", self -> group ? self -> group : "<none>");
    printf("  expression = \"%s\"\n", self -> expression ? self -> expression : "<none>");
    printf("  inMenu = %s\n", self -> inMenu ? "true" : "false");
    printf("  hasNazi = %s\n", self -> hasNazi ? "true" : "false");
    printf("  minTime = %d\n", self -> minTime);
    printf("  maxTime = %d\n", self -> maxTime);
    printf("  connection = %p\n", self -> connection);
    printf("  connectionInfo = %p\n", self -> connectionInfo);
    printf("  controlPanel = %p\n", self -> controlPanel);
    printf("  controlPanelInfo = %p\n", self -> controlPanelInfo);
    printf("  callback = %p\n", self -> callback);
    printf("  context = %p\n", self -> context);
}


/* Answers the receiver's group */
char *Subscription_getGroup(Subscription self)
{
    SANITY_CHECK(self);
    return self -> group;
}


/* Answers the receiver's subscription expression */
char *Subscription_getExpression(Subscription self)
{
    SANITY_CHECK(self);
    return self -> expression;
}


/* Answers if the receiver should appear in the Control Panel menu */
int Subscription_isInMenu(Subscription self)
{
    SANITY_CHECK(self);
    return self -> inMenu;
}


/* Answers true if the receiver should automatically show mime messages */
int Subscription_isAutoMime(Subscription self)
{
    SANITY_CHECK(self);
    return self -> hasNazi;
}


/* Sets the receiver's ElvinConnection */
void Subscription_setConnection(Subscription self, ElvinConnection connection)
{
    if (self -> connection != NULL)
    {
	ElvinConnection_unsubscribe(self -> connection, self -> connectionInfo);
    }

    self -> connection = connection;

    if (self -> connection != NULL)
    {
	self -> connectionInfo = ElvinConnection_subscribe(
	    self -> connection, self -> expression,
	    (NotifyCallback)HandleNotify, self);
    }
}


/* Registers the receiver with the ControlPanel */
void Subscription_setControlPanel(
    Subscription self, ControlPanel controlPanel)
{
    SANITY_CHECK(self);

    /* If it's the same control panel we had before then bail */
    if (self -> controlPanel == controlPanel)
    {
	return;
    }

    if (self -> controlPanel != NULL)
    {
	ControlPanel_removeSubscription(self -> controlPanel, self -> controlPanelInfo);
    }

    self -> controlPanel = controlPanel;

    if ((self -> controlPanel != NULL) && (self -> inMenu))
    {
	self -> controlPanelInfo = ControlPanel_addSubscription(
	    controlPanel,
	    self -> group,
	    (ControlPanelCallback) SendMessage,
	    self);
    }
}
