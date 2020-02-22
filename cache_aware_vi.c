//
//  cache_aware_vi.c
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#include "cache_aware_vi.h"
#include <time.h>
#include <sys/time.h>
#include <omp.h>


double heat_epsilon_final = heat_epsilon_final_def;
double heat_epsilon_initial = heat_epsilon_initial_def;
extern char       gInputFileName[];
#define CLUSTERED


double cache_aware_vi(struct StateListNode *list, int MaxIter, int round, int component_size)
{
    world_t *w;
    double retVal = 0;
    double heat_epsilon_current = heat_epsilon_final;
    float iter_count = 0;
    unsigned long total_updates = 0, total_updates_iters = 0;
    
    double epsilon_partition_initial, epsilon_overall, time;
    clock_t compStartTime;
    struct timeval tInitial;
    struct timeval tFinal;
    double timeDay;

    open_logfile_stdout();
    //This will partition the list into parts (or 2 level parts) and coplete value Iteration.
#ifdef __TEST__
    print_back_list(list, component_size, "verify_list");
    //return 1;
#endif
    w = init_world(list, component_size, round);   //Divides the component into partitions. Allocating states per part. & level1 parts.
    initialize_partitions( w );
    
    if (w->part_level0_to_level1 != NULL)
        initialize_level1_partitions(w);

#ifdef __TEST__
    print_back_mdp(w, "verify_mdp_initialized");
    //return 1;
    print_back_mdp(w, "verify_mdp_translated_neg");
    //    return 1;
    print_back_deps(w, "verify_deps");
    //return 1;
    //    print_back_cache(w, "verify_cached");
#endif
    
    //resolve external dependencies.
    resolve_ext_deps(w);
    cache_dependencies_in_states(w);
    translate_all(w);
    compute_initial_partition_priorities( w );
    init_part_heap( w );
    reorder_states_within_partitions( w );

/*    heat_epsilon_current = heat_epsilon_initial;
    iter_count = 0;
    epsilon_partition = 10000000*heat_epsilon_final; //heat_epsilon_initial;
    epsilon_overall = 1000000*heat_epsilon_final;
    solve_using_prioritized_vi( w, epsilon_partition, epsilon_overall );
    wlog(1, "Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu\n", round, epsilon_partition, epsilon_overall, w->num_value_updates + w->num_value_updates_iters);
 */
    w->num_value_updates = 0; w->num_value_updates_iters = 0; w->new_partition_wash = 0;
    w->val_update_time = 0; w->val_update_iters_time = 0; w->num_value_updates_attempted =0; w->num_value_update_iters_attempted = 0;
    
    epsilon_partition_initial = 10; //10; //heat_epsilon_initial;
    epsilon_overall = heat_epsilon_final;
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    
    compStartTime = clock();
    gettimeofday(&tInitial, NULL);

/*    retVal = value_iterate(w, 1000, 1000);
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, 500, 500);
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, 100, 100);
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, 50, 50);
    init_level1_part_queue(w);
    init_level0_bit_queue(w);*/
//    retVal = value_iterate(w, 10, 10);
//    init_level1_part_queue(w);
//    init_level0_bit_queue(w);
//    retVal = value_iterate(w, 1, 1);
//    init_level1_part_queue(w);
//    init_level0_bit_queue(w);
//    retVal = value_iterate(w, 0.1, 0.1);
//    init_level1_part_queue(w);
//    init_level0_bit_queue(w);
//    retVal = value_iterate(w, 0.01, 0.01);
//    init_level1_part_queue(w);
//    init_level0_bit_queue(w);
//    retVal = value_iterate(w, 0.0001, 0.0001);
//    init_level1_part_queue(w);
//    init_level0_bit_queue(w);
    retVal = value_iterate(w, epsilon_partition_initial, epsilon_overall);
//    solve_using_prioritized_vi( w, epsilon_partition, epsilon_overall );
//    solve_using_prioritized_vi( w, epsilon_partition, epsilon_overall );

    gettimeofday(&tFinal, NULL);
    timeDay = (tFinal.tv_sec - tInitial.tv_sec) + (float)(tFinal.tv_usec - tInitial.tv_usec) / 1000000;

    time = (float)(clock()-compStartTime)/CLOCKS_PER_SEC;

    printf("Actual Value_iterate function in: %f secs. As per day: %f secs\n", time, timeDay);
    printf("Update time taken = %f; Update iters time taken = %f secs\n", w->val_update_time, w->val_update_iters_time);

/*    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, epsilon_partition, epsilon_overall);
*/
//    solve_using_prioritized_vi( w, epsilon_partition, epsilon_overall );
    wlog(1, "Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu\n", round, epsilon_partition_initial, epsilon_overall, w->num_value_updates + w->num_value_updates_iters);
    wlog(1, "Number of new partition washes=%lu, number of updates =%lu, number of update_iters=%lu, num attempt_updates=%lu, numattempted update_iters=%lu\n", w->new_partition_wash, w->num_value_updates, w->num_value_updates_iters, w->num_value_updates_attempted, w->num_value_update_iters_attempted);
    
/*    while (epsilon_overall > heat_epsilon_final)
    {
        
        heap_clear(w->part_heap);
        compute_initial_partition_priorities(w);
        init_part_heap(w);
        wlog(1, "Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu\n", round, epsilon_partition, epsilon_overall, w->num_value_updates + w->num_value_updates_iters);
        total_updates += w->num_value_updates + w->num_value_updates_iters;
        total_updates_iters += w->num_value_updates_iters;
        w->num_value_updates = 0;w->num_value_updates_iters=0;
        epsilon_partition = epsilon_partition/10;
        epsilon_overall = epsilon_overall/10;
    }*/
    
/*
    while (epsilon_overall > heat_epsilon_final)
    {
        init_level1_part_queue(w);
        init_level0_bit_queue(w);
        retVal = value_iterate(w, epsilon_partition, epsilon_overall);
        wlog(1, "Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu\n", round, epsilon_partition, epsilon_overall, w->num_value_updates + w->num_value_updates_iters);
        total_updates += w->num_value_updates + w->num_value_updates_iters;
        total_updates_iters += w->num_value_updates_iters;
        w->num_value_updates = 0;w->num_value_updates_iters=0;
        epsilon_partition = epsilon_partition/10;
        epsilon_overall = epsilon_overall/10;
    }
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, epsilon_partition, epsilon_overall);
    wlog(1, "Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu\n", round, epsilon_partition, epsilon_overall, w->num_value_updates + w->num_value_updates_iters);
    total_updates += w->num_value_updates + w->num_value_updates_iters;
    total_updates_iters += w->num_value_updates_iters;
    w->num_value_updates = 0;w->num_value_updates_iters=0;
  */
    wlog(1, "Final Number of Backups for round-%d with ep_part=%6f, ep_overall=%6f:\t%lu. Total_iter backups =%lu\n", round, epsilon_partition_initial, epsilon_overall, total_updates, total_updates_iters);

#ifndef CLUSTERED
    save_resulting_vector(w, "cached_output", round, component_size);
#else
    save_resulting_vector(w, "cached_output_clust_ann", round, component_size);
#endif
/*
    while (heat_epsilon_current > heat_epsilon_final)
    {
        init_level1_part_queue(w);
        init_level0_bit_queue(w);
        retVal = value_iterate(w, heat_epsilon_current);
        wlog(1, "Number of Backups for round-%d with heat=%6f:\t%lu\n", round, heat_epsilon_current, w->num_value_updates + w->num_value_updates_iters);
        total_updates += w->num_value_updates + w->num_value_updates_iters;
        total_updates_iters += w->num_value_updates_iters;
        w->num_value_updates = 0;w->num_value_updates_iters=0;
//        if (iter_count < 3)
            iter_count++;
  //      else
    //        iter_count += 0.5;
        
        heat_epsilon_current = heat_epsilon_initial * linear(iter_count); //gauss(iter_count);
        if (heat_epsilon_current < heat_epsilon_final)
            heat_epsilon_current = heat_epsilon_final;
    }
    init_level1_part_queue(w);
    init_level0_bit_queue(w);
    retVal = value_iterate(w, heat_epsilon_current);
    wlog(1, "Number of Backups for round-%d with heat=%6f: %lu\n", round, heat_epsilon_current, w->num_value_updates + w->num_value_updates_iters);
    total_updates += w->num_value_updates + w->num_value_updates_iters;
    total_updates_iters += w->num_value_updates_iters;
    w->num_value_updates = 0;w->num_value_updates_iters=0;

    wlog(1, "Final Number of Backups for round-%d with heat=%6f: %lu. Total_iter backups =%lu\n", round, heat_epsilon_current, total_updates, total_updates_iters);
 */
    return retVal;
    //return 0;
}

