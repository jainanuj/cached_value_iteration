#ifndef _MED_HASH_H
#define _MED_HASH_H

#include <stdio.h>
#include <stdlib.h>

#define PROVIDE_DOUBLE

#define MH_ADD_OK       1
#define MH_ADD_REPLACED 2
#define MH_ADD_FULL     3

#define MH_NOTFOUND  0
#define MH_FOUND     1

#define MH_DEFAULT_EKEY 0xFFFFFFFF

typedef union val_t {
  void *vptr;
  float f;
  int i;
#ifdef PROVIDE_DOUBLE
  double d;
#endif
} val_t;

//
// This hash is designed to map integers to val_t.
//
// It will dynamically expand as you put more things into it.
// You can't take things out.
// It's not designed to be fancy.  Simple and fast.
//

//
// XXX
//
// This is an ADD-ONLY hash!  That means that you can't really remove
// things from it.  It will break.  I promise.
//
// XXX
//

typedef struct {
  int key;
  val_t val;
} datum_t;

typedef struct {
  int empty_val;  // a value that should never be used as a key
  int nalloc;     // the number of elements allocated
  int nelts;      // the number of elements in the hash
  datum_t *data;   // the data in the hash
  int collisions;
} med_hash_t;

med_hash_t *med_hash_create( int nalloc );
med_hash_t *med_hash_create_ekey( int nalloc, int empty_val );
void med_hash_destroy( med_hash_t *m );

int med_hash_add( med_hash_t *m, int key, val_t val );
int med_hash_get( med_hash_t *m, int key, val_t *val );
int med_hash_check( med_hash_t *m, int key );

int med_hash_getp( med_hash_t *m, int key, val_t **val );

int med_hash_add_float( med_hash_t *m, int key, float val );
int med_hash_get_float( med_hash_t *m, int key, float *val );
int med_hash_get_floatp( med_hash_t *m, int key, float **val );

#ifdef PROVIDE_DOUBLE
int med_hash_add_double( med_hash_t *m, int key, double val );
int med_hash_get_double( med_hash_t *m, int key, double *val );
int med_hash_get_doublep( med_hash_t *m, int key, double **val );
#endif

void med_hash_clear( med_hash_t *m );

int med_hash_iterate( med_hash_t *m, int *index,
		      int *key, val_t **val );
int med_hash_iterate_float( med_hash_t *m, int *index,
			    int *key, float **val );
#ifdef PROVIDE_DOUBLE
int med_hash_iterate_double( med_hash_t *m, int *index,
			     int *key, double **val );
#endif

int med_hash_set_add( med_hash_t *m1, int key1, int key2 );
int med_hash_hash_add( med_hash_t *m1, int key1, int key2, val_t val );

int med_hash_hash_iterate( med_hash_t *m1, int *index1, int *index2,
			   int *key1, int *key2, val_t **val );

#endif
