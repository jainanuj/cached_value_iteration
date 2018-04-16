//
//  cached_vi.h
//  TVI
//
//  Created by Anuj Jain on 11/29/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#ifndef cached_vi_h
#define cached_vi_h

#include <stdio.h>
#include <math.h>
#include "types.h"
#include "logger.h"
#include "cache_aware_vi.h"

#ifdef __TEST__
#include "graph.h"
#endif


//double value_iterate(world_t *w, double heat_epsilon_current);
double value_iterate( world_t *w, double epsilon_partition, double epsilon_overall );
double value_iterate_level1_partition( world_t *w, int level1_part );
double value_iterate_partition( world_t *w, int l_part );

int level1_part_available_to_process(world_t *w);
int get_next_level1_part(world_t *w);
void add_level1_parts_deps_for_eval(world_t *w, int level1_part_changed);
int clear_level0_queue(world_t *w);
unsigned long check_dirty(world_t *w, int l_part);
int add_level0_queue(world_t *w, int l_part);
int clear_level0_dirty_flag(world_t *w, int l_part);
int part_available_to_process(world_t *w);
int get_next_part(world_t *w);
void add_level0_partition_deps_for_eval(world_t *w, int l_part_changed);
int set_dirty(world_t *w, int l_part);
double value_update( world_t *w, int l_part, int l_state );
double reward_or_value( world_t *w, int l_part, int l_state, int a );
double get_remainder( world_t *w, int l_part, int l_state, int action );
double value_update_iters( world_t *w, int l_part, int l_state );
double reward_or_value_iters( world_t *w, int l_part, int l_state, int a );
double entries_vec_mult( entry_t *et, int cnt, vec_t *b );

float part_heat( world_t *w, int l_part_num );
void update_partition_potentials( world_t *w, int l_part_changed, double heat_left );
void compute_part_heat( world_t *w, int l_part_num );
void clear_partition_heat( world_t *w, int l_part_num );
void update_part_heat( world_t *w, int l_part_num, double heat_left);
void solve_using_prioritized_vi( world_t *w, double epsilon_partition, double epsilon_overall );
int pick_part_and_wash_it( world_t *w );
int pick_partition( world_t *w );
double get_remainder_no_cache( world_t *w, int l_part, int l_state, int action );
double reward_or_value_no_cache( world_t *w, int l_part, int l_state, int a );
double get_heat( world_t *w, int l_part, int l_state );



#endif /* cached_vi_h */