double linear(double x)
{
    double den = pow(2, x);
    return 1/den;
}

double gauss(double x)
{
    double sigmasq = GAUSS_CONST*GAUSS_CONST;
    double power_val = -1*(x*x)/sigmasq;
    return exp(power_val);
}


world_t *init_world(struct StateListNode *list, int component_size, int round)
{
    world_t *w;
    int l_part, st_index;
    
    clock_t     compStartTime = 0;
    double time = 0;
    //Define Threshold sizes for partitions.
    w = (world_t *)malloc( sizeof( world_t ) );
    w->num_global_states = component_size;
    w->num_global_parts = component_size/(int)PART_SIZE;
    
    if (component_size % PART_SIZE > 0)
        w->num_global_parts += 1;
    w->num_global_parts += 1;
    
    w->parts = (part_t *)malloc( sizeof(part_t) * w->num_global_parts );
    if ( w->parts == NULL ) {
        wlog( 1, "Could not allocate partition array!\n" );
        exit( 0 );
    }
    memset( w->parts, 0, sizeof(part_t) * w->num_global_parts );
    
    w->state_to_partnum = (int *)malloc( sizeof(int) * w->num_global_states );
    if ( w->state_to_partnum == NULL ) {
        wlog( 1, "Error allocating state_to_partnum!\n" );
        exit( 0 );
    }
    for (st_index = 0; st_index < w->num_global_states; st_index++)
    {
        w->state_to_partnum[st_index] = -1;         //Initialize states to a non-existent part
    }
    
    w->gsi_to_lsi = (int *)malloc( sizeof(int) * w->num_global_states );
    if ( w->gsi_to_lsi == NULL ) {
        wlog( 1, "Error allocating gsi_to_lsi!\n" );
        exit( 0 );
    }
    
    compStartTime = clock();
#ifndef CLUSTERED
    assign_state_to_part_num(list, w);
#else
    assign_state_to_part_cluster(list, w, round);
#endif
    time = (float)(clock()-compStartTime)/CLOCKS_PER_SEC;
    printf("Partitioned in: %f secs\n", time);
    
    for (l_part = 0; l_part < w->num_global_parts; l_part++)
    {
        w->parts[ l_part ].states = (state_t *)malloc( sizeof(state_t)*w->parts[l_part].num_states);
        w->parts[ l_part ].states_ind = (int *)malloc(sizeof(int) * w->parts[l_part].num_states);
        if ( w->parts[ l_part ].states == NULL ) {
            wlog( 1, "Error allocating states!\n" );
            exit( 0 );
        }
        memset( w->parts[ l_part ].states, 0, sizeof(state_t)*w->parts[l_part].num_states );
        memset( w->parts[ l_part ].states_ind, 0, sizeof(int)*w->parts[l_part].num_states );
        w->parts[l_part].values.elts = (double *)malloc(sizeof(double)*w->parts[l_part].num_states );
        w->parts[l_part].values.nelts = w->parts[l_part].num_states;
        w->parts[l_part].convergence_factor = -1;       //initialize convergence to high initial value.
    }
    w->total_int_deps = 0; w->total_ext_deps = 0;
    traverse_comp_form_parts(list, w, round);
    wlog(1, "Total internal dependents = %lu; Ext = %lu\n", w->total_int_deps, w->total_ext_deps);
    form_level1_parts(w);
    //Create all the queues needed to mantain states/parts/level1 parts to be processed during VI.
    
    //Number of parts in each level1 part. //Two params are number of items and the max value of the item.
    w->part_queue = queue_create(NUM_PARTS_IN_LEVEL1 + 1, w->num_global_parts);
    if ( w->part_queue == NULL ) {
        wlog( 1, "Error creating queue!\n" );
        exit( 0 );
    }
    w->part_level1_queue = queue_create(w->num_level1_parts, w->num_level1_parts);
    if ( w->part_level1_queue == NULL ) {
        wlog( 1, "Error creating queue!\n" );
        exit( 0 );
    }
    
    /* Create our local priority queue. This is done with a heap. */
    w->part_heap = heap_create( w->num_global_parts, part_cmp_func,
                               part_swap_func, part_add_func, w );
    if ( w->part_heap == NULL ) {
        wlog( 1, "Error creating heap!\n" );
        exit( 0 );
    }

    //init_level1_part_queue(w);
    w->part_level0_bit_queue = create_bit_queue(w->num_global_parts);
    w->part_level0_processing_bit_queue = create_bit_queue(w->num_global_parts);
    w->part_level0_waiting_bitq = create_bit_queue(w->num_global_parts);
    //w->processor_busy_bitq = create_bit_queue(omp_get_num_threads());
    w->leader_thread = 0;
    if ( w->part_level0_bit_queue == NULL || w->part_level0_processing_bit_queue == NULL || w->part_level0_waiting_bitq == NULL)// || w->processor_busy_bitq == NULL) 
    {
        wlog( 1, "Error creating bit queue!\n" );
        exit( 0 );
    }
    //init_level0_bit_queue(w);
    return w;
}

