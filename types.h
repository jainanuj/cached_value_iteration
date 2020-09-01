//
//  types.h
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#ifndef types_h
#define types_h
#include "med_hash.h"
#include "intqueue.h"
#include "intheap.h"
#include "intqueue_conc.h"

#define PART_SIZE  5000//5000//6100       //3
#define ARR_SIZE  1000000
#define NUM_PARTS_IN_LEVEL1 10000
#define NUM_PROCS 40


typedef struct vec_t {
    int nelts;
    double *elts;
} vec_t;

typedef struct entry_t {
    int col;
    double entry;
    void* ext_st_ptr;       //Pointer to a state if it is outside the world.
} entry_t;

typedef struct trans_t {
    /* this is the number of dependencies internal to the partition */
    unsigned short int_deps;
    /* this is the number of dependencies external to the partition */
    unsigned short ext_deps;
    entry_t *entries;
    double reward;
    int origin_global_state_index;     // FOR REVERSE graph.
} trans_t;

typedef struct state {
    trans_t *tps; /* the results of each action */
    
//    trans_t **reverse_actions_tps;       //For Reverse Graph
    int global_state_index;  /* to map l_state_t to g_state_t */
    int over_mdp_state_index;
    unsigned char num_actions;
    unsigned char num_reverse_actions;
    unsigned char current_rev_action;
    double *external_dep_vals;       //array for storing aggregate of all extrnal states to this state for each possible action.
    val_t ***external_state_vals;     //Pointer to value of external state stored in the partition. This points to state values cached with partition's ext_parts_states.
    int bestAction;
    int Terminal;
    int goal;
    
} state_t;
typedef struct arr_states_t {
    
    state_t *states;
    vec_t values;

} arr_states_t;

typedef struct part_t {
    
    double heat, primary_heat;
    double convergence_factor;
    int visits, washes, my_heap_num;
    int num_states;
    state_t *states;
    int *states_ind;
    /* we only use this variable while loading the MDP */
    int cur_local_state;
    int *variable_ordering;
    /* this partition has heat links to several other partitions.
     we don't distinguish between local and foreign here.
     this is just a map from integers (which are partition numbers)
     to prec_ts (which are the heats). */
    med_hash_t *heat_links;

    /* this is the matrix stuff we use */
    vec_t values;
    vec_t *rhs;
    vec_t *values_lower;        //lower bound of the value of the state.
    
//    matrix_t *cur_pol_matrix;
    
    /* this hash collects all of the partitions (LOCAL ONLY) that depend
     on this partition.  this hash maps partition numbers to hashes. */
    med_hash_t *my_local_dependents;
    med_hash_t *my_ext_parts_states;        //At the beginning of the iteration we go through all these ext parts and states within them and cache their values as the value doesn't change during iteration.
    med_hash_t *my_global_dependents;
    /* for visualization stuff */
//    char marked;
    
} part_t;

typedef struct level1_part_t {
    int *sub_parts;        //This is an array of part#s of sub parts belonging to this level1 part.
    int num_sub_parts;     //Number of sub_parts belonging to this level1 part.
    med_hash_t *my_local_dependents;
    double convergence_factor;
} level1_part_t;


typedef struct world_t {
    int num_global_parts;
    int num_global_states;
    int num_local_states;
    int num_level1_parts;
    
    part_t *parts;
    level1_part_t   *level1_parts;
    heap *part_heap;
    
    queue_conc *part_queue;
    queue_conc *part_level1_queue;
    
    bit_queue_conc *part_level0_bit_queue;
    bit_queue *part_level0_processing_bit_queue;
    bit_queue *part_level0_waiting_bitq;

//    bit_queue *terminal_bit_queue;
//    bit_queue *dead_bit_queue;
//    bit_queue *planningStates;
    
    int cur_part_sorting;
    unsigned long num_value_updates;
    unsigned long num_value_updates_attempted;
    unsigned long num_value_updates_iters;      //Counter for all cached updates.
    unsigned long num_value_update_iters_attempted;
    unsigned long new_partition_wash;
    unsigned long total_int_deps;
    unsigned long total_ext_deps;
    
    double val_update_time;
    double val_update_iters_time;
    double inProcessQTimeSpent;
    int processor_counter[NUM_PROCS];
    
    /* this is the number of partitions that we've processed. */
    int parts_processed;
    /* this is the number of value-iteration sweeps */
    int vi_sweeps;
    /* this is the number of policy-iteration sweeps */
    int pi_sweeps, max_pi_sweeps, pi_iters;
    /* this maps GLOBAL states to GLOBAL partnums! */
    int *state_to_partnum;
    //This maps level0 parts to level1 parts
    int *part_level0_to_level1;
    int *gsi_to_lsi;
    int processing_items;
    
} world_t;




#endif /* types_h */
