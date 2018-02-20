//
//  cached_vi.c
//  TVI
//
//  Created by Anuj Jain on 11/29/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#include "cached_vi.h"

double heat_epsilon_partition;
double heat_epsilon_overall;
/*
double value_iterate( world_t *w, double heat_epsilon_current )
{
    int   level1_part;
    double tmp =0; //maxheat
    heat_epsilon_partition = heat_epsilon_current;
    heat_epsilon_overall = heat_epsilon_current;
    //maxheat = 0;
    while (level1_part_available_to_process(w))
    {
        level1_part = get_next_level1_part(w);
        tmp = value_iterate_level1_partition( w, level1_part );
        if (tmp > heat_epsilon_overall)
        {
            add_level1_parts_deps_for_eval(w, level1_part);
            //maxheat = tmp;
        }
    }
    return tmp;
}
*/
double value_iterate( world_t *w, double epsilon_partition, double epsilon_overall )
{
    int   level1_part;
    double tmp =0; //maxheat
    heat_epsilon_partition = epsilon_partition;
    heat_epsilon_overall = epsilon_overall;
    //maxheat = 0;
    while (level1_part_available_to_process(w))
    {
        level1_part = get_next_level1_part(w);
        tmp = value_iterate_level1_partition( w, level1_part );
        if (tmp > heat_epsilon_overall)
        {
            add_level1_parts_deps_for_eval(w, level1_part);
            //maxheat = tmp;
        }
    }
    return tmp;
}


double value_iterate_level1_partition( world_t *w, int level1_part )
{
    int i, l_part, next_level0_part;
    double  tmp, maxheat = 0;
    
    clear_level0_queue(w);
    for (i=0; i< w->level1_parts[level1_part].num_sub_parts; i++ )
    {
        l_part = w->level1_parts[level1_part].sub_parts[i];
        if (check_dirty(w, l_part) )
        {
            add_level0_queue(w, l_part);
            clear_level0_dirty_flag(w, l_part);
        }
    }
    while (part_available_to_process(w) )
    {
        next_level0_part = get_next_part(w);
        tmp = value_iterate_partition(w, next_level0_part);
        w->parts[next_level0_part].washes++;
        if (tmp > heat_epsilon_overall)
        {
            //Add local deps to queue. Mark global as dirty.
            add_level0_partition_deps_for_eval(w, next_level0_part);
            if (w->part_queue->numitems > w->level1_parts[level1_part].num_sub_parts )
                wlog(1, "storing too many items in level0 q. NumItems = %d\n",w->part_queue->numitems);
            maxheat = tmp;
        }
    }
    return maxheat;
}

double value_iterate_partition( world_t *w, int l_part )
{
    part_t *pp;
    int l_state, state_cnt, i;
    float max_heat, delta, part_internal_heat;
    med_hash_t *dep_part_hash;
    int numPartitionIters = 0;
    
    int store_state_index;
    
    int g_end_ext_partition, l_end_ext_state, index1 = 0, index2 = 0;
    val_t *val_state_action;
    /*   FILE *fp; */
    
    
    pp = &( w->parts[ l_part ] );
    state_cnt = pp->num_states;
    
    
    dep_part_hash = w->parts[l_part].my_ext_parts_states;
    //Iterate over all external states grouped by partitions they belong to.
    //Load their values from their respective partition arrays.
    while ( med_hash_hash_iterate( dep_part_hash, &index1, &index2,
                                  &g_end_ext_partition, &l_end_ext_state, &val_state_action ))
    {
        store_state_index = w->parts[g_end_ext_partition].states_ind[l_end_ext_state];
        val_state_action->d = w->all_states_store.values.elts[store_state_index]; //Setting the value of that ext state
    }
    max_heat = 0;
    //First iteration of the partition.
    for ( i = 0; i < state_cnt; i++ )
    {
        l_state = i;                            //pp->variable_ordering[i];
        delta = 0;
        store_state_index = w->parts[l_part].states_ind[l_state];
        if ( (w->all_states_store.states[store_state_index].Terminal != 1) && (w->all_states_store.states[store_state_index].Terminal !=5) )
            delta = value_update( w, l_part, l_state );
        max_heat = fabs( delta ) > max_heat ? fabs( delta ): max_heat;
    }
    
    if (max_heat > heat_epsilon_partition)
    {
        //This is equivalent to while(true) as we don't change max_heat in the while loop.
        //If max_heat == 0 we don't need to enter this loop as partition is already cold.
        while(max_heat > 0)
        {
            //part_internal_heat initialized to 0 at beginning of each iteration.
            //It attains value of max heat in that iteration and keeps on reducing with every iteration.
            //It signifies that we are making progress within the partition.
            part_internal_heat = 0;
            for ( i = 0; i < state_cnt; i++ )
            {
                l_state = i;                //pp->variable_ordering[i];
                delta = 0;
                store_state_index = w->parts[l_part].states_ind[l_state];
                if ( (w->all_states_store.states[store_state_index].Terminal != 1) && (w->all_states_store.states[store_state_index].Terminal !=5) )
                    delta = value_update_iters( w, l_part, l_state );
                part_internal_heat = fabs( delta ) > part_internal_heat ? fabs( delta ): part_internal_heat;
            }
            w->parts[ l_part ].washes++;
            numPartitionIters++;
            if (part_internal_heat < heat_epsilon_partition) //excluding (numPartitionIters > MAX_ITERS_PP) ||
            {
                //if (numPartitionIters > 1)
                //if (numPartitionIters >= 20)
                //  if ( verbose ) { wlog( 1, "Partition %d was processed %d number of times. Part Internal Heat is: %.6f. Max heat to begin with is: %.6f \n", l_part, numPartitionIters, part_internal_heat, max_heat ); }
                break;
            }
        }
    }
    return max_heat;
}

