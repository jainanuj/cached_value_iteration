//
//  test_fns.h
//  TVI
//
//  Created by Anuj Jain on 2/24/18.
//  Copyright © 2018 Anuj Jain. All rights reserved.
//

#ifndef test_fns_h
#define test_fns_h

#include <stdio.h>
#include "cache_aware_vi.h"
#include "cached_vi.h"
//#define __TEST__

#ifdef __TEST__
int lsi_to_gsi_over_mdp(world_t *w, int l_part, int state_index);
void print_back_mdp(world_t *w, char *verify_mdp);
int state_index_to_over_mdp_index(world_t *w, int state_index);
void print_back_list(struct StateListNode *list, int component_size, char *verify_list);
void print_back_deps(world_t *w, char *verify_deps);
#endif

#endif /* test_fns_h */
