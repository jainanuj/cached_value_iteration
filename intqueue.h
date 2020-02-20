//
//  intqueue.h
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "logger.h"

#ifndef _PART_QUEUE_H
#define _PART_QUEUE_H

#define BIT_ARRAY_SIZE 31
#define DEBSEQ 0x077CB531
#define MAX_DEB_SEQ_SIZE 0xFFFFFFFF
#define DEB_SEQ_REM_WINDOW 27

static const int DEBRUIJNBITPOS[32] =
{
    0, 1, 28, 2, 29, 14, 24, 3,
    30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7,
    26, 12, 18, 6, 11, 5, 10, 9
};

/*#define queue_add queue_add_bit
#define queue_pop queue_pop_bit
#define queue_has_items bit_queue_has_items
#define queue_create create_bit_queue
#define queue bit_queue
*/

#define BITQ 1

typedef struct {
    unsigned long *bit_arrays;
    //    int maxitems;
    int max_bit_arrays;
    int num_items;
} bit_queue;

typedef struct {
  int *items;
  bit_queue* bitqueue;
  int numitems, maxitems;
  int start_item_ptr, end_item_ptr;
  double add_time;
  double pop_time;
} queue;


bit_queue* create_bit_queue( int maxitems );
int queue_add_bit( bit_queue *q, int obj );
//int queue_pop_bit( bit_queue *q, int *result );
//int bit_queue_has_items(bit_queue *q);
int bit_queue_pop( bit_queue *bq, int obj );
unsigned long check_bit_obj_present( bit_queue *bq, int obj );
int bit_queue_last_item(bit_queue *bq);

int least_bit(unsigned long map);


queue* queue_create( int maxitems, int max_val );
void destroy_queue(queue *q);
int queue_add( queue *q, int a );
int queue_pop( queue *q, int *result );
int check_obj_present_in_q( queue *q, int obj);
int queue_has_items(queue *q);
int empty_queue( queue *q);
int empty_bit_queue(bit_queue *bq);
int bit_queue_has_items(bit_queue *bq);

void print_bit_queue(bit_queue *bq, char *bit_queue_file);
void Or_bit_queue(bit_queue *bq_dest, bit_queue *bq_to_add);
void destroy_bit_queue(bit_queue *bq);






#endif
