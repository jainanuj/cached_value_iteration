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

#define END_PLACE -4
#define START_PLACE -2
#define EMPTYVAL -1
#define START_TRANS -6

#define CAS __sync_bool_compare_and_swap

typedef struct {
    unsigned long *bit_arrays;
    //    int maxitems;
    int max_bit_arrays;
    int num_items;
} bit_queue_conc;

typedef struct {
  int *items;
  bit_queue_conc* bitqueue_conc;
  int numitems, maxitems, maxNumInts;
  int start_item_ptr, end_item_ptr;
  double add_time;
  double pop_time;
} queue_conc;

queue_conc* queue_conc_create( int maxitems, int max_val );
int queue_conc_add( queue_conc *q, int a );
int queue_conc_pop( queue_conc *q, int *result );
int queue_conc_has_items(queue_conc *q);
int empty_queue_conc( queue_conc *q);
void destroy_queue_conc(queue_conc *q);

//bit queues
bit_queue_conc* create_bit_queue_conc( int maxitems );
int queue_conc_add_bit( bit_queue_conc *q, int obj );
int bit_queue_conc_pop( bit_queue_conc *bq, int obj );
unsigned long check_bit_obj_present_conc( bit_queue_conc *bq, int obj );
int bit_queue_conc_has_items(bit_queue_conc *bq);
int empty_bit_queue_conc(bit_queue_conc *bq);
void destroy_bit_queue_conc(bit_queue_conc *bq);

//int queue_conc_pop_bit( bit_queue_conc *q, int *result );
//int bit_queue_conc_has_items(bit_queue_conc *q);


void print_bit_queue_conc(bit_queue_conc *bq, char *bit_queue_conc_file);






#endif
