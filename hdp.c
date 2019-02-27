/* hdp.c */
#include <time.h>
#include "solve.h"
#include "track.h"
#include "graph.h"
#include "stack.h"
#include "top.h"
#include "vi.h"
#include "backup.h"

#include <stdlib.h>
#include <stdio.h>


static int DFS(int s, double epsilon, struct intStack* visited, struct intStack* stack);

#define INFINITY   1000000

int *solved;
int *IDX, *LOW;
struct intStack *visited, *stack;
int *idx;

//void top (struct StateListNode *list, int MaxIter)
void HDP(int s0, double epsilon)
{
  int s, round;
  struct StateNode     *state;
  struct StateListNode *list, *new;
  struct ActionNode    *actionNode;
  struct StateListNode **stateListNode;
  
  round = 0;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  IDX = (int*)malloc((unsigned)gNumStates * sizeof(int));
  LOW = (int*)malloc((unsigned)gNumStates * sizeof(int));
  idx = (int*)malloc(sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;
  for (s = 0; s < gNumStates; s++)
    IDX[s] = -1;

  while(solved[s0] == 0) {
    round++;
    visited = CreateStack();
    stack = CreateStack();
    idx[0] = 0;
    DFS(s0, epsilon, visited, stack);
    while (empty(visited) == 0) {
      s = pop(visited);
      IDX[s] = -1;
    }
    while (empty(stack) == 0) {
      pop(stack);
    }
    Backup(Start);
    printf("\n%d ( %f secs.)  f: %f", 
        round, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  }
  
  Backup(Start);
  printf("\n%d ( %f secs.)  f: %f Converged!", 
      round, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  printf("\n");
}

int DFS(int s, double epsilon, struct intStack* visited, struct intStack* stack)
{
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;
  struct StateNode *state, *stateprime;
  int flag, sprime;

  //printf("\n state number: %d", s);
  // base case
  state = StateIndex[s];
  if ((solved[s] == 1) || (state->Terminal == 1) || (state->Terminal == 5)) {
    solved[s] = 1;
    return 0;
  }
  
  // check residual
  if (Backup(state) > epsilon)
    return 1;
    
  // mark state as active
  push(visited, s);
  push(stack, s);
  IDX[s] = idx[0];
  LOW[s] = idx[0];
  idx[0]++;
  
  // recursive call
  flag = 0;
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    for (nextState = actionNode->NextState;
	 nextState;
	 nextState = nextState->Next) {
      stateprime = nextState->State;
      sprime = stateprime->StateNo;
      if (IDX[sprime] == -1) {
        if (DFS(sprime, epsilon, visited, stack) == 1)
	  flag = 1;
        if (LOW[s] > LOW[sprime])
	  LOW[s] = LOW[sprime];
      }
      else if (instack(stack, sprime) == 1) {
        if (LOW[s] > IDX[sprime])
	  LOW[s] = IDX[sprime];
      }
    }
  }
  
  // update if necessary
  if (flag == 1) {
    Backup(state);
    return 1;
  }
  
  // try to label
  else if (IDX[s] == LOW[s]) {
    while (top(stack) != s) {
      sprime = pop(stack);
      solved[sprime] = 1;
    }
    pop(stack);
    solved[s] = 1;
  }
  
  return flag;
}
 
