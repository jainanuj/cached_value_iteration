//
//  test_fns.c
//  TVI
//
//  Created by Anuj Jain on 2/24/18.
//  Copyright © 2018 Anuj Jain. All rights reserved.
//

#include "test_fns.h"

#ifdef __TEST__
int lsi_to_gsi_over_mdp(world_t *w, int l_part, int state_index)
{
    return w->parts[l_part].states[state_index].over_mdp_state_index;
}

int state_index_to_over_mdp_index(world_t *w, int state_index)
{
    int part_num = state_to_partnum(w, state_index);
    int l_index = gsi_to_lsi(w, state_index);
    return w->parts[part_num].states[l_index].over_mdp_state_index;
}


void print_back_mdp(world_t *w, char *verify_mdp)
{
    FILE *fp_verify = fopen(verify_mdp, "wb");
    int l_part = 0; int l_state = 0, g_index = 0, action = 0, dep_num = 0, dep_count = 0, state_index, over_mdp_state_index;
    double state_prob = 0.0;
    struct StateNode *st_ptr;
    state_t *st = NULL; trans_t *trans;
    
    fprintf(fp_verify, "%d\n", w->num_global_states);
    for (g_index = 0; g_index < w->num_global_states; g_index++)
    {
        l_part = state_to_partnum(w, g_index);
        l_state = gsi_to_lsi(w, g_index);
        st = &(w->parts[l_part].states[l_state]);
        over_mdp_state_index = st->over_mdp_state_index;
        fprintf(fp_verify, "%d %d\n",over_mdp_state_index, st->num_actions);
        trans = st->tps;
        for (action =0; action < st->num_actions; action++)
        {
            fprintf(fp_verify, "%.2f ", trans[action].reward);
            dep_num = trans[action].int_deps + trans[action].ext_deps;
            fprintf(fp_verify, "%d ", dep_num);
            for (dep_count = 0; dep_count < dep_num; dep_count++)
            {
                if (trans[action].entries[dep_count].ext_st_ptr != NULL)
                {
                    state_index=-1;
                    st_ptr = (struct StateNode*)(trans[action].entries[dep_count].ext_st_ptr);
                    over_mdp_state_index = st_ptr->StateNo;
                }
                else
                {
                    state_index = trans[action].entries[dep_count].col;
                    //                if (dep_count < trans[action].int_deps)        //Translate state_index to global.
                    //                  state_index = lsi_to_gsi_over_mdp(w, l_part, state_index);
                    over_mdp_state_index = state_index_to_over_mdp_index(w, state_index);
                }
                state_prob = trans[action].entries[dep_count].entry;
                if (dep_count < trans[action].int_deps)
                    fprintf(fp_verify, "Int-indx:%d %.6f ", state_index, state_prob);
                else
                    fprintf(fp_verify, "ExtDep:over_mdp:%d,indx:%d %.6f ", over_mdp_state_index,state_index, state_prob);
            }
            fprintf(fp_verify, "\n");
        }
    }
    fclose(fp_verify);
}

