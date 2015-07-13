/*
 * Common Declarations
 *
 * - c.h is included by all modules.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/* Exported macros */
#define NULL ((void*)0)
#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

/* Typedefs */

/* #include "config.h" */

/* Interface */

/* Exported types */
enum { PERM=0, FUNC, STMT };

/* Exported data */

/* Exported functions */
