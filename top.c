/* top.c */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "solve.h"
#include "track.h"
#include "graph.h"
#include "top.h"
#include "vi.h"
#include "backup.h"

#define MAX_ITER   1000
static int scnt;
static int cnt;

int *id;
int *postI;
int *postR;
int *reachable;
int *reachableC;
int *sizes;
  
void reachability(int);

void TVI()
{
  int state, round, biggest_size = 0;
  struct StateNode     *node;
  struct StateListNode *list, *newlist;
  struct ActionNode    *actionNode;
  struct StateListNode **stateListNode;
  
  id = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postI = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postR = (int*)malloc((unsigned)gNumStates * sizeof(int));
  reachable = (int*)malloc((unsigned)gNumStates * sizeof(int));

  for (state = 0; state < gNumStates; state++) {
    reachable[state] = 0;
  }
  reachability(-1);
/*
  for (state = 0; state < gNumStates; state++) {
    printf("  %d %d", state, reachable[state]);
  }
*/
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = 0; state < gNumStates; state++)
    if (id[state] == -1)
      dfsR(state);
  for (state = 0; state < gNumStates; state++)
    postR[state] = postI[state];
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = gNumStates - 1; state >= 0; state--)
  //for (state = 0; state < gNumStates; state++)
    if (id[postR[state]] == -1) {
      dfs(postR[state]);
      scnt++;
    }
  
  printf("\nComponents: %d", scnt);
  // debugging  
  stateListNode = (struct StateListNode**)malloc(scnt * sizeof(struct StateListNode*));
  reachableC = (int*)malloc(scnt * sizeof(int));
  sizes = (int*)malloc(scnt * sizeof(int));     //Stores size of each strogly connected component.
  
  for (round = 0; round < scnt; round++)
  {
    stateListNode[round] = (struct StateListNode*)malloc(sizeof(struct StateListNode));
    stateListNode[round]->Node = NULL;
    stateListNode[round]->Next = NULL;
    reachableC[round] = 0;
    sizes[round] = 0;
  }
    
  for (state = 0; state < gNumStates; state++)
  {
    //printf("\n%d", id[state]);
      list = stateListNode[id[state]];
      node = StateIndex[state];
      if (reachable[state])
          reachableC[id[state]] = 1;
      
      ++sizes[id[state]];
      if (!list->Node)
      {
          list->Node = node;
      }
      else
      {
          newlist = (struct StateListNode*)malloc((unsigned)sizeof(struct StateListNode));
          newlist->Node = list->Node;
          newlist->Next = list->Next;
          list->Node = node;
          list->Next = newlist;
      }
  }
  
  /* we only need to backup strongly connected component reachable from the initial state */
//  scnt = id[gInitialState] + 1;
  for (round = 0; round < scnt; ++round)
  {
      if (reachableC[round] == 1)
      {
      //printf("\tComponent %d", round);
          list = stateListNode[round];
          int cur_size = sizes[round];
          //printf("\n%d", cur_size);
          if (cur_size > biggest_size)
              biggest_size = cur_size;
          viter(list, MAX_ITER);
//      printf("\n%3d ( %f secs.)  f: %f", round, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
      }
  }
  Backup(Start);
  printf("\nBiggest component size: %d", biggest_size);
  printf("\n%d ( %f secs.)  f: %f Converged!", round, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  printf("\n");
}

void dfs(int w)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;
   
  id[w] = scnt;
  state = StateIndex[w];
  
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    for (nextState = actionNode->NextState;
	 nextState;
	 nextState = nextState->Next) {
      nodeNumber = nextState->State->StateNo;
      if (id[nodeNumber] == -1)
        dfs(nodeNumber);
    }
  }
  postI[cnt++] = w;
}
 
void dfsR(int w)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateNode *prevState;
   
  id[w] = scnt;
  state = StateIndex[w];
  
  for (actionListNode = state->PrevAction;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    prevState = actionNode->PrevState;
      nodeNumber = prevState->StateNo;
      if (id[nodeNumber] == -1)
        dfsR(nodeNumber);
  }
  postI[cnt++] = w;
}

void viter(struct StateListNode *list, int MaxIter)
{
  int                       Iter;
  double                    diff, maxdiff, /* Bellman residual */
                            error, time, threshold = 300.0;
  struct StateListNode     *stateListNode;

// debug print out states
/*
  for (stateListNode = list; stateListNode; stateListNode = stateListNode->Next) {
    printf("\nstate %d", stateListNode->Node->StateNo);
    fflush(0);
  }
*/

  for (Iter = 0; Iter < MaxIter; Iter++)
  {
    maxdiff = 0.0;
    for (stateListNode = list; stateListNode; stateListNode = stateListNode->Next)
    {
      if ((stateListNode->Node->Terminal != 1) && (stateListNode->Node->Terminal != 5))
      {
          diff = Backup(stateListNode->Node);
          if (diff > maxdiff)
              maxdiff = diff;
      }
    }
    if ( gDiscount < 1.0 ) 
      error = (maxdiff * gDiscount)/(1.0 - gDiscount);
    else
      error = maxdiff;
    //printf("\n%3d ( %f secs.)  f: %f  Error bound: %f", Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, error);   
    //fflush(0);
    time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
//    if ( error < gEpsilon )
    if ( (error < gEpsilon) || (time > threshold) )
      return;
  }
}

void reachability(int s) {
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;
   
  if (s < 0)
    state = Start;
  else {
    state = StateIndex[s];
    reachable[s] = 1;
  }

  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    for (nextState = actionNode->NextState;
	 nextState;
	 nextState = nextState->Next) {
      nodeNumber = nextState->State->StateNo;
      if (reachable[nodeNumber] == 0)
        reachability(nodeNumber);
    }
  }
}
