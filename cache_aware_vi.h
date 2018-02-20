//
//  cache_aware_vi.h
//  TVI
//
//  Created by Anuj Jain on 11/12/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#ifndef cache_aware_vi_h
#define cache_aware_vi_h

//#define __TEST__


#include <sys/stat.h>
#include <string.h>
#include "types.h"
#include "graph.h"
#include "cached_vi.h"
#include "logger.h"

#define GAUSS_CONST 2
#define heat_epsilon_final_def 0.0001;
#define heat_epsilon_initial_def 10000;


double cache_aware_vi(struct StateListNode *list, int MaxIter, int round, int component_size);
world_t* init_world(struct StateListNode *list, int component_size, int round);

void traverse_comp_form_parts(struct StateListNode *list, world_t *w, int round);
void bfs_list_to_parts(struct StateListNode *list, world_t *w);

void create_ext_state_val_deps(world_t *w, int part_num, int l_state);
void copy_state_to_part(struct StateNode *state, world_t *w, int round);
void assign_state_to_part_num(struct StateListNode *list, world_t *w);
void form_level1_parts(world_t *w);
void assign_part_to_level1_part(world_t *w);

void resolve_ext_deps(world_t *w);
void cache_dependencies_in_states( world_t *w );

void add_dep( world_t *w,
             int l_start_part, int l_end_part, int level1_start_part, int level1_end_part );

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
void initialize_level1_partitions( world_t *w );


void init_level1_part_queue( world_t *w );
void init_level0_bit_queue(world_t *w);

void translate_and_negate_all( world_t *w );
void translate_all(world_t *w);

void translate_to_local_matrix( world_t *w, int l_part );
void negate_matrix( world_t *w, int l_part );
double gauss(double x);
double linear(double x);

#ifdef __TEST__
int lsi_to_gsi_over_mdp(world_t *w, int l_part, int state_index);
void print_back_mdp(world_t *w, char *verify_mdp);
int state_index_to_over_mdp_index(world_t *w, int state_index);
void print_back_list(struct StateListNode *list, int component_size, char *verify_list);
void print_back_deps(world_t *w, char *verify_deps);
#endif

#endif /* cache_aware_vi_h */