void print_back_deps(world_t *w, char *verify_deps)
{
    FILE *fp_verify = fopen(verify_deps, "wb");
    int l_part = 0; int l_state = 0, state_index, over_mdp_state_index;
    int index, index2, dep_part, l1_part, l_sub_part; val_t *v;
    
    med_hash_t *dep_hash;
    val_t *v2;
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "State_to_partnum:\n comp_state    over_mdp_state  Part_num\n");
    for (state_index=0; state_index < w->num_global_states; state_index++)
    {
        l_part = state_to_partnum(w, state_index);
        over_mdp_state_index = state_index_to_over_mdp_index(w, state_index);
        fprintf(fp_verify, "comp_num:%d  over_mdp_num:%d  part_num:%d\n", state_index, over_mdp_state_index, l_part );
    }
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "Part_lev0_lev1\n");
    for (l_part=0; l_part < w->num_global_parts; l_part++)
    {
        l1_part = w->part_level0_to_level1[l_part];
        fprintf(fp_verify, "L0 Part:%d  L1 Part:%d\n", l_part, l1_part);
    }
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "Parts %d:\n", w->num_global_parts);
    for (l_part=0; l_part < w->num_global_parts; l_part++)
    {
        fprintf(fp_verify, "part %d num_states: %d\n",l_part, w->parts[l_part].num_states);
        for (state_index=0; state_index < w->parts[l_part].num_states; state_index++)
        {
            over_mdp_state_index = lsi_to_gsi_over_mdp(w, l_part, state_index);
            fprintf(fp_verify, "%d ",over_mdp_state_index);
        }
        fprintf(fp_verify, "\n");
    }
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "L1 parts: %d\n", w->num_level1_parts);
    for (l1_part=0; l1_part < w->num_level1_parts; l1_part++)
    {
        fprintf(fp_verify, "Level1 part %d sub_parts: %d\n", l1_part, w->level1_parts[l1_part].num_sub_parts);
        for (l_sub_part=0; l_sub_part <  w->level1_parts[l1_part].num_sub_parts; l_sub_part++)
        {
            fprintf(fp_verify, "%d ",w->level1_parts[l1_part].sub_parts[l_sub_part]);
        }
        fprintf(fp_verify, "\n");
    }
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "Printing deps for parts: %d\n", w->num_global_parts);
    for (l_part=0; l_part < w->num_global_parts; l_part++)
    {
        fprintf(fp_verify, "Part %d - States: %d\n",l_part, w->parts[l_part].num_states);
        dep_hash = w->parts[l_part].my_local_dependents;
        fprintf(fp_verify, "local_deps for part %d:\n", l_part);
        index=0;
        while (med_hash_iterate(dep_hash, &index, &dep_part, &v))
        {
            fprintf(fp_verify, "%d ", dep_part);
        }
        fprintf(fp_verify, "\n");
        
        dep_hash = w->parts[l_part].my_global_dependents;
        fprintf(fp_verify, "global_deps for part %d:\n", l_part);
        index=0;
        while (med_hash_iterate(dep_hash, &index, &dep_part, &v))
        {
            fprintf(fp_verify, "%d ", dep_part);
        }
        fprintf(fp_verify, "\n");
    }
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "Printing deps for L1 parts: %d\n", w->num_level1_parts);
    for (l1_part=0; l1_part < w->num_level1_parts; l1_part++)
    {
        fprintf(fp_verify, "Level1 part %d - Sub parts: %d\n", l1_part, w->level1_parts[l1_part].num_sub_parts);
        dep_hash = w->level1_parts[l1_part].my_local_dependents;
        fprintf(fp_verify, "local_deps for Level1 part %d:\n", l1_part);
        index=0;
        while (med_hash_iterate(dep_hash, &index, &dep_part, &v))
        {
            fprintf(fp_verify, "%d ", dep_part);
        }
        fprintf(fp_verify, "\n");
    }
    
    
    fprintf(fp_verify, "*****************\n");
    fprintf(fp_verify, "Printing ext_part_states for each part\n");
    for (l_part=0; l_part < w->num_global_parts; l_part++)
    {
        fprintf(fp_verify, "Part %d - States: %d\n",l_part, w->parts[l_part].num_states);
        dep_hash = w->parts[l_part].my_ext_parts_states;
        fprintf(fp_verify, "Ext part states for %d:\n", l_part);
        index=0;index2=0;
        while (med_hash_hash_iterate( dep_hash, &index, &index2,
                                     &dep_part, &l_state, &v2 ))
        {
            over_mdp_state_index = lsi_to_gsi_over_mdp(w, dep_part, l_state);
            fprintf(fp_verify, "\tExt part: %d - State: %d\n", dep_part, over_mdp_state_index);
        }
        fprintf(fp_verify, "\n");
    }
    fclose(fp_verify);
}


void print_back_list(struct StateListNode *list, int component_size, char *verify_list)
{
    struct StateNode *state = NULL;
    struct StateListNode     *stateListNode;
    struct ActionListNode *actionListNode;
    struct ActionNode *actionNode;
    struct StateDistribution *nextState = NULL;
    int ndep = 0;
    
    
    FILE *fp_verify = fopen(verify_list, "wb");
    fprintf(fp_verify, "%d\n", component_size);
    for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
    {
        state = stateListNode->Node;
        fprintf(fp_verify, "%d %d\n",state->StateNo, state->num_actions);
        for (actionListNode = state->Action; actionListNode; actionListNode = actionListNode->Next)
        {
            actionNode = actionListNode->Node;
            if ( actionNode->Dominated )
                continue;
            ndep = 0;
            for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
                ndep++;
            fprintf(fp_verify, "%.2f %d ", actionNode->Cost, ndep);
            for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
            {
                fprintf(fp_verify, "%d %.6f ",nextState->State->StateNo, nextState->Prob);            //Change this so that we can order as int deps and ext deps.
            }
            fprintf(fp_verify, "\n");
        }
    }
    fclose(fp_verify);
}

#endif