void init_level1_part_queue( world_t *w )
{
    int level1_part;
    for ( level1_part=0; level1_part<w->num_level1_parts; level1_part++ )
    {
        queue_add( w->part_level1_queue, level1_part );
    }
}
void init_level0_bit_queue(world_t *w)
{
    int l_part_num;
    for ( l_part_num=0; l_part_num<w->num_global_parts; l_part_num++ )
    {
        queue_add_bit( w->part_level0_bit_queue, l_part_num );
    }
}



void traverse_comp_form_parts(struct StateListNode *list, world_t *w, int round)
{
    struct StateNode *state = NULL;
    struct StateListNode     *stateListNode;
    
    for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
    {
        state = stateListNode->Node;
        copy_state_to_part(state, w, round);
    }
}

void create_ext_state_val_deps(world_t *w, int part_num, int l_state)
{
    double *external_deps = NULL;
    val_t*** external_state_vals = NULL;
    state_t *st = &(w->parts[part_num].states[l_state]);
    int a = 0;
    int nacts = st->num_actions;
    external_deps = (double *)malloc(sizeof(double) * nacts);
    external_state_vals = (val_t ***)malloc(sizeof(val_t **) * nacts);        //Commented out for eff
    if ( external_deps == NULL || external_state_vals == NULL ) {
        wlog( 1 , "Error allocating external_deps cache!\n" );
        exit( 0 );
    }
    /* set up this state's information */
    st->external_dep_vals = external_deps;
    st->external_state_vals = external_state_vals;
    for (a = 0; a < nacts; a++)
    {
        if (st->tps[a].ext_deps > 0)
            st->external_state_vals[a] = (val_t **)malloc(sizeof(val_t *) * st->tps[ a ].ext_deps);
        else
            st->external_state_vals[a] = NULL;
    }
}

//Need a way to assign co-herent partitions.
void assign_state_to_part_num(struct StateListNode *list, world_t *w)
{
    struct StateNode *state = NULL;
    struct StateListNode     *stateListNode;
    int part_num = 0;
    int comp_state_num = 0;
    
    for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
    {
        state = stateListNode->Node;
        state->comp_state_num = comp_state_num++;       //Assigning index to state in comp. starting at index 0
        if (w->parts[part_num].num_states == PART_SIZE)
            part_num++;
        w->state_to_partnum[state->comp_state_num] = part_num;     //comp_state_num is num for this component?- VERIFY-ANUJ
        w->gsi_to_lsi[state->comp_state_num] = w->parts[part_num].num_states;
        w->parts[part_num].num_states++;
    }
    wlog(1, "Actual total number of Parts=%d\n", part_num);
    if ( (comp_state_num != w->num_global_states) && (part_num != w->num_global_parts - 1) )
    {
        wlog(1, "Something wrong with counting states and parts.");
        exit(1);
    }
}

