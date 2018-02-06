
#include "med_hash.h"

/*
 * ----------------------------------------------------------------------------
 */

// private functions

int med_hash_expand( med_hash_t *m, int nalloc );
int med_hash_real_add( int nalloc, datum_t *data, int empty_val,
			      int key, val_t val, int *collisions );

/*
 * ----------------------------------------------------------------------------
 */

med_hash_t *med_hash_create( int nalloc ) {
  return med_hash_create_ekey( nalloc, MH_DEFAULT_EKEY );
}

med_hash_t *med_hash_create_ekey( int nalloc, int empty_val ) {
  med_hash_t *m;
  int i;

  // ensure some sanity.
  if ( nalloc == 0 ) {
    nalloc = 1;
  }

  m = (med_hash_t *)malloc( sizeof(med_hash_t) );
  if ( m == NULL ) {
    return NULL;
  }

  m->data = (datum_t *)malloc( sizeof(datum_t) * nalloc );
  if ( m->data == NULL ) {
    free( m );
    return NULL;
  }

  for ( i=0; i<nalloc; i++ ) {
    m->data[i].key = empty_val;
  }

  m->nelts = 0;
  m->nalloc = nalloc;
  m->empty_val = empty_val;
  m->collisions = 0;

  return m;
}

void med_hash_destroy( med_hash_t *m ) {
  free( m->data );
  free( m );
}

/*
 * ----------------------------------------------------------------------------
 */

int med_hash_add( med_hash_t *m, int key, val_t val ) {
  int rval, newsize;

  if ( m->nelts > m->nalloc / 2 ) {
    // due to our hashing algorithm, we never want a hash to get much
    // more than half full.  If it is, we want to expand it.

    if ( m->nelts > m->nalloc ) {
      newsize = m->nelts * 2;
    } else {
      newsize = m->nalloc * 2;
    }

    if ( !med_hash_expand( m, newsize ) ) {
      // can't expand.  Well, we're still not full, so we'll go ahead
      // and keep adding things, but our efficiency will drop dramatically.
      if ( m->nelts == m->nalloc ) {
	return MH_ADD_FULL; // we're really full.
      }
    }
  }

  rval = med_hash_real_add( m->nalloc, m->data, m->empty_val, key, val,
			    &(m->collisions) );

  if ( rval == MH_ADD_OK ) {
    m->nelts++;
  }

  return rval;
}

/*
 * ----------------------------------------------------------------------------
 */

int med_hash_check( med_hash_t *m, int key ) {
  val_t v;
  return med_hash_get( m, key, &v );
}

int med_hash_getp( med_hash_t *m, int key, val_t **val ) {
  int start_index, index;

  // a little bit of optimization...
  if ( key < m->nalloc && m->data[ key ].key == key ) {
    *val = &( m->data[ key ].val );
    return MH_FOUND;
  }

  start_index = key % m->nalloc;
  index = start_index;

  while ( 1 ) {

    if ( m->data[ index ].key == key ) {
      *val = &( m->data[ index ].val );
      return MH_FOUND;
    }

    if ( m->data[ index ].key == m->empty_val ) {
      return MH_NOTFOUND;
    }

    index++;

    if ( index == m->nalloc ) {
      index = 0;
    }

    if ( index == start_index ) {
      break;
    }

  }

  return MH_NOTFOUND;
}


int med_hash_get( med_hash_t *m, int key, val_t *val ) {
  int start_index, index;

  // a little bit of optimization...
  if ( key < m->nalloc && m->data[ key ].key == key ) {
    *val = m->data[ key ].val;
    return MH_FOUND;
  }

  start_index = key % m->nalloc;
  index = start_index;

  while ( 1 ) {

    if ( m->data[ index ].key == key ) {
      *val = m->data[ index ].val;
      return MH_FOUND;
    }

    if ( m->data[ index ].key == m->empty_val ) {
      return MH_NOTFOUND;
    }

    index++;

    if ( index == m->nalloc ) {
      index = 0;
    }

    if ( index == start_index ) {
      break;
    }

  }

  return MH_NOTFOUND;
}


/*
 * ----------------------------------------------------------------------------
 */

int med_hash_add_float( med_hash_t *m, int key, float val ) {
  val_t v;
  v.f = val;
  return med_hash_add( m, key, v );
}

int med_hash_get_float( med_hash_t *m, int key, float *val ) {
  int r;
  val_t v;

  r = med_hash_get( m, key, &v );

  if ( r == MH_FOUND ) {
    *val = v.f;
  }

  return r;
}

int med_hash_get_floatp( med_hash_t *m, int key, float **val ) {
  int r;
  val_t *v;

  r = med_hash_getp( m, key, &v );

  if ( r == MH_FOUND ) {
    *val = &(v->f);
  }

  return r;
}

/*
 * ----------------------------------------------------------------------------
 */

#ifdef PROVIDE_DOUBLE

int med_hash_add_double( med_hash_t *m, int key, double val ) {
  val_t v;
  v.d = val;
  return med_hash_add( m, key, v );
}

int med_hash_get_double( med_hash_t *m, int key, double *val ) {
  int r;
  val_t v;

  r = med_hash_get( m, key, &v );

  if ( r == MH_FOUND ) {
    *val = v.d;
  }

  return r;
}

int med_hash_get_doublep( med_hash_t *m, int key, double **val ) {
  int r;
  val_t *v;

  r = med_hash_getp( m, key, &v );

  if ( r == MH_FOUND ) {
    *val = &(v->d);
  }

  return r;
}

#endif

/*
 * ----------------------------------------------------------------------------
 */