int level1_part_available_to_process(world_t *w)
{
    return queue_has_items(w->part_level1_queue);
}
int get_next_level1_part(world_t *w)
{
    int next_level1_partition;
    queue_pop(w->part_level1_queue, &next_level1_partition);
    return next_level1_partition;
}
void add_level1_parts_deps_for_eval(world_t *w, int level1_part_changed)
{
    
    int level1_start_part;
    med_hash_t *dep_part_hash;
    int index1;
    val_t *v;
    
    dep_part_hash = w->level1_parts[level1_part_changed].my_local_dependents;
    index1 = 0;
    while ( med_hash_iterate( dep_part_hash, &index1, &level1_start_part, &v ) )
    {
        queue_add(w->part_level1_queue, level1_start_part);
    }
}

int clear_level0_queue(world_t *w)
{
    return empty_queue(w->part_queue);
}
unsigned long check_dirty(world_t *w, int l_part)
{
    return check_bit_obj_present(w->part_level0_bit_queue, l_part);
}
int add_level0_queue(world_t *w, int l_part)
{
    return queue_add(w->part_queue, l_part);
}
int clear_level0_dirty_flag(world_t *w, int l_part)
{
    return bit_queue_pop(w->part_level0_bit_queue, l_part);
}
int part_available_to_process(world_t *w)
{
    return queue_has_items(w->part_queue);
}
int get_next_part(world_t *w)
{
    int next_partition;
    queue_pop(w->part_queue, &next_partition);
    return next_partition;
}
void add_level0_partition_deps_for_eval(world_t *w, int l_part_changed)
{
    int l_start_part;
    med_hash_t *dep_part_hash;
    int index1;
    val_t *v;
    
    dep_part_hash = w->parts[ l_part_changed ].my_local_dependents;
    index1 = 0;
    while ( med_hash_iterate( dep_part_hash, &index1, &l_start_part, &v ) )
    {
        queue_add(w->part_queue, l_start_part);
    }
    
    dep_part_hash = w->parts[ l_part_changed ].my_global_dependents;
    index1 = 0;
    while ( med_hash_iterate( dep_part_hash, &index1, &l_start_part, &v ) )
    {
        set_dirty(w, l_start_part);
        //        queue_add(w->part_queue, l_start_part);
    }
}
int set_dirty(world_t *w, int l_part)
{
    return queue_add_bit(w->part_level0_bit_queue, l_part);
}

