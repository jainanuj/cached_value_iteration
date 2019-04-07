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

/*double value_iterate( world_t *w, double heat_epsilon_current )
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

void solve_using_prioritized_vi( world_t *w, double epsilon_partition, double epsilon_overall )
{
    int total_visits;
    
    total_visits = 0;
    heat_epsilon_partition = epsilon_partition;
    heat_epsilon_overall = epsilon_overall;

    while (1) {
        if ( pick_part_and_wash_it( w ) )
        {
            total_visits++;
        } else {
            break;
        }
    }
    
    wlog( 1, "\n\nTotal partitions processed: %d\n\n", total_visits );
    
}


int pick_part_and_wash_it( world_t *w )
{
    int l_part_num;
    float heat_left;
    
    l_part_num = pick_partition( w );
    if ( l_part_num == -1 ) {
        /* either there was an error, or there isn't any more heat. */
        return 0;
    }
    
    w->parts_processed++;
    w->parts[ l_part_num ].visits++;
    
    heat_left = value_iterate_partition( w, l_part_num );
        /*     fprintf( stderr, "%.4f\n", maxheat ); */
    
    update_partition_potentials( w, l_part_num, heat_left );
    
    heap_add( w->part_heap, l_part_num );
    
    return 1;
}

int pick_partition( world_t *w )
{
    int l_part_num, result;
    float heat;
    
    /* the 0 here indicates the position of the element we want to
     peek at.  in this case, it's the head of the head. */
    result = heap_peek( w->part_heap, 0, &l_part_num );
    if ( !result ) {
        wlog( 1, "Whoa!  Error peeking the heap!\n" );
        return -1;
    }
    
    heat = part_heat( w, l_part_num );
    
    if ( heat <  heat_epsilon_overall) {
        /*     if ( verbose ) { */
        /*       wlog( 1, "\n\nNo more heat. All done.\n\n\n" ); */
        /*     } */
        return -1;
    }
    
    result = heap_pop( w->part_heap, &l_part_num );
    if ( !result ) {
        wlog( 1, "Whoa!  Error popping the heap!\n" );
        return -1;
    }
    
    w->parts[ l_part_num ].my_heap_num = -1;
    
    
    /*   if ( !heap_verify( w->part_heap ) ) { */
    /*     my_heap_dump( w, w->part_heap ); */
    /*     wlog( 1, "WHOA! HEAP ERROR!\n" ); */
    /*     wlog( 1, "\n\n\n" ); */
    /*   } */
    
    return l_part_num;
}

float part_heat( world_t *w, int l_part_num ) {
    return w->parts[ l_part_num ].heat;
}

void update_partition_potentials( world_t *w, int l_part_changed, double heat_left ) {
    int l_start_state;
    int l_start_part;
    float *part_heat, tmpheat;
    med_hash_t *dep_part_hash, *dep_state_hash;
    int index1, index2;
    val_t *v;
    
    if (heat_left > heat_epsilon_overall)
        update_part_heat(w, l_part_changed, heat_left);
    else
        clear_partition_heat( w, l_part_changed );
    
    /* We have to process every state that depends on us.  We'll clear
     all the dependent partitions that have a heat link to us.  Then, as
     we're processing each state, we always increment the max.
     Remember, my_local_dependents is a hash_set, and it maps
     g_start_part's to g_start_state's
     */
    
    dep_part_hash = w->parts[ l_part_changed ].my_local_dependents;
    
    index1 = 0;
    while ( med_hash_iterate( dep_part_hash, &index1, &l_start_part, &v ) ) {
        
        
        /* Grab a pointer to the  g_start_part -> g_part_changed heatlink */
        med_hash_get_floatp( w->parts[ l_start_part ].heat_links,
                            l_part_changed, &part_heat );
        
        /* Reset the heat link from g_start_part to g_part_changed */
        *part_heat = 0;
        
        /* iterate over all of the states in l_start_part that depend on
         something in l_part_changed */
        dep_state_hash = (med_hash_t *)(v->vptr);
        index2 = 0;
        while ( med_hash_iterate( dep_state_hash, &index2,
                                 &l_start_state, &v ) ) {
            
            
            tmpheat = get_heat( w, l_start_part, l_start_state );       //tmpheat should give an estimate of how much value of this state can improve.
            
            /*       if ( tmpheat != 0 && l_part_changed == 7 ) { */
            /* 	wlog( 1, "    Heat of state %d is %.2f\n", l_start_state, tmpheat ); */
            /*       } */
            
            if ( tmpheat > *part_heat ) {
                *part_heat = tmpheat;
            }
            
        }
        
        /* 'kay. now we've updated the g_start_part -> g_part_changed heatlink.
         we need to recompute the heat of this partition. */
        compute_part_heat( w, l_start_part );
        
    }
    
}

