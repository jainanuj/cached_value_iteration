
#include <stdio.h>
#include <stdlib.h>

#ifndef _HEAP_H
#define _HEAP_H

/* #define HEAP_LTVAL 1 */
/* #define HEAP_GTVAL -1 */
/* #define HEAP_EQVAL 0 */

#define HEAP_LTVAL 0
#define HEAP_GTVAL 1
#define HEAP_EQVAL 0

typedef int(*compare_t)( int a, int b, void *userdata );
typedef void(*swap_t)( int a, int b, void *userdata );
typedef void(*add_t)( int obj, int pos, void *userdata );

typedef struct {
  int *items;
  int numitems, maxitems;
  compare_t comparefunc;
  swap_t swapfunc;
  add_t addfunc;
  void *userdata;
} heap;

heap *heap_create( int maxitems, compare_t c, swap_t s, add_t a, void *data );
int heap_peek( heap *h, int pos, int *result );
int heap_add( heap *h, int a );
int heap_pop( heap *h, int *result );
int heap_remove( heap *h, int item_num, int *result );
void heap_blast_init( heap *h );
void heap_dump( heap *h );

int heap_verify(  heap *h );

//inline void swap( heap *h, int a, int b );
int bubble_up( heap *h, int pos );
int bubble_down( heap *h, int pos );
int heap_clear(heap *h);

#endif