#define PROB_TRANS_THRESH 0.2
#define CUMM_PROB_THRESH 1.0
#define MIN_NBRS 2
#define MAX_NBRS 3
#define NBR_VAL 0.3
 void assign_state_to_part_cluster(struct StateListNode *list, world_t *w, int round)
 {
     struct StateNode *state = NULL;
     struct StateListNode     *stateListNode;
     int part_num = 0;
     int comp_state_num = 0, cur_state, it_present;
     queue* states_queue;
     struct StateNode **states_array;
     int states_in_cur_part = 0, num_nbrs_added = 0, nbr_is_val = 0;
     struct ActionListNode *actionListNode = NULL;
     struct ActionNode *actionNode = NULL;
     struct StateDistribution *nextState = NULL;
     double cumm_action_prob = 0;

 
     states_array = (struct StateNode **)malloc( sizeof(struct StateNode *) * w->num_global_states);
     comp_state_num = 0;
     for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
     {
         state = stateListNode->Node;
         state->comp_state_num = comp_state_num++;       //Assigning index to state in comp. starting at index 0
         states_array[state->comp_state_num] = state;
     }
     states_queue = queue_create(w->num_global_states, w->num_global_states);
     
     //Initialize number of states in each partition.
     part_num = 0; states_in_cur_part = 0;
     
     for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
     {
         state = stateListNode->Node;
         comp_state_num = state->comp_state_num;       //Get index of the state.
         if (w->state_to_partnum[comp_state_num] != -1)     //State already assigned to some part.
             continue;
         queue_add(states_queue, comp_state_num);
         //w->parts[part_num].num_states = 0;             //This doesn't seem right. The num_states is initialized when allocated.
         while (queue_has_items(states_queue) && states_in_cur_part < PART_SIZE)
         {
             it_present = queue_pop(states_queue, &cur_state);
             w->state_to_partnum[cur_state] = part_num;     //Assign this state number to current partition.
             states_in_cur_part++;
             w->parts[part_num].num_states++;

             if (states_in_cur_part < PART_SIZE)        //Look at neighbors only if partition is not yet full.
             {
                 state = states_array[cur_state];       //retreive the state just assigned to this part and visit nbrs
                 //Visit Neighbors and add all neighbors to the queue.
                 for (actionListNode = state->Action; actionListNode; actionListNode = actionListNode->Next)
                 {
                     actionNode = actionListNode->Node;
                     //validate_actionNode(actionNode);
                     if (actionNode->Dominated == 1)
                         continue;
                     cumm_action_prob = 0;
                     num_nbrs_added=0;
                     for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
                     {
                         if (nextState != NULL)
                         {
                             if (nextState->State->component_id == round)   //Looking at nbrs only in the same SCC.
                             {
                                 comp_state_num = nextState->State->comp_state_num;
                                 if (comp_state_num >= w->num_global_states || comp_state_num < 0)
                                 {
                                     printf("\nBad Error!!!!. compStateNum is somehow not in SCC size.\n");
                                     printf("Happening with State: %d\n", actionNode->StateNo);
                                     exit(1);
                                 }
                                 else if (w->state_to_partnum[comp_state_num] == -1)      //Only adding those nbrs to the q that have not been assigned already
                                 {
                                     if (nextState->Prob >= (float)PROB_TRANS_THRESH)            //Add only those neighbors to the cluster that have high prob of transition.
                                     {
                                         nbr_is_val = 1;
                                         if (cumm_action_prob > 0)
                                         {
                                             if ((nextState->Prob/cumm_action_prob) > (double)NBR_VAL)
                                             {nbr_is_val = 1;}
                                             else
                                             {nbr_is_val = 0;}
                                         }
                                         if (nbr_is_val)
                                         {
                                             queue_add(states_queue, comp_state_num);
                                             num_nbrs_added++;
                                             cumm_action_prob += nextState->Prob;
                                             if ( (cumm_action_prob >= (double)CUMM_PROB_THRESH) || (num_nbrs_added >= (int)MAX_NBRS) )
                                                 break;             //Total probability for imm nbrs for an action reached.
                                         }
                                     }
                                 }  //end else if - nbr was not yet assigned to part.
                             }      //Nbr in same scc
                         }  //end if - Nbr was not null.
                     }      //end for all possible transitions for this action
                 }  //end for all possible actions for this state.
             }   //End if - Look at neighbors only if partition is not yet full.
         }      //End while
         if (states_in_cur_part == PART_SIZE)       //The partition is full to its capacity so now new partitoin will start filling up.
         {
             empty_queue(states_queue);     //Empty the queue as no more states can be added to the partition just filled up.
             part_num++;        //Now we will start filling up next partition.
             states_in_cur_part = 0;        //Initialize number of states in next partition.
         }
         
     }      //End for statelist.
     wlog(1, "Actual total number of Parts=%d\n", part_num);

     free(states_array);
     destroy_queue(states_queue);
     //destroy states_queue. Implement the destroy function in queue.
 }

void assign_part_to_level1_part(world_t *w)
{
    int num_part = 0, curr_part;
    int level_1_part = 0;
    for (num_part = 0; num_part < w->num_global_parts; num_part++)
    {
        if (w->level1_parts[level_1_part].num_sub_parts == NUM_PARTS_IN_LEVEL1)
            level_1_part++;
        curr_part = w->level1_parts[level_1_part].num_sub_parts;
        w->part_level0_to_level1[num_part] = level_1_part;
        w->level1_parts[level_1_part].sub_parts[curr_part] = num_part;
        w->level1_parts[level_1_part].num_sub_parts++;
    }
}

void form_level1_parts(world_t *w)
{
    int i;
    w->part_level0_to_level1 = (int *)malloc( sizeof(int) * w->num_global_parts);
    if ( w->part_level0_to_level1 == NULL )
    {
        wlog( 1, "Error allocating part_level0_to_level1!\n" );
        exit( 0 );
    }
    memset(w->part_level0_to_level1, 0, sizeof(int) * w->num_global_parts);
    
    w->num_level1_parts = w->num_global_parts/NUM_PARTS_IN_LEVEL1;
    if (w->num_global_parts % NUM_PARTS_IN_LEVEL1 > 0)
        w->num_level1_parts += 1;
    w->level1_parts = malloc( sizeof(level1_part_t) * w->num_level1_parts);
    memset(w->level1_parts, 0, sizeof(level1_part_t) * w->num_level1_parts);
    for (i = 0; i < w->num_level1_parts; i++)
    {
        w->level1_parts[i].sub_parts = malloc(sizeof(int) * NUM_PARTS_IN_LEVEL1);
        memset(w->level1_parts[i].sub_parts, 0, sizeof(int) * NUM_PARTS_IN_LEVEL1);
        w->level1_parts[i].num_sub_parts = 0;
    }
    assign_part_to_level1_part(w);
}