void compute_part_heat( world_t *w, int l_part_num ) {
    int index;
    float max_heat, *tmpf;
    med_hash_t *m;
    int key;
    int remove_result;
    
    /* my maximum heat is the maximum between me and any other partition.
     iterate over all of my heat links, testing the heat. */
    max_heat = 0;
    index = 0;
    m = w->parts[ l_part_num ].heat_links;
    while ( med_hash_iterate_float( m, &index, &key, &tmpf ) ) {
        max_heat = max_heat > *tmpf ? max_heat:*tmpf;
    }
    
    if ( max_heat == w->parts[ l_part_num ].heat ) {
        return;
    }
    
    w->parts[ l_part_num ].heat =
    max_heat > w->parts[ l_part_num ].primary_heat ? max_heat: w->parts[ l_part_num ].primary_heat;
    
    /* update the partition heap! */
    heap_remove( w->part_heap, w->parts[ l_part_num ].my_heap_num,
                &remove_result );
    heap_add( w->part_heap, l_part_num );
    
    
    /* XXX super debug code */
    
}


void clear_partition_heat( world_t *w, int l_part_num ) {
    int index;
    med_hash_t *m;
    int key;
    float *val;
    
    /* my heat is zero */
    w->parts[ l_part_num ].heat = 0;
    w->parts[ l_part_num ].primary_heat = 0;
    
    /* All of my states that depend on other partitions
     have a heat of zero.  Each heat link is therefore zero as well. */
    index = 0;
    m = w->parts[ l_part_num ].heat_links;
    while ( med_hash_iterate_float( m, &index, &key, &val ) ) {
        *val = 0;
    }
}

void update_part_heat( world_t *w, int l_part_num, double heat_left)
{
    int index, index2;
    med_hash_t *m_heat_links;
    val_t val2, *v;
    int linked_partnum, l_start_state;
    med_hash_t *dep_part_hash, *dep_state_hash;
    double tmpheat, max_heat_ext_part = 0, max_heat_ext = 0;
    float *part_link_heat;
    
    index = 0;
    m_heat_links = w->parts[ l_part_num ].heat_links;
    max_heat_ext = 0;
    while (med_hash_iterate_float(m_heat_links, &index, &linked_partnum, &part_link_heat ) ) {
        dep_part_hash = w->parts[linked_partnum].my_local_dependents;
        if (med_hash_get(dep_part_hash, l_part_num, &val2) == MH_FOUND) {
            dep_state_hash = (med_hash_t *)(val2.vptr);
            max_heat_ext_part = 0; index2 = 0;
            
            while ( med_hash_iterate( dep_state_hash, &index2, &l_start_state, &v ) ) {
                tmpheat = get_heat( w, l_part_num, l_start_state );       //tmpheat should give an estimate of how much value of this state can improve.
                if ( tmpheat > max_heat_ext_part ) {
                    max_heat_ext_part = tmpheat;
                }
            }
        }
        *part_link_heat = max_heat_ext_part;
        if (max_heat_ext_part > max_heat_ext)
            max_heat_ext = max_heat_ext_part;
    }
    w->parts[ l_part_num ].heat = max_heat_ext > heat_left ? max_heat_ext:heat_left;
    w->parts[ l_part_num ].primary_heat = heat_left;
}


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
        w->new_partition_wash++;
        w->parts[next_level0_part].washes++;
        if (tmp > heat_epsilon_overall)
        {
            //Add local deps to queue. Mark global as dirty.
            add_level0_partition_deps_for_eval(w, next_level0_part);
            if (w->part_queue->numitems > w->level1_parts[level1_part].num_sub_parts )
                wlog(1, "storing too many items in level0 q. NumItems = %d\n",w->part_queue->numitems);
            maxheat = tmp>maxheat ? tmp: maxheat;
        }
    }
    return maxheat;
}