void med_hash_clear( med_hash_t *m ) {
  int i;

  for ( i=0; i<m->nalloc; i++ ) {
    m->data[ i ].key = m->empty_val;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

int med_hash_iterate( med_hash_t *m, int *index,
		      int *key, val_t **val ) {
  if ( *index >= m->nalloc || *index < 0 ) {
    return 0;
  }

  while ( 1 ) {
    if ( m->data[ *index ].key != m->empty_val ) {
      *key = m->data[ *index ].key;
      *val = &( m->data[ *index ].val );
      *index += 1;
      return 1;
    }

    *index += 1;

    if ( *index >= m->nalloc || *index == 0 ) {
      return 0;
    }
  }
}

int med_hash_iterate_float( med_hash_t *m, int *index,
			    int *key, float **val ) {
  val_t *v;

  if ( med_hash_iterate( m, index, key, &v ) ) {
    *val = &(v->f);
    return 1;
  }

  return 0;
}

#ifdef PROVIDE_DOUBLE

int med_hash_iterate_double( med_hash_t *m, int *index,
			    int *key, double **val ) {
  val_t *v;

  if ( med_hash_iterate( m, index, key, &v ) ) {
    *val = &(v->d);
    return 1;
  }

  return 0;
}

#endif

/*
 * ----------------------------------------------------------------------------
 * HASH HASH FUNCTIONS
 * ----------------------------------------------------------------------------
 */

// XXX this is a surrogate for true set functionality!
int med_hash_set_add( med_hash_t *m1, int key1, int key2 ) {
  val_t v;
  // we don't care what this value is.
  v.i = 0;
  return med_hash_hash_add( m1, key1, key2, v );
}

int med_hash_hash_add( med_hash_t *m1, int key1, int key2, val_t val ) {
  med_hash_t *m2;
  int rval;
  val_t v;

  rval = med_hash_get( m1, key1, &v );

  if ( rval == MH_NOTFOUND ) {
    // key1 didn't have an entry in the hash.
    // create a new hash and add it.
    //    printf( "creating hash for key %d\n", key1 );
    m2 = med_hash_create( 4 );
    if ( m2 == NULL ) {
      fprintf( stderr, "Couldn't allocate hash hash!!\n");
      exit( 0 );
    }

    v.vptr = (void *)m2;
    rval = med_hash_add( m1, key1, v );
    if ( rval == MH_ADD_FULL ) {
      return rval;
    }

  } else {
    //    printf( "already have hash for key %d\n", key1 );
    m2 = (med_hash_t *)v.vptr;
  }

  return med_hash_add( m2, key2, val );
}

int med_hash_hash_iterate( med_hash_t *m1, int *index1, int *index2,
			   int *key1, int *key2, val_t **val ) {
  med_hash_t *m2;
  val_t tmpv;

  if ( *index1 >= m1->nalloc || *index1 < 0 ) {
    return 0;
  }

  while ( 1 ) {
    if ( m1->data[ *index1 ].key != m1->empty_val ) {
      *key1 = m1->data[ *index1 ].key;
      tmpv = m1->data[ *index1 ].val;
      m2 = (med_hash_t *)tmpv.vptr;

      //      printf("about to iterate on %d\n", *index1 );

      if ( med_hash_iterate( m2, index2, key2, val ) ) {
	return 1;
      }
    }

    *index1 += 1;
    *index2 = 0;

    if ( *index1 >= m1->nalloc || *index1 == 0 ) {
      return 0;
    }
  }
}

/*
 * ----------------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 * ----------------------------------------------------------------------------
 */

// I guess you could use this to contract as well.
// we don't actually depend on the fact that nalloc > m->nalloc, but
// the behavior isn't well defined.

int med_hash_expand( med_hash_t *m, int nalloc ) {
  int i;
  datum_t *d;

/*   printf( "expanded; collisions/nelts/nalloc: %d/%d/%d\n", */
/* 	  m->collisions, m->nelts, m->nalloc ); */

  d = (datum_t *)malloc( sizeof(datum_t) * nalloc );
  if ( d == NULL ) {
    return 0;
  }

  for ( i=0; i<nalloc; i++ ) {
    d[i].key = m->empty_val;
  }
  m->collisions = 0;

  // take old values and stick them into the new array.
  for ( i=0; i<m->nalloc; i++ ) {
    if ( m->data[ i ].key != m->empty_val ) {
      // add this key to the new array.
      med_hash_real_add( nalloc, d, m->empty_val,
			 m->data[ i ].key, m->data[ i ].val,
			 &( m->collisions ) );
    }
  }

  // ixnay old data
  free( m->data );

  m->data = d;
  m->nalloc = nalloc;

  return 1;
}

int med_hash_real_add( int nalloc, datum_t *data, int empty_val,
			      int key, val_t val, int *collisions ) {
  int start_index, index;

  // a little bit of optimization...
  if ( key < nalloc && data[ key ].key == empty_val ) {
    data[ key ].key = key;
    data[ key ].val = val;
    return MH_ADD_OK;
  }

  start_index = key % nalloc;
  index = start_index;

  while ( 1 ) {

    if ( data[ index ].key == empty_val ) {
      data[ index ].key = key;
      data[ index ].val = val;
      return MH_ADD_OK;
    }

    if ( data[ index ].key == key ) {
      data[ index ].val = val;
      return MH_ADD_REPLACED;
    }

    index++;
    *collisions = *collisions + 1;

    if ( index == nalloc ) {
      index = 0;
    }

    if ( index == start_index ) {
      break;
    }

  }

  return MH_ADD_FULL;
}

/*
 * ----------------------------------------------------------------------------
 */
