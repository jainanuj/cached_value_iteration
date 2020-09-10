//
//  intqueue.c
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//



#include <sys/time.h>
#include <omp.h>
#include "intqueue_conc.h"

/* don't use floats with this function -- you'll run into precision
 problems!*/
/*double whenq( void ) {
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (double)(tv.tv_sec) + (double)(tv.tv_usec)*(double)(1e-6);
}
*/


//
// ----------------------------------------------------------------------------
//

queue_conc *queue_conc_create( int maxitems, int max_val )
{
    queue_conc *q;
    int power = 1;
    bf_conc *bq;
    
    q = (queue_conc *)malloc( sizeof(queue_conc) );
    if (q == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    q->maxitems = maxitems;
    power = 1;
    while (power < maxitems)
    {
        power *= 2;
    }
    q->queue_size = power;
    q->items = (unsigned int *)malloc( q->queue_size * sizeof(unsigned int) );
    printf("Requested size was: %d. Allocated: %d", maxitems, power);
    if (q->items == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    bq = create_bit_queue_conc(max_val);
    q->q_bf = bq;
    empty_queue_conc(q);
    return q;
}

void destroy_queue_conc(queue_conc *q)
{
    destroy_bf_conc(q->q_bf);
    free(q->items);
    free(q);
}


int empty_queue_conc( queue_conc *q)
{
    int i = 0;
    unsigned int val = 0;
    q->FRONT = 0;
    q->REAR = 0;
    //val consists of two parts. value, ref count. value is in the left two bytes and ref count in right two bytes.
    val = EMPTYVAL;
    val <<= INT_SIZE/2;
    for (i = 0; i < q->queue_size; i++)
    {
        q->items[i] = val;
    }
    empty_bf_conc(q->q_bf);
    q->add_time = 0.0;
    q->pop_time = 0.0;
    return 0;
}


//Enq at rear of the q.
int queue_conc_enq( queue_conc *q, int obj )
{
    unsigned int val, newVal, iVal, iRef;
    unsigned int rear;
    if (!bf_conc_add_bit(q->q_bf,  obj))       //If this obj is already present or added by some other thread, don't attempt to add it to q.
        return 0;
    while (1)
    {
        rear = q->REAR;
        val = q->items[rear % q->queue_size];
        if (rear != q->REAR)
            continue;
        if (rear == q->FRONT + q->maxitems)
        {
            printf("Q already has maxitems:%d!!!\n",q->maxitems);
            return 0;
        }
        iRef = val & 0x0000FFFF;
        iVal = val >> INT_SIZE/2;
        if (iRef == 0xFFFF)
            iRef = 0;
        newVal = obj; newVal <<= INT_SIZE/2; newVal |= (iRef + 1);
        if (iVal == EMPTYVAL)
        {
            if (CAS(&(q->items[rear % q->queue_size]), val, newVal ))
            {
                CAS(&(q->REAR), rear, rear+1);
                return 1;
            }
            else
                 CAS(&(q->REAR), rear, rear+1);     //Help the thread that actually added its newVal to q.
        }
    }
}


int queue_conc_deq( queue_conc *q, unsigned int *result )
{
    unsigned int val, iVal, iRef, front, newEmpty;
    *result = EMPTYVAL;
    while (1)
    {
        front = q->FRONT;
        val = q->items[front % q->queue_size];
        if (front != q->FRONT)
            continue;
        if (front == q->REAR)
        {
            printf("Was trying to deq. Q is already empty!!!");
            return 0;
        }
        iRef = val & 0x0000FFFF;
        iVal = val >> INT_SIZE/2;
        if (iRef == 0xFFFF)
            iRef = 0;
        newEmpty = EMPTYVAL; newEmpty <<= INT_SIZE/2; newEmpty |= (iRef + 1);
        if (iVal != EMPTYVAL)
        {
            if (CAS(&(q->items[front % q->queue_size]), val, newEmpty))
            {
                *result = iVal;
                bf_atomic_unset(q->q_bf, iVal);
                CAS(&(q->FRONT), front, front + 1);
                return 1;
            }
        }
        else
            CAS(&(q->FRONT), front, front + 1);
    }
}


unsigned int queue_conc_has_items(queue_conc *q)
{
    return (q->REAR - q->FRONT);
}

/*********----------------------------------------------------**************/

bf_conc *create_bit_queue_conc( int max_items)
{
    bf_conc *bq;
    int num_bit_arrays;
    bq = (bf_conc *)malloc(sizeof(bf_conc));
    if (bq == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    num_bit_arrays = (max_items / BIT_ARRAY_SIZE) + 1;
    bq->bit_arrays = (unsigned long *)malloc(sizeof(unsigned long) * (num_bit_arrays) );
    if (bq->bit_arrays == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(0);
    }
    bq->max_bit_arrays = num_bit_arrays;
    empty_bf_conc(bq);
    return bq;
}

int empty_bf_conc(bf_conc *bq)
{
    memset(bq->bit_arrays, 0, sizeof(unsigned long)*bq->max_bit_arrays);
    return 0;
}


int bf_conc_add_bit( bf_conc *bq, unsigned int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bitset = 0x1 << set_bit, currentVal;        //The number with the required bit set.
    currentVal = bq->bit_arrays[index_bit_array];
    if (currentVal & number_bitset)
        return 0;
    while (1)
    {
        currentVal = bq->bit_arrays[index_bit_array];
        if (!(currentVal & number_bitset) )
        {
            if (CAS(&(bq->bit_arrays[index_bit_array]), currentVal, currentVal|number_bitset ) )
                return 1;
            else
                continue;
        }
        else
            return 0;       //Somebody else set the same bit.
    }
}

int bf_atomic_unset(bf_conc *bq, int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bit_unset = 0x1 << set_bit, number_bit_unset_comp;        //The number with the required bit set.
    number_bit_unset_comp = (~number_bit_unset) & MAX_DEB_SEQ_SIZE;
    
#pragma omp atomic update
    bq->bit_arrays[index_bit_array] &= number_bit_unset_comp;
    return 1;
}


void destroy_bf_conc(bf_conc *bq)
{
    free(bq->bit_arrays);
    free (bq);
}


unsigned long check_bit_obj_present_conc( bf_conc *bq, int obj )
{
    int index_bit_array = obj/BIT_ARRAY_SIZE;       //index of the bit array to use.
    int set_bit = obj - (index_bit_array * BIT_ARRAY_SIZE);     //The index of bit to be set in the chosen bit array
    unsigned long number_bitset = 0x1 << set_bit;        //The number with the required bit set.
    unsigned long retVal = 0;
    if ( (set_bit > BIT_ARRAY_SIZE) || (index_bit_array >= bq->max_bit_arrays))
    {
        fprintf(stderr, "Bit manip problem!\n");
        exit(0);
    }
    //will return non-zero if the bit is already set else zero.
#pragma omp atomic read
    retVal = bq->bit_arrays[index_bit_array];
    return (retVal & number_bitset);
}
