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

#ifndef VM_H
#define VM_H

#ifndef lint
static const char cvs_VM_H[] = "$Id: vm.h,v 2.3 2000/11/18 00:57:35 phelps Exp $";
#endif /* lint */

/* Objects are really handles to the world outside the VM */
typedef struct vm_object *vm_object_t;

/* The virtual machine type */
typedef struct vm *vm_t;


/* The types of objects */
typedef enum
{
    /* Atomic types are 0-7 */
    SEXP_NIL = 0,
    SEXP_INTEGER,
    SEXP_LONG,
    SEXP_FLOAT,
    SEXP_STRING,
    SEXP_CHAR,

    /* Combined types are 8-15 */
    SEXP_SYMBOL = 8,
    SEXP_CONS,
    SEXP_PRIM,
    SEXP_LAMBDA,
    SEXP_ENV,
    SEXP_ARRAY
} object_type_t;

/* Allocates and initializes a new virtual machine */
vm_t vm_alloc(elvin_error_t error);

/* Frees a virtual machine */
int vm_free(vm_t self, elvin_error_t error);


/* Swaps the top two elements on the stack */
int vm_swap(vm_t self, elvin_error_t error);

/* Push nil onto the vm's stack */
int vm_push_nil(vm_t self, elvin_error_t error);

/* Pushes an integer onto the vm's stack */
int vm_push_integer(vm_t self, int32_t value, elvin_error_t error);

/* Pushes a long integer onto the vm's stack */
int vm_push_long(vm_t self, int64_t value, elvin_error_t error);

/* Pushes a float onto the vm's stack */
int vm_push_float(vm_t self, double value, elvin_error_t error);

/* Pushes a string onto the vm's stack */
int vm_push_string(vm_t self, char *value, elvin_error_t error);

/* Pushes a char onto the vm's stack */
int vm_push_char(vm_t self, int value, elvin_error_t error);


/* Makes a symbol out of the string on the top of the stack */
int vm_make_symbol(vm_t self, elvin_error_t error);

/* Creates a cons cell out of the top two elements on the stack */
int vm_make_cons(vm_t self, elvin_error_t error);

/* Reverses the pointers in a list that was constructed upside-down */
int vm_unwind_list(vm_t self, elvin_error_t error);


/* Evaluates the top of the stack, leaving the result in its place */
int vm_eval(vm_t self, elvin_error_t error);





/* Prints the top of the stack onto stdout */
int vm_print(vm_t self, elvin_error_t error);

/* For debugging only */
int vm_print_stack(vm_t self, elvin_error_t error);


#endif
