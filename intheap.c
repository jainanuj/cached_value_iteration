
#include "intheap.h"

//
// ----------------------------------------------------------------------------
//

heap *heap_create( int maxitems, compare_t comparefunc, swap_t swapfunc,
		   add_t addfunc, void *userdata ) {
  heap *h;

  h = (heap *)malloc( sizeof(heap) );
  if (h == NULL) {
    fprintf(stderr, "Out of memory!\n");
    exit(0);
  }

  h->maxitems = maxitems;
  h->numitems = 0;
  h->comparefunc = comparefunc;
  h->swapfunc = swapfunc;
  h->addfunc = addfunc;
  h->userdata = userdata;

  h->items = (int *)malloc( maxitems * sizeof(int) );
  if (h->items == NULL) {
    fprintf(stderr, "Out of memory!\n");
    exit(0);
  }

  return h;
}

int heap_clear(heap *h)
{
    h->numitems = 0;
    return 0;
}
//
// ----------------------------------------------------------------------------
//

int heap_peek( heap *h, int pos, int *result ) {
  if (pos >= h->numitems) {
    fprintf( stderr, "Invalid heap peek: %d (max=%d)\n", pos, h->numitems );
    return 0;
  }
  *result = h->items[pos];
  return 1;
}

//
// ----------------------------------------------------------------------------
//
// dump the object at the end of the array and bubble up.
//

int heap_add( heap *h, int obj ) {
  int pos;

    if ( h->numitems >= h->maxitems ) {
        fprintf(stderr, "Hey!  Heap's full!\n");
        return 0;
    }

  // tack the new object onto the end of the heap.
  pos = h->numitems;
  h->items[ pos ] = obj;
  h->numitems++;

  if ( h->addfunc != NULL ) {
        (*h->addfunc)( obj, pos, h->userdata );
    }
  bubble_up( h, pos );

    return 1;
}

//
// ----------------------------------------------------------------------------
//

int heap_pop( heap *h, int *result ) {
  if (h->numitems == 0) {
    fprintf( stderr, "Hey! Heap's empty!\n" );
    return 0;
  }

  *result = h->items[0];
  h->numitems--;

  // take the lowest element and put it up top
  swap( h, 0, h->numitems );

  bubble_down( h, 0 );

  return 1;
}

//
// ----------------------------------------------------------------------------
//

int heap_remove( heap *h, int item_num, int *result ) {
  int newpos;

  if ( item_num == 0 ) {
    return heap_pop( h, result );
  }

  if ( item_num > h->numitems-1) {
    fprintf( stderr, "Invalid heap access: %d (max=%d)!\n", item_num,
	     h->numitems );
    return 0;
  }

  *result = h->items[ item_num ];

  if ( item_num == h->numitems-1 ) {
    // last item?  done.
    h->numitems--;
    return 1;
  }

  // put the last element in the removed item's place
  swap( h, item_num, h->numitems-1 );
  h->numitems--;

  newpos = bubble_up( h, item_num );
  // and
  bubble_down( h, newpos );

  return 1;
}

//
// ----------------------------------------------------------------------------
//

void heap_blast_init( heap *h ) {
  int i, cnt;

  cnt = h->numitems = h->maxitems;

  for ( i=0; i<cnt; i++ ) {
    h->items[i] = i;
  }
}

void heap_dump_r( heap *h, int pos ) {
  int lchild, rchild;

  lchild = pos*2 + 1;
  rchild = pos*2 + 2;
  if ( pos >= h->numitems ) { return; }

  heap_dump_r( h, lchild );
  heap_dump_r( h, rchild );

  fprintf( stderr, "  %d: %d\n", pos, h->items[pos] );
}

void heap_dump( heap *h ) {
  heap_dump_r( h, 0 );
}

//
// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================
//

/*inline */ void swap( heap *h, int a, int b ) {
  int tmp;

  tmp = h->items[a];
  h->items[a] = h->items[b];
  h->items[b] = tmp;

  if ( h->swapfunc != NULL ) {
    (*(h->swapfunc))( h->items[a], h->items[b], h->userdata );
  }
}

//
// ----------------------------------------------------------------------------
//

int bubble_up( heap *h, int pos ) {
  int parent;

  // now heapify the result
  while (pos > 0) {
    parent = (pos-1) / 2;

    if ( (*(h->comparefunc))( h->items[pos], h->items[parent],
			      h->userdata ) == HEAP_GTVAL ) {
      swap( h, pos, parent );
    } else {
      break;
    }

    pos = parent;
  }

  return pos;
}


int bubble_down( heap *h, int pos ) {
  int rchild, lchild, target;

  lchild = pos*2 + 1;
  rchild = pos*2 + 2;

  while (lchild < h->numitems)
  {

    if ( rchild >= h->numitems )
    {
      // rchild is out of range.  there is only a left child, which
      // means that this is the last compare.
      if ((*(h->comparefunc))( h->items[pos], h->items[lchild],
			       h->userdata ) != HEAP_GTVAL )
      {
	swap( h, pos, lchild );
	pos = lchild;
      }
      break;
    }

    // the general case.  compare the children to each other,
    // then compare the parent to the larger of the children.

    if ((*(h->comparefunc))( h->items[lchild], h->items[rchild],
			     h->userdata ) == HEAP_GTVAL ) {
      target = lchild;
    } else {
      target = rchild;
    }

    if ((*(h->comparefunc))( h->items[pos], h->items[target],
			     h->userdata ) == HEAP_GTVAL ) {
      // I'm bigger than the target, and the target is bigger than the
      // other guy.  we're done!
      break;
    }
    swap( h, pos, target );
    pos = target;
    lchild = pos*2 + 1;
    rchild = pos*2 + 2;

  }

  return pos;
}

//
// ----------------------------------------------------------------------------
//

int heap_verify_r( heap *h, int pos ) {
  int lchild, rchild;

  if ( pos >= h->numitems ) {
    return 1;
  }

  lchild = pos*2 + 1;
  rchild = pos*2 + 2;

  if ( lchild < h->numitems ) {
    if ( (*(h->comparefunc))( h->items[pos],
			      h->items[lchild],
			      h->userdata ) == HEAP_LTVAL ) {
      fprintf( stderr, "ERROR BETWEEN %d and %d!\n", pos, lchild );
      return 0;
    }
  }

  if ( rchild < h->numitems ) {
    if ( (*(h->comparefunc))( h->items[pos],
			      h->items[rchild],
			      h->userdata ) == HEAP_LTVAL ) {
      fprintf( stderr, "ERROR BETWEEN %d and %d!\n", pos, rchild );
      return 0;
    }
  }

  if ( heap_verify_r( h, lchild ) ) {
    return heap_verify_r( h, rchild );
  } else {
    return 0;
  }
}

int heap_verify( heap *h ) {
  return heap_verify_r( h, 0 );
}

//
// ----------------------------------------------------------------------------
//