void copy_state_to_part(struct StateNode *state, world_t *w, int round)
{
    //Dest structures
    trans_t *tt_dest = NULL;
    state_t *st_dest = NULL;
    int ndeps = 0, action_num = 0, end_dep, beg_dep;
    entry_t *et_dest;
    
    //Source structures
    struct ActionListNode *actionListNode;
    struct ActionNode *actionNode;
    struct StateDistribution *nextState = NULL;

    int part_num = w->state_to_partnum[state->comp_state_num];
    int state_index_curr_part = w->parts[part_num].cur_local_state++;
    st_dest = &(w->parts[part_num].states[state_index_curr_part]);
    st_dest->global_state_index = state->comp_state_num;
    w->gsi_to_lsi[state->comp_state_num] = state_index_curr_part;
    st_dest->over_mdp_state_index = state->StateNo;     //Mainly for debugging.
    
    //Copy all the actions from the state and all the possible next states for each action.
    tt_dest = (trans_t *)malloc( sizeof(trans_t) * state->num_actions );
    if ( tt_dest == NULL ) {
        wlog( 1 , "Error allocating transition!\n" );
        exit( 0 );
    }
    st_dest->tps = tt_dest;
    st_dest->num_actions = state->num_actions;
    st_dest->Terminal = state->Terminal;
    if (state == Goal)
        st_dest->goal = 1;
    action_num = 0;

    for (actionListNode = state->Action; actionListNode; actionListNode = actionListNode->Next)
    {
        actionNode = actionListNode->Node;
        if ( actionNode->Dominated )
            continue;
        ndeps = 0;
        //count number of deps.
        for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
            ndeps++;
        if (ndeps == 0)
        {
            st_dest->tps[action_num].entries = NULL;
            st_dest->tps[action_num].int_deps = 0;
            st_dest->tps[action_num].ext_deps = 0;
            action_num++;
            continue;
        }
        et_dest = (entry_t *)malloc( sizeof(entry_t) * ndeps );
        memset(et_dest, 0, sizeof(entry_t) * ndeps);
        st_dest->tps[action_num].entries = et_dest;
        st_dest->tps[action_num].reward = actionNode->Cost;
        end_dep = ndeps-1; beg_dep = 0;
        for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
        {
            //First find out if the nextState even belongs to this world. If not, then col should just point to an external state.
            //The value of the external state should be stored in external_state_vals[action][dep]. --ANUJ
            if (nextState->State->component_id != round)
            {
                et_dest[end_dep].entry = nextState->Prob;
                et_dest[end_dep].col = -1;
                et_dest[end_dep].ext_st_ptr = nextState->State;
                end_dep--;
            }
            else if (w->state_to_partnum[nextState->State->comp_state_num] == part_num)
            {
                et_dest[beg_dep].entry = nextState->Prob;            //Change this so that we can order as int deps and ext deps.
                et_dest[beg_dep].col = nextState->State->comp_state_num;
                et_dest[beg_dep].ext_st_ptr = NULL;
                beg_dep++;
            }
            else
            {
                et_dest[end_dep].entry = nextState->Prob;
                et_dest[end_dep].col = nextState->State->comp_state_num;
                et_dest[end_dep].ext_st_ptr = NULL;
                end_dep--;
            }
        }
        st_dest->tps[action_num].int_deps = beg_dep;
        st_dest->tps[action_num].ext_deps = ndeps - beg_dep;     //This is not correct. Need to separate int and ext deps.
        
        w->total_int_deps += st_dest->tps[action_num].int_deps;
        w->total_ext_deps += st_dest->tps[action_num].ext_deps;
        action_num++;
    }
    w->parts[part_num].values.elts[state_index_curr_part] = state->fWeight;
}

void initialize_partitions( world_t *w )
{
    int l_part;
    
    for ( l_part=0; l_part< w->num_global_parts; l_part++ )
    {
        /* create the dependency hashes for this partition */
        w->parts[ l_part ].my_local_dependents = med_hash_create( 4 );
        w->parts[ l_part ].my_global_dependents = med_hash_create( 4 );
        w->parts[ l_part ].my_ext_parts_states = med_hash_create( 4 );
        w->parts[ l_part ].heat_links = med_hash_create( 4 );
        
        if (w->parts[ l_part ].my_local_dependents == NULL ||
            w->parts[ l_part ].my_ext_parts_states  == NULL ||
            w->parts[ l_part ].my_global_dependents == NULL ||
            w->parts[ l_part ].heat_links == NULL)
        {
            wlog( 1, "Error creating dependency hashes for part %d!\n", l_part );
            exit( 0 );
        }
        
        /* create all of the matrix stuff necessary for our library */
        //part_matrix_init( w, l_part );
    }
}

void initialize_level1_partitions( world_t *w )
{
    int level1_part;
    
    if (w->part_level0_to_level1 == NULL)
        return;
    
    for (level1_part = 0; level1_part < w->num_level1_parts; level1_part++)
    {
        /* create the dependency hashes for this partition */
        w->level1_parts[level1_part].my_local_dependents = med_hash_create( 4 );
        if (w->level1_parts[level1_part].my_local_dependents == NULL)
        {
            wlog( 1, "Error creating dependency hashes for level1 part %d!\n", level1_part );
            exit( 0 );
        }
    }
}


int state_to_partnum( world_t *w, int state_t )
{
    return w->state_to_partnum[state_t];
}

int gsi_to_lsi(world_t *w, int global_index)
{
    return w->gsi_to_lsi[global_index];
}

void resolve_ext_deps(world_t *w)
{
    int l_start_state, state_cnt;
    int l_start_part, level1_start_part = 0, level1_end_part = 0;
    int g_end_state, l_end_part, l_end_state;
    int action, next_st_index, ext_dep_cnt;
    trans_t *t;
    state_t *st;
    
    for (l_start_part = 0; l_start_part < w->num_global_parts; l_start_part++ )
    {
        state_cnt = w->parts[ l_start_part ].num_states;        //Base states
        for ( l_start_state = 0; l_start_state < state_cnt; l_start_state++ )
        {
            st = &( w->parts[ l_start_part ].states[ l_start_state ] );
            for (action=0; action < st->num_actions; action++)
            {
                t = &( st->tps[ action ] );
                ext_dep_cnt = t->ext_deps;
                for ( next_st_index=0; next_st_index < ext_dep_cnt; next_st_index++ )
                {
                    g_end_state = t->entries[ next_st_index + t->int_deps ].col;
                    if (g_end_state != -1)  //End state is in world. If out of world, no dep resolution reqd. Just cache value.
                    {
                        l_end_part = state_to_partnum( w, g_end_state );//where is g_end_state. What partition it's in.
                        if ( l_start_part == l_end_part )
                        {
                            continue;
                        }
                        /* g_end_state isn't in start partition.
                         g_end_part needs to know that if it changes, start_part needs to be re-evaluated.*/
                        level1_start_part = w->part_level0_to_level1[l_start_part];
                        level1_end_part = w->part_level0_to_level1[l_end_part];
                        add_dep( w, l_start_part, l_start_state, l_end_part, level1_start_part, level1_end_part );

                        //Every partition also mantains a list of ext partitions it transitions to.
                        //In each of the external partitions will be list of states that can be transitioned into.
                        //This is used to load values of external states during vi in cache efficient manner.
                        l_end_state = gsi_to_lsi(w, g_end_state);
                        add_part_ext_dep_states(w, l_start_part, l_end_state, l_end_part);
                    }
                }       //End of each dependent for an action.
            }     //End of each action in a state
            create_ext_state_val_deps(w, l_start_part, l_start_state);
        }       //End of looping over each state in a partitoin
    }
}