//Need to minimize cost (not maximize reward)
double value_update( world_t *w, int l_part, int l_state )
{
    int action, min_action, nacts;      // max_action
    int store_state_index;
    double tmp, min_value, cval;  //max_value
    
    store_state_index = w->parts[l_part].states_ind[l_state];
    if (w->all_states_store.states[store_state_index].goal == 1)
        return 0;
    
    cval = w->all_states_store.values.elts[store_state_index];
    
    min_action = 0;
    min_value = reward_or_value( w, l_part, l_state, 0 );
    
    /* remember that there is an action bias! */
    nacts = w->all_states_store.states[store_state_index].num_actions;
    for (action=1; action<nacts; action++)
    {
        tmp = reward_or_value( w, l_part, l_state, action );
        if ( tmp < min_value ) {
            min_value = tmp;
            min_action = action;
        }
    }
    w->all_states_store.states[store_state_index].bestAction = min_action;       //update best Action for this state.
    //Commenting Out - ANUJ - max_value can be -ve. This is when we have a cost to pay for the action.
    //  if ( max_value < 0 ) {
    //    fprintf( stderr, "WARGH!\n" );
    //    exit( 0 );
    //  }
    w->all_states_store.values.elts[store_state_index] = min_value;       //Update the V(s) for this state.
    w->num_value_updates++;
    if (cval < min_value)
        return min_value - cval;
    else
        return cval - min_value;
}
double reward_or_value( world_t *w, int l_part, int l_state, int a ) {
    double value, tmp;
    state_t *st;
    int store_state_index;
    
    store_state_index = w->parts[ l_part ].states_ind[ l_state ];
    st = &( w->all_states_store.states[store_state_index] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             w->parts[l_part].states_ind,
                             &(w->all_states_store.values) );
    
    /* add in external deps! */
    
    /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */
    
    tmp = get_remainder( w, l_part, l_state, a );     //Getting external dependencies.
    
    st->external_dep_vals[a] = tmp;      //Cache the values of the external deps in the state.
    value += tmp;
    
    /* we have to do this because we negated the A matrix! */
    //value = -value + st->tps[ a ].reward;
    value = value + st->tps[ a ].reward;        //Not negating.
    return value;
}
double get_remainder( world_t *w, int l_part, int l_state, int action ) {
    int i, dep_cnt;
    trans_t *tt;
    entry_t *ext_et;
    double val_hash2;
    int store_state_index;
    
    val_hash2 = 0;
    store_state_index = w->parts[ l_part ].states_ind[ l_state ];
    tt = &( w->all_states_store.states[store_state_index].tps[ action ] );
    dep_cnt = tt->ext_deps;
    ext_et = &( tt->entries[ tt->int_deps ] );    //* point to the first external entry
    
    for ( i=0; i<dep_cnt; i++ )
    {
        val_hash2 += ext_et[ i ].entry * (w->all_states_store.states[store_state_index].external_state_vals[action][i]->d);
#ifdef __TEST__
        if  (ext_et[ i ].col == -1)
        {
            struct StateNode *st_mdp = (struct StateNode*)(ext_et[i].ext_st_ptr);
            if (w->parts[l_part].states[l_state].external_state_vals[action][i]->d != st_mdp->fWeight)
            {
                wlog(1, "out of world_value doesn't match!!!");
                exit(1);
            }
        }
#endif
    }
    return val_hash2;
}
double value_update_iters( world_t *w, int l_part, int l_state )
{
    int action, min_action, nacts;     //max_action,
    double tmp, min_value, cval;        //max_value,
    int store_state_index;
    
    store_state_index = w->parts[ l_part ].states_ind[ l_state];
    cval = w->all_states_store.values.elts[store_state_index];
    
    
    min_action = 0;
    min_value = reward_or_value_iters( w, l_part, l_state, 0 );     //ANUJ - Calling the iters version.
    
    /* remember that there is an action bias! */
    nacts = w->all_states_store.states[store_state_index].num_actions;
    for (action=1; action<nacts; action++) {
        tmp = reward_or_value_iters( w, l_part, l_state, action );
        if ( tmp < min_value ) {
            min_value = tmp;
            min_action = action;
        }
    }
//    if ( min_value < 0 ) {
//        fprintf( stderr, "WARGH!\n" );
//        exit( 0 );
//    }
    
    w->all_states_store.values.elts[store_state_index] = min_value;       //Update the V(s,a) for this state.
    w->num_value_updates_iters++;
    
    if (cval < min_value)
        return min_value - cval;
    else
        return cval - min_value;
}
double reward_or_value_iters( world_t *w, int l_part, int l_state, int a )
{
    double value;//, tmp;
    state_t *st;
    int store_state_index;
    
    store_state_index = w->parts[ l_part ].states_ind[ l_state ];
    st = &( w->all_states_store.states[store_state_index] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             w->parts[ l_part ].states_ind,
                             &(w->all_states_store.values) );
    /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */
    value += st->external_dep_vals[a];      //External deps
    
    /* we have to do this because we negated the A matrix! */
    //value = -value + st->tps[ a ].reward;
    value = value + st->tps[ a ].reward;        //Not negating.
    return value;
}

double entries_vec_mult( entry_t *et, int cnt, int *indexes, vec_t *b )
{
    int j;
    int ind;
    double tmpr;
    
    tmpr = 0;
    for ( j=0; j<cnt; j++ ) {
        ind = indexes[et[j].col];
        tmpr += et[j].entry * b->elts[ind];
    }
    
    return tmpr;
}




