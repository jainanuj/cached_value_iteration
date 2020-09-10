//
//  intqueue_conc.h
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//#include "logger.h"

#ifndef _PART_QUEUE_CONC_H
#define _PART_QUEUE_CONC_H


/*#define queue_add queue_add_bit
#define queue_pop queue_pop_bit
#define queue_has_items bit_queue_has_items
#define queue_create create_bit_queue
#define queue bit_queue
*/

#define BITQ_CONC 1
#define BIT_ARRAY_SIZE 31
#define MAX_DEB_SEQ_SIZE 0xFFFFFFFF

#define EMPTYVAL 30000      //This is some big number. Treating it as empty val as we won't have that many partitions.
#define INT_SIZE 32

#define CAS __sync_bool_compare_and_swap

typedef struct {
    unsigned long *bit_arrays;
    int maxitems;
    int max_bit_arrays;
} bf_conc;

typedef struct {
  unsigned int *items;
  bf_conc* q_bf;
  int maxitems, queue_size;
  unsigned int FRONT, REAR;
  double add_time;
  double pop_time;
} queue_conc;

queue_conc* queue_conc_create( int maxitems, int max_val );
int queue_conc_enq( queue_conc *q, int obj );
int queue_conc_deq( queue_conc *q, unsigned int *result );
unsigned int queue_conc_has_items(queue_conc *q);
int empty_queue_conc( queue_conc *q);
void destroy_queue_conc(queue_conc *q);

//bit queues
bf_conc* create_bit_queue_conc( int maxitems );
//int queue_conc_add_bit( bit_queue_conc *q, int obj );
int bf_conc_add_bit( bf_conc *q, unsigned int obj );
int bf_atomic_unset( bf_conc *bq, int obj );
int bf_has_items(bf_conc *bq);
int empty_bf_conc(bf_conc *bq);
void destroy_bf_conc(bf_conc *bq);
unsigned long check_bit_obj_present_conc( bf_conc *bq, int obj );






#endif