void add_dep( world_t *w,
             int l_start_part, int l_start_state,
             int l_end_part, int level1_start_part, int level1_end_part )
{
    
    val_t val0;
    val0.i = 0;
    int rval = 0;
    
    /* g_start_state (which resides in l_start_part) depends on
     g_end_state (which resides in l_end_part).
     so, l_end_part needs to have a dependent which is l_start_part. */
    rval = med_hash_add_float( w->parts[ l_start_part ].heat_links, l_end_part, 0 );
    if (rval > MH_ADD_REPLACED)
        wlog(1, "Problems adding to heat links of start part. Rval=%d\n", rval);

    if (level1_start_part == level1_end_part)       //Within same level1 partition so just add to deps of local part.
    {
        //        med_hash_set_add( w->parts[ l_end_part ].my_local_dependents,
        //                         g_start_part, l_start_state );
        rval = med_hash_set_add( w->parts[ l_end_part ].my_local_dependents,
                            l_start_part, l_start_state );
        if (rval > MH_ADD_REPLACED)
            wlog(1, "Problems adding to local deps of base parts. Rval=%d\n", rval);
    }
    
    else
    {
        //Add the base partition to global dependents as it is in a different higher level partition
        //        med_hash_set_add( w->parts[ l_end_part ].my_global_dependents,
        //                         g_start_part, l_start_state );
        rval = med_hash_set_add( w->parts[ l_end_part ].my_global_dependents,
                            l_start_part, l_start_state);
        if (rval > MH_ADD_REPLACED)
            wlog(1, "Problems adding to global deps of base parts. Rval=%d\n", rval);
        
        
        //Add dependency of higher level partitions, so
        //the higher level partition can be scheduled if the dpendent changed.
        //        med_hash_set_add(w->level1_parts[level1_end_part].my_local_dependents, level1_start_part, g_start_part);
        rval = med_hash_add(w->level1_parts[level1_end_part].my_local_dependents,
                            level1_start_part, val0);
        if (rval > MH_ADD_REPLACED)
            wlog(1, "Problems adding to local deps of l1 parts. Rval=%d\n", rval);
    }
}


void add_part_ext_dep_states( world_t *w,
                             int l_start_part, int l_end_state, int l_end_part)
{
    med_hash_set_add( w->parts[ l_start_part ].my_ext_parts_states,
                     l_end_part, l_end_state );
}

void cache_dependencies_in_states( world_t *w )
{
    int l_start_state, state_cnt;
    int l_start_part;
    int g_end_state, l_end_part, l_end_state;
    int action, ext_dep, dep_cnt;
    trans_t *t;
    state_t *st;
    
    struct StateNode *ext_global_mdp_state;
    double val_out_of_world_state = 0;
    
    for( l_start_part = 0; l_start_part < w->num_global_parts; l_start_part++)
    {
        state_cnt = w->parts[ l_start_part ].num_states;
        for ( l_start_state = 0; l_start_state < state_cnt; l_start_state++ )
        {
            st = &( w->parts[ l_start_part ].states[ l_start_state ] );
            for (action=0; action < st->num_actions; action++)
            {
                t = &( st->tps[ action ] );
                dep_cnt = t->ext_deps;
                for ( ext_dep=0; ext_dep < dep_cnt; ext_dep++ )
                {
                    g_end_state = t->entries[ ext_dep + t->int_deps ].col;
                    if (g_end_state != -1)      //The external state is within this world.
                    {
                        l_end_part = state_to_partnum( w, g_end_state );
                        if ( l_start_part == l_end_part )
                        {
                            /* no further action required if it's in our partition. */
                            continue;
                        }
                        l_end_state = gsi_to_lsi(w, g_end_state);
                        add_cache_states(w, l_start_part, l_start_state, l_end_state, l_end_part,
                                         w->parts[l_start_part].states[l_start_state].external_state_vals[action], ext_dep);
                    }
                    else //The external state is outside the world, so just cache its final value directly.
                    {
                        ext_global_mdp_state = (struct StateNode*)t->entries[ext_dep + t->int_deps].ext_st_ptr;
                        val_out_of_world_state = ext_global_mdp_state->fWeight;    //Final value of the state outside the world. ANUJ
                        cache_val_state_out_of_world(w->parts[l_start_part].states[l_start_state].external_state_vals[action],
                                                     ext_dep, val_out_of_world_state);
                        
                    }
                }       //for all external deps of an action.
            }           //for all actions of a state.
        }               //for all states in a partition.
    }                   //for all partitions.
}

void add_cache_states(world_t *w,
                      int l_start_part,
                      int l_start_state, int l_end_state, int l_end_part, val_t **arrayValptrs, int indexVal )
{
    val_t *val_address;
    int r;
    val_t v2;
    med_hash_t *m2;
    
    med_hash_get( w->parts[ l_start_part ].my_ext_parts_states, l_end_part, &v2);
    m2 = (med_hash_t *)v2.vptr;
    r = med_hash_getp(m2, l_end_state, &val_address);
    if (r == MH_FOUND)
        arrayValptrs[indexVal] = val_address;       //The state, action array points to the value of the external state.
    else
        wlog(1, "Strange!!! just added Values in Cache not found!!!");
}

//The state to be cached is outside the world, so we just cache its value directly.
void cache_val_state_out_of_world(val_t **arrayValptrs, int indexVal, double ext_val)
{
    val_t *ext_final_val = (val_t *)malloc(sizeof(val_t));
    ext_final_val->d = ext_val;
    arrayValptrs[indexVal] = ext_final_val;
}