double value_iterate_partition( world_t *w, int l_part )
{
    part_t *pp;
    int l_state, state_cnt, i;
    float max_heat, delta, part_internal_heat = 0;
    med_hash_t *dep_part_hash;
    int numPartitionIters = 0;
    
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
        val_state_action->d = w->parts[g_end_ext_partition].values.elts[l_end_ext_state]; //Setting the value of that ext state
    }
    max_heat = 0;
    //First iteration of the partition.
    for ( i = 0; i < state_cnt; i++ )
    {
        l_state = pp->variable_ordering[i];
        delta = 0;
        if ( (w->parts[l_part].states[l_state].Terminal != 1) && (w->parts[l_part].states[l_state].Terminal !=5) )
            delta = value_update( w, l_part, l_state );
        max_heat = fabs( delta ) > max_heat ? fabs( delta ): max_heat;
    }
    
    if (max_heat > heat_epsilon_partition)
    {
        //This is equivalent to while(true) as we don't change max_heat in the while loop.
        //If max_heat == 0 we don't need to enter this loop as partition is already cold.
        part_internal_heat = 0;
        while(max_heat > 0)
        {
            //part_internal_heat initialized to 0 at beginning of each iteration.
            //It attains value of max heat in that iteration and keeps on reducing with every iteration.
            //It signifies that we are making progress within the partition.
            part_internal_heat = 0;
            for ( i = 0; i < state_cnt; i++ )
            {
                l_state = pp->variable_ordering[i];
                delta = 0;
                if ( (w->parts[l_part].states[l_state].Terminal != 1) && (w->parts[l_part].states[l_state].Terminal !=5) )
                    delta = value_update_iters( w, l_part, l_state );
                part_internal_heat = fabs( delta ) > part_internal_heat ? fabs( delta ): part_internal_heat;
            }
            w->parts[ l_part ].washes++;
            numPartitionIters++;
            if (part_internal_heat > max_heat)
                max_heat = part_internal_heat;
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
    double tmp, value, cval;  //max_value
    
    if (w->parts[l_part].states[l_state].goal == 1)
        return 0;
    
    cval = w->parts[ l_part ].values.elts[ l_state ];
    
    min_action = 0;
    value = reward_or_value( w, l_part, l_state, 0 );
    
    /* remember that there is an action bias! */
    nacts = w->parts[ l_part ].states[ l_state ].num_actions;
    for (action=1; action<nacts; action++)
    {
        tmp = reward_or_value( w, l_part, l_state, action );
        if ( tmp < value ) {            //minimization <
            value = tmp;
            min_action = action;
        }
    }
    w->parts[l_part].states[l_state].bestAction = min_action;       //update best Action for this state.
    //Commenting Out - ANUJ - max_value can be -ve. This is when we have a cost to pay for the action.
    //  if ( max_value < 0 ) {
    //    fprintf( stderr, "WARGH!\n" );
    //    exit( 0 );
    //  }
    w->parts[ l_part ].values.elts[ l_state ] = value;       //Update the V(s) for this state.
    w->num_value_updates++;
    if (value <= cval)
        return cval - value;
    else
    {
        return value - cval;
    }
    
}
double reward_or_value( world_t *w, int l_part, int l_state, int a ) {
    double value, tmp;
    state_t *st;
    st = &( w->parts[ l_part ].states[ l_state ] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             &(w->parts[ l_part ].values) );
    
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

double reward_or_value_no_cache( world_t *w, int l_part, int l_state, int a ) {
    double value, tmp;
    state_t *st;
    st = &( w->parts[ l_part ].states[ l_state ] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             &(w->parts[ l_part ].values) );
    
    /* add in external deps! */
    
    /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */
    
    tmp = get_remainder_no_cache( w, l_part, l_state, a );     //Getting external dependencies.
    
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
    
    val_hash2 = 0;
    tt = &( w->parts[ l_part ].states[ l_state ].tps[ action ] );
    dep_cnt = tt->ext_deps;
    ext_et = &( tt->entries[ tt->int_deps ] );    //* point to the first external entry
    
    for ( i=0; i<dep_cnt; i++ )
    {
        val_hash2 += ext_et[ i ].entry * (w->parts[l_part].states[l_state].external_state_vals[action][i]->d);
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


double get_remainder_no_cache( world_t *w, int l_part, int l_state, int action ) {
    int i, e_l_state, e_g_state, dep_cnt, e_l_p;
    trans_t *tt;
    entry_t *ext_et;
    double val;
    
    val = 0;
    
    tt = &( w->parts[ l_part ].states[ l_state ].tps[ action ] );
    dep_cnt = tt->ext_deps;
    ext_et = &( tt->entries[ tt->int_deps ] );  /* point to the first external entry */
    
    for ( i=0; i<dep_cnt; i++ )
    {
        if (ext_et[i].col == -1)
            val += ext_et[ i ].entry * (w->parts[l_part].states[l_state].external_state_vals[action][i]->d);
        else
        {
            e_g_state = ext_et[ i ].col;
            e_l_p = state_to_partnum( w, e_g_state );
            e_l_state = gsi_to_lsi( w, e_g_state );
            val += ext_et[ i ].entry * w->parts[ e_l_p ].values.elts[ e_l_state ];
        }
    }
    
    return val;
}



double value_update_iters( world_t *w, int l_part, int l_state )
{
    int action, min_action, nacts;     //max_action,
    double tmp, value, cval;        //max_value,
    
    if (w->parts[l_part].states[l_state].goal == 1)
        return 0;
    
    cval = w->parts[ l_part ].values.elts[ l_state ];
    
    min_action = 0;
    value = reward_or_value_iters( w, l_part, l_state, 0 );     //ANUJ - Calling the iters version.
    
    /* remember that there is an action bias! */
    nacts = w->parts[ l_part ].states[ l_state ].num_actions;
    for (action=1; action<nacts; action++) {
        tmp = reward_or_value_iters( w, l_part, l_state, action );
        if ( tmp < value ) {            // < minimization
            value = tmp;
            min_action = action;
        }
    }
//    if ( min_value < 0 ) {
//        fprintf( stderr, "WARGH!\n" );
//        exit( 0 );
//    }
    
    w->parts[ l_part ].values.elts[ l_state ] = value;       //Update the V(s,a) for this state.
    w->num_value_updates_iters++;
    
    if (value <= cval)
        return cval - value;
    else
    {
        return value - cval;
    }
}

double reward_or_value_iters( world_t *w, int l_part, int l_state, int a )
{
    double value;//, tmp;
    state_t *st;
    st = &( w->parts[ l_part ].states[ l_state ] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             &(w->parts[ l_part ].values) );
    /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */
    value += st->external_dep_vals[a];      //External deps
    
    /* we have to do this because we negated the A matrix! */
    //value = -value + st->tps[ a ].reward;
    value = value + st->tps[ a ].reward;        //Not negating.
    return value;
}

double entries_vec_mult( entry_t *et, int cnt, vec_t *b )
{
    int j;
    double tmpr;
    
    tmpr = 0;
    for ( j=0; j<cnt; j++ ) {
        tmpr += et[j].entry * b->elts[et[j].col];
    }
    
    return tmpr;
}


double get_heat( world_t *w, int l_part, int l_state ) {
    int action, nacts;
    double cur, could_be, heat, tmp;
    
    if (w->parts[l_part].states[l_state].goal == 1)
        return 0;

    heat = 0;
    cur = w->parts[ l_part ].values.elts[ l_state ];
    could_be = reward_or_value_no_cache( w, l_part, l_state, 0 );
    if (could_be < cur)         // < for minimization
        heat = cur - could_be;      // cur - could_be for minimization
    
    nacts = w->parts[ l_part ].states[ l_state ].num_actions;
    for ( action=1; action<nacts; action++ ) {
        tmp = 0;
        could_be = reward_or_value_no_cache( w, l_part, l_state, action );
        if (could_be < cur)         // < for minimization
            tmp = cur - could_be;       // cur - could_be for minimization
        heat = tmp>heat?tmp:heat;
    }
    
    /*   return fabs(heat); */
    return heat;
}


