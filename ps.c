/* ps.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "ps.h"
#include "backup.h"
#include "solve.h"
#include "statequeue.h"
#define MAX 999.99

/* Static variables */
static int NumExpandedStates; // number of states expanded since start of alg
static int NumSolutionStates;
static struct StateListNode *ExpandedStateList;
static int CountBackups;
static double Residual;
static int ExitFlag;
static int NumInQueues;

int* inQueue;
float elapsedTime;

void PS(int flag)
{
  struct StateNode *node, *predecessor;
  struct StateQueue *queue;
  struct ActionNode *predecessorAction, *actionNode;
  struct ActionListNode *predecessorActions, *actionListNode;
  struct StateDistribution *nextState;
  int state, i, index;
  double change, prob = 0.0;
  
  inQueue = (int*)malloc((unsigned)(gNumStates + 1) * sizeof(int));
  for (i = 0; i <= gNumStates; i++)
    inQueue[i] = 0;
  NumExpandedStates = 0;
  NumInQueues = 0;
  /*for (i = 0; i < gNumStates; i++) {
    StateIndex[i]->f = MAX;
    StateIndex[i]->g = MAX;
  }
  Goal->f = 0.0;
  Goal->g = 0.0;
  Start->f = MAX;
  Start->g = MAX;*/
  queue = (struct StateQueue*)CreateQueue(gNumStates);
  CreateHeuristic();
  
  InsertQueue(queue, Goal->StateNo, 0.0);
  inQueue[Goal->StateNo] = 1;
  NumInQueues++;

  while (!IsEmpty(queue)) {
    if ((NumExpandedStates % 10000) == 0) {
      //printqueue(queue);
      elapsedTime = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
      if (elapsedTime > 3600) {
        printf("Total number of InQueue operations: %d\n", NumInQueues);
        return;
      }
      printf("\n%d ( %f secs.)", 
          NumExpandedStates, elapsedTime);
      printf("  f: %f", Start->f);
    }
    
    state = OutQueue(queue);
    if (state < 0) {
      inQueue[gNumStates] = 0;
      node = Start;
    }
    else {
      inQueue[state] = 0;
      node = StateIndex[state];
    }
    NumExpandedStates++;
    
    change = Backup(node);
    if ((change > gEpsilon) || (node == Goal)) {
    
    /* backup its predecessors and insert them into the queue */
    if (flag == 0) {
      predecessorActions = node->PrevAction;
    }
    else {
      predecessorActions = node->BestPrevActions; 
    }  
    while(predecessorActions) {
      predecessorAction = predecessorActions->Node;
      predecessor = predecessorAction->PrevState;
      state = predecessor->StateNo;
      for (actionListNode = predecessor->Action;
           actionListNode;
           actionListNode = actionListNode->Next) {
        actionNode = actionListNode->Node;
        for (nextState = actionNode->NextState;
             nextState;
             nextState = nextState->Next) {
          if (nextState->State == node) {
            prob = nextState->Prob;
          }
        }
      }
      //change = Backup(predecessor);
      //if ((change > gEpsilon) && (!inQueue[state])) {
      if (((state < 0) && (!inQueue[gNumStates])) || (!inQueue[state])) {
        InsertQueue(queue, state, change * prob);
        //InsertQueue(queue, state, change);
        if (state >= 0)
          inQueue[state] = 1;
        else
          inQueue[gNumStates] = 1;
        NumInQueues++;
      }
      else {
      	index = InQueue(queue, state);
      	if (Value(queue, index) < change * prob) {
      	  IncreaseValue(queue, index, change * prob);
      	}
      }
      predecessorActions = predecessorActions->Next;
    }
  }
  }
  
  
  printf("\n%d ( %f secs.)", 
      NumExpandedStates, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
  printf("  f: %f Converged!\n", Start->f);
  printf("Total number of InQueue operations: %d\n", NumInQueues);

}