void translate_all(world_t *w)
{
    int l_part;
    
    for ( l_part=0; l_part< w->num_global_parts; l_part++ ) {
        translate_to_local_matrix( w, l_part );
    }
    
}
void translate_and_negate_all( world_t *w )
{
    int l_part;
    
    for ( l_part=0; l_part< w->num_global_parts; l_part++ ) {
        translate_to_local_matrix( w, l_part );
        negate_matrix( w, l_part );
    }
}
void translate_to_local_matrix( world_t *w, int l_part )
{
    int l_state, nacts, a, ndeps, i, tmp;
    state_t *st;
    
    for ( l_state=0; l_state < w->parts[l_part].num_states; l_state++ ) {
        st = &( w->parts[ l_part ].states[ l_state ] );
        nacts = st->num_actions;
        
        for ( a=0; a<nacts; a++ )
        {
            ndeps = st->tps[a].int_deps;
            
            for ( i=0; i<ndeps; i++ )
            {
                tmp = gsi_to_lsi( w, st->tps[a].entries[i].col );
                st->tps[a].entries[i].col = tmp;
            }
            
        }
    }
}

void negate_matrix( world_t *w, int l_part )
{
    int cnt, l_state, nd, nacts, a, i;
    state_t *st;
    
    cnt = w->parts[ l_part ].num_states;
    
    for ( l_state=0; l_state<cnt; l_state++ ) {
        
        st = &( w->parts[ l_part ].states[ l_state ] );
        nacts = st->num_actions;
        
        for ( a=0; a<nacts; a++ )
        {
            nd = st->tps[a].int_deps + st->tps[a].ext_deps;
            for ( i=0; i<nd; i++ ) {
                st->tps[a].entries[i].entry *= -1;
            }
        }
    }
    
}

/*
 * ----------------------------------------------------------------------------
 */

/* a and b need to be LOCAL partition indices! */

int part_cmp_func( int lp_a, int lp_b, void *vw ) {
    float a_heat, b_heat;
    
    /*  wlog( 1, "  comparing %d to %d: %.2f %.2f\n", */
    /*	   a, b, w->parts[a].heat, w->parts[b].heat ); */
    
    a_heat = ((world_t *)vw)->parts[ lp_a ].heat;
    b_heat = ((world_t *)vw)->parts[ lp_b ].heat;
    
    return a_heat > b_heat;
    
    /*   if ( a_heat < b_heat ) { */
    /*     return 1; */
    /*   } else if ( a_heat > b_heat ) { */
    /*     return -1; */
    /*   } */
    /*   return 0; */
}

void part_swap_func( int lp_a, int lp_b, void *vw ) {
    int tmp;
    world_t *w;
    
    w = (world_t *)vw;
    tmp = w->parts[ lp_a ].my_heap_num;
    w->parts[ lp_a ].my_heap_num = w->parts[ lp_b ].my_heap_num;
    w->parts[ lp_b ].my_heap_num = tmp;
}

void part_add_func( int lp_obj, int pos, void *vw ) {
    world_t *w;
    
    w = (world_t *)vw;
    w->parts[ lp_obj ].my_heap_num = pos;
}

void init_part_heap( world_t *w ) {
    int l_part_num;
    
    for ( l_part_num=0; l_part_num<w->num_global_parts; l_part_num++ ) {
        heap_add( w->part_heap, l_part_num );
    }
}

/*
 * ----------------------------------------------------------------------------
 */




void compute_initial_partition_priorities( world_t *w ) {
    int l_part, g_part, l_state;
    int state_cnt;
    double tmpheat, maxheat;
    
    l_part = -1;
    g_part = -1;
    for ( l_part=0; l_part< w->num_global_parts; l_part++ )
    {
        
        maxheat = 0;
        state_cnt = w->parts[ l_part ].num_states;
        
        for ( l_state = 0; l_state < state_cnt; l_state++ ) {
            tmpheat = get_heat( w, l_part, l_state );
            
            maxheat = maxheat > tmpheat ? maxheat:tmpheat;
        }
        
        /* we need to update the initial heat of this partition */
        w->parts[ l_part ].heat = maxheat;
        w->parts[ l_part ].primary_heat = maxheat;
        
    }
}

void reorder_states_within_partitions( world_t *w ) {
    int l_part;
    
    for ( l_part=0; l_part < w->num_global_parts; l_part++ ) {
        //reorder_no_reorder(w, l_part);
        reorder_states_within_partition( w, l_part );
    }
}

void reorder_no_reorder( world_t *w, int l_part ) {
    part_t *pp;
    int i, total;
    /*   int toadd; */
    
    pp = &( w->parts[ l_part ] );
    total = pp->num_states;
    
    /* allocate fun and interesting data structures */
    
    pp->variable_ordering = (int *)malloc( sizeof(int) * total );
    for ( i=0; i<total; i++ ) {
        pp->variable_ordering[i] = i;
    }
}

