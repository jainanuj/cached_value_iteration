//
//  cache_aware_vi.h
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#ifndef cache_aware_vi_h
#define cache_aware_vi_h



#include <sys/stat.h>
#include <string.h>
#include "types.h"
#include "graph.h"
#include "cached_vi.h"
#include "logger.h"
#include "intheap.h"

#define GAUSS_CONST 2
#define heat_epsilon_final_def 0.000001;
#define heat_epsilon_initial_def 1000;


double cache_aware_vi(struct StateListNode *list, int MaxIter, int round, int component_size);
world_t* init_world(struct StateListNode *list, int component_size, int round);

void traverse_comp_form_parts(struct StateListNode *list, world_t *w, int round);
void bfs_list_to_parts(struct StateListNode *list, world_t *w);

void create_ext_state_val_deps(world_t *w, int part_num, int l_state);
void copy_state_to_part(struct StateNode *state, world_t *w, int round);
void assign_state_to_part_num(struct StateListNode *list, world_t *w);
void assign_state_to_part_cluster(struct StateListNode *list, world_t *w, int round);
void form_level1_parts(world_t *w);
void form_thread_parts(world_t *w);
void assign_part_to_level1_part(world_t *w);
void assign_part_to_thread_parts(world_t *w);

void resolve_ext_deps(world_t *w);
void cache_dependencies_in_states( world_t *w );

void add_dep( world_t *w,
             int l_start_part, int l_start_state, int l_end_part, int level1_start_part, int level1_end_part );

void add_part_ext_dep_states( world_t *w,
                             int l_start_part,
                             int l_end_state, int l_end_part);

void add_cache_states(world_t *w,
                      int l_start_part,
                      int l_start_state, int l_end_state, int l_end_part, val_t **arrayValptrs, int indexVal );

void cache_val_state_out_of_world(val_t **arrayValptrs, int indexVal, double ext_val);


int state_to_partnum( world_t *w, int state_t );
int gsi_to_lsi(world_t *w, int global_index);
void initialize_partitions( world_t *w );
void initialize_thread_parts( world_t *w );
void initialize_level1_partitions( world_t *w );


void init_level1_part_queue( world_t *w );
void init_level0_bit_queue(world_t *w);
void init_thread_bit_queue(world_t *w);

void translate_and_negate_all( world_t *w );
void translate_all(world_t *w);

void translate_to_local_matrix( world_t *w, int l_part );
void negate_matrix( world_t *w, int l_part );
double gauss(double x);
double linear(double x);

int part_cmp_func( int lp_a, int lp_b, void *vw );
void part_swap_func( int lp_a, int lp_b, void *vw );
void part_add_func( int lp_obj, int pos, void *vw );
void init_part_heap( world_t *w );

void compute_initial_partition_priorities( world_t *w );
void reorder_states_within_partitions( world_t *w );
void reorder_states_within_partition( world_t *w, int l_part );
void reorder_no_reorder( world_t *w, int l_part );

void save_resulting_vector( world_t *w, char *fn, int r, int component_size);
void save_resulting_list(struct StateListNode *list, char *fn, int r, int component_size);

#endif /* cache_aware_vi_h */
