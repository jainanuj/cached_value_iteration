/* ips.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "ips.h"
#include "backup.h"
#include "solve.h"
#include "statequeue.h"

/* Static variables */
static int NumExpandedStates; // number of states expanded since start of alg
static int NumSolutionStates;
static struct StateListNode *ExpandedStateList;
static int CountBackups;
static double Residual;
static int ExitFlag;
static int NumInQueues;

int* inQueue;

void IPS(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node, *predecessor;
  struct StateQueue *queue;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  int state, i, index;
  double change, newprio;
  float elapsedTime;
  
  inQueue = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (i = 0; i < gNumStates; i++)
    inQueue[i] = 0;
  NumExpandedStates = 0;
  NumInQueues = 0;
  queue = (struct StateQueue*)CreateQueue(gNumStates);
  /*for (i = 0; i < gNumStates; i++)
    StateIndex[i]->f = MAX;
  Goal->f = 0.0;
  Start->f = MAX;*/
  CreateHeuristic();

  InsertQueue(queue, Goal->StateNo, 1.0);
  inQueue[Goal->StateNo] = 1;

  while (!IsEmpty(queue)) {
    if ((NumExpandedStates % 10000) == 0) {
      elapsedTime = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
      if (elapsedTime > 3600) {
        printf("Total number of InQueue operations: %d\n", NumInQueues);
        return;
      }
      printf("\n%d ( %f secs.)", 
          NumExpandedStates, elapsedTime);
      printf("  f: %f", Start->f);
    }
    
    double value;
    value = QueueTopValue(queue);
    state = OutQueue(queue);
    //printf("\nState: %d", state);
    inQueue[state] = 0;
    NumInQueues++;
    node = StateIndex[state];
    NumExpandedStates++;
    
    /* backup its predecessors and insert them into the queue */
    predecessorActions = node->PrevAction;  
    while(predecessorActions) {
      predecessorAction = predecessorActions->Node;
      predecessor = predecessorAction->PrevState;
      state = predecessor->StateNo;
      //change = FBackup(predecessor);
      change = Backup(predecessor);
      //newprio = predecessor->goalProb * 100000 + change / predecessor->f;
      newprio = change / predecessor->f;
      if (change > gEpsilon) {
      	if (state < 0)
      	  continue;
      	if (!inQueue[state]) {
          InsertQueue(queue, state, newprio);
          inQueue[state] = 1;
          NumInQueues++;
        }
        else {
      	  index = InQueue(queue, state);
      	  if (Value(queue, index) < newprio) {
      	    IncreaseValue(queue, index, newprio);
      	  }
        }
      }
      predecessorActions = predecessorActions->Next;
    }
  }
  
  printf("\n%d ( %f secs.)", 
      NumExpandedStates, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
  printf("  f: %f Converged!\n", Start->f);
  /*for (i = 0; i < gNumStates; i++) {
    printf("state: %d value: %f\n", i, StateIndex[i]->f);
  }
  printf("Goal: %d\n", Goal->StateNo);*/
  printf("Total number of InQueue operations: %d\n", NumInQueues);

}

 
 