void reorder_states_within_partition( world_t *w, int l_part ) {
    part_t *pp;
    state_t *sp;
    int i, j, total, done_cnt, scnt, a;
    int *zero_deg_list, zd_cnt;
    int *deg_cnt, min_deg;
    int l_state, misses;
    /*   int toadd; */
    
    pp = &( w->parts[ l_part ] );
    total = pp->num_states;
    
    /* allocate fun and interesting data structures */
    
    pp->variable_ordering = (int *)malloc( sizeof(int) * total );
    zero_deg_list = (int *)malloc( sizeof(int) * total );
    deg_cnt = (int *)malloc( sizeof(float) * total );
    if ( zero_deg_list == NULL || pp->variable_ordering == NULL || deg_cnt == NULL ) {
        wlog( 1, "Whoa! Couldn't allocate reordering data structures!\n" );
        exit( 0 );
    }
    memset( pp->variable_ordering, 0, sizeof(int)*total );
    memset( zero_deg_list, 0, sizeof(int)*total );
    memset( deg_cnt, 0, sizeof(int)*total );
    
    /* initialize the degree counts */
    /* these are in-degrees! */
    
    for ( i=0; i<total; i++ ) {
        sp = &( pp->states[ i ] );
        
        for ( a=0; a<sp->num_actions; a++ ) {
            scnt = sp->tps[a].int_deps;
            
            for ( j=0; j<scnt; j++ ) {
                l_state = sp->tps[a].entries[j].col;
                /* 'kay. local state i depends on global state g_state. */
                /* 	toadd = (int)( (float)100.0 * sp->tps[a].entries[j].entry ); */
                /* 	deg_cnt[ gsi_to_lsi( w, g_state ) ] += toadd; */
                deg_cnt[ l_state ] += 1;
            }
        }
    }
    
    /* initialize the zero-degree states */
    
    zd_cnt = 0;
    for ( i=0; i<total; i++ ) {
        if ( deg_cnt[i] == 0 ) {
            zero_deg_list[zd_cnt] = i;
            zd_cnt++;
        }
    }
    
    /* main loop */
    
    misses = 0;
    done_cnt = 0;
    while ( done_cnt < total ) {
        
        /* pick a state with zero degree */
        if ( zd_cnt > 0 ) {
            zd_cnt--;
            i = zero_deg_list[zd_cnt];
            
        } else {
            /* we have a cycle. Pick the state with the lowest degree */
            min_deg = 1e9; /* arbitrary big number */
            i = -1;
            for ( j=0; j<total; j++ ) {
                /* note: some degree counts can become negative because we
                 force deg_cnt[i] to be zero! */
                if ( deg_cnt[j] > 0 && deg_cnt[j] < min_deg ) {
                    i = j;
                    min_deg = deg_cnt[j];
                }
            }
            
            if ( i == -1 ) {
                wlog( 1, "WARGH! No more nodes to be done!\n" );
            }
            
            /* the number of times we had to do a full walk over the
             states */
            misses++;
        }
        
        /* force this to be negative, otherwise we might pick it up again */
        deg_cnt[i] = -1;
        
        /* State i is the state with the lowest in-degree.  Having the
         lowest in-degree means that nothing depends on it, meaning that
         we want to process it *last*.  That's why we put it at the
         *back* of the final ordering.
         */
        pp->variable_ordering[ total-1 - done_cnt ] = i;
        /*    pp->variable_ordering[ done_cnt ] = i; */
        done_cnt++;
        
        /* subtract out edges to dependencies. */
        /* if the dependency node has zero degree after subtracting, add it
         to the queue */
        
        sp = &( pp->states[ i ] );
        for ( a=0; a<sp->num_actions; a++ ) {
            scnt = sp->tps[a].int_deps;
            
            for ( j=0; j<scnt; j++ ) {
                l_state = sp->tps[a].entries[j].col;
                /* 'kay. local state i depends on global state g_state. */
//                l_state = gsi_to_lsi( w, g_state );
                
                /* 	toadd = (int)( (float)100.0 * sp->tps[a].entries[j].entry ); */
                /* 	deg_cnt[ l_state ] -= toadd; */
                deg_cnt[ l_state ] -= 1;
                
                /* 	if ( toadd != 0 && deg_cnt[ l_state ] == 0 ) { */
                if ( deg_cnt[ l_state ] == 0 ) {
                    zero_deg_list[zd_cnt] = l_state;
                    zd_cnt++;
                }
                
            }
        }
    }
    
    /* sanity check: make sure each variable is used! */
    memset( zero_deg_list, 0, sizeof(int)*total );
    
    for ( i=0; i<total; i++ ) {
        zero_deg_list[ pp->variable_ordering[i] ] = 1;
    }
    
    for ( i=0; i<total; i++ ) {
        if ( zero_deg_list[i] != 1 ) {
            wlog( 1, "WHOA! BAD VARIABLE ORDERING -- PART %d!\n", l_part );
            for ( i=0; i<total; i++ ) {
                wlog( 1, "%d ", pp->variable_ordering[i] );
            }
            wlog( 1, "\n" );
            abort();
        }
    }
    
    /*   wlog( 1, "  %d: %d\n", l_part, misses ); */
    
    free( zero_deg_list );
    free( deg_cnt );
    
}

void save_resulting_vector( world_t *w, char *fn, int r, int component_size )
{
    FILE *fp;
    int l_part, g_state, l_state, cnt, i, len;
    state_t *st;
    double val;
    char *filename = NULL, *probName = NULL;
    
    if (fn != NULL)
    {
        filename = (char *)malloc(sizeof(char)*strlen(fn) + 80);
        sprintf(filename, "%s_%d_", fn, r);
        len = (int) strlen(filename);
        probName = strrchr(gInputFileName, '/');
        if (probName != NULL)
        {
            for (i=1; i < strlen(probName)-4; i++)
            {
                filename[len+i-1] = probName[i];
            }
            filename[len+i-1] = '\0';
        }
        else
        {
            for (i=0; i < strlen(gInputFileName)-4; i++)
            {
                filename[len+i] = gInputFileName[i];
            }
            filename[len+i] = '\0';
        }
    }
    
    if ( filename == NULL ) {
        fp = stdout;
    } else {
        fp = fopen( filename, "wb" );
        if ( fp == NULL ) {
            wlog( 1, "Error opening %s!\n", filename );
            return;
        }
    }
    
    fprintf(fp, "%d\n", component_size);
    for ( l_part = 0; l_part < w->num_global_parts; l_part++ )
    {
        cnt = w->parts[ l_part ].num_states;
        for ( l_state = 0; l_state < cnt; l_state++ ) {
            
            st = &( w->parts[ l_part ].states[ l_state ] );
            g_state = st->over_mdp_state_index;
            val = w->parts[ l_part ].values.elts[ l_state ];
            
            fprintf( fp, "%d %.4f\n", g_state, val );
        }
    }
    
    if ( filename != NULL ) {
        fclose( fp );
    }
    free(filename);
}

void save_resulting_list(struct StateListNode *list, char *fn, int r, int component_size)
{
    FILE *fp;
    double val;
    char *filename = NULL;
    struct StateNode *state = NULL;
    struct StateListNode     *stateListNode;
    
    int g_state;
    
    if (fn != NULL)
    {
        filename = (char *)malloc(sizeof(char)*strlen(fn) + 10);
        sprintf(filename, "%s_%d", fn, r);
    }
    
    if ( filename == NULL ) {
        fp = stdout;
    } else {
        fp = fopen( filename, "wb" );
        if ( fp == NULL ) {
            wlog( 1, "Error opening %s!\n", filename );
            return;
        }
    }
    
    fprintf(fp, "%d\n", component_size);
    for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
    {
        state = stateListNode->Node;
        g_state = state->StateNo;
        val = state->fWeight;
        fprintf( fp, "%d %.4f\n", g_state, val );
    }
    if ( filename != NULL ) {
        fclose( fp );
    }
    
    free(filename);
}






