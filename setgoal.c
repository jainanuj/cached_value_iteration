/* setgoal.c */

#include <stdio.h>
#include <stdlib.h>
#include "setgoal.h"
#include "graph.h"
#include "track.h"
#include "strings.h"

void setgoal(int goal_num, int goal_layer)
{
  struct StateListNode *CurrentList, *NewList, *node;
  struct StateNode *snode, *stateNode;
  struct ActionListNode *actionList;
  struct StateDistribution *nextState;
  struct ActionListNode *actionListNode;
  int layer = 0, max_layer = 0, max_state = 0, cur_state;
  double max_value = 9999.99, temp, cur_best;

  CurrentList = CreateStateList();
//  snode = StateIndex[gInitialState];
  snode = StateIndex[0];
  AppendStateList(CurrentList, snode);

  if (goal_layer == 0) {
  while ( CurrentList->Node ) { /* check for empty list */
    NewList = CreateStateList();
    cur_state = 0;
    ++layer;
    for (node = CurrentList; node; node = node->Next) {
      for (actionList = node->Node->Action; actionList; actionList = actionList->Next) {
        nextState = actionList->Node->NextState;
//    for (nextState; nextState; nextState = nextState->Next) {     //ANUJ fixing compile warning
        for (; nextState; nextState = nextState->Next) {
          stateNode = nextState->State;
          if (stateNode->Visited == 0) {
            stateNode->Visited = 1;
            AppendStateList(NewList, stateNode);
            ++cur_state;
          }
        }
      }
    }
    DeleteStateList(CurrentList);
    CurrentList = NewList;
    NewList = NULL;
//    printf("\nlayer %d states: %d", layer, cur_state);
    if (cur_state > max_state) {
      max_layer = layer;
      max_state = cur_state;
    }
  }
  DeleteStateList(CurrentList);
  printf("\nBest layer %d states: %d", max_layer, max_state);
  fflush(0);
  }
  else
    max_layer = goal_layer;

  char out_file_name[30];
  FILE *outf;
  int i;

  sprintf(&out_file_name[0], "mdps/goal.dat");
  outf = fopen(out_file_name, "w");

  for (i = 0; i < gNumStates; ++i) {
    StateIndex[i]->Visited = 0;
  }
  layer = 0;
  CurrentList = CreateStateList();
//  snode = StateIndex[gInitialState];
  snode = StateIndex[0];
  AppendStateList(CurrentList, snode);
  while ( (layer < max_layer) && (CurrentList->Node) ) { /* check for empty list */
    NewList = CreateStateList();
    ++layer;
    for (node = CurrentList; node; node = node->Next) {
      for (actionList = node->Node->Action; actionList; actionList = actionList->Next) {
        nextState = actionList->Node->NextState;
//      for (nextState; nextState; nextState = nextState->Next) {     //ANUJ - compilation error.
        for (; nextState; nextState = nextState->Next) {
          stateNode = nextState->State;
          if (stateNode->Visited == 0) {
            stateNode->Visited = 1;
            AppendStateList(NewList, stateNode);
            if ( (layer == max_layer) && (goal_num > 0) ) {
              fprintf(outf, "%d\n", stateNode->StateNo);
              --goal_num;
            }
          }
        }
      }
    }
    DeleteStateList(CurrentList);
    CurrentList = NewList;
    NewList = NULL;
  }

  fclose(outf); 
}

void loadgoal(int goal_num)
{
  char in_file_name[30], str[164];
  FILE *inf;
  int i;
  struct StateNode *state;

  sprintf(&in_file_name[0], "mdps/goal.dat");

  inf = fopen(in_file_name, "r");
  int goal;
  for (i = 0; i < goal_num; ++i) {
    bzero(str, 64);
    fgets(str, 64, inf); 
    sscanf(str, "%d", &goal);
    state = StateIndex[goal];
    Goal = state;
    state->f = 0;
    state->h = 0;
    state->goalProb = 1.0;
    state->Terminal = 1;
  }
  fclose(inf); 
}
