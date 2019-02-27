/* fdp.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "fdp.h"
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

void FDP(int flag)
{
  struct StateListNode *AncestorList;
  struct StateNode *node, *predecessor;
  struct StateQueue *queue;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  int state, i;
  double change, newprio;
  float elapsedTime;

  if (flag == 1)
    CreateHeuristic();
  
  inQueue = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (i = 0; i < gNumStates; i++)
    inQueue[i] = 0;
  NumExpandedStates = 0;
  NumInQueues = 0;
  queue = (struct StateQueue*)CreateQueue(gNumStates);
  for (i = 0; i < gNumStates; i++)
    StateIndex[i]->f = MAX;
  Goal->f = 0.0;
  Start->f = MAX;

  InsertQueue(queue, Goal->StateNo, 1.0);
  //PrintQueue(queue);
  inQueue[Goal->StateNo] = 1;

  //while (!IsEmpty(queue)) {
  while (!IsEmpty(queue) && (QueueTopValue(queue) > (1.0 / Start->f))) {
    //PrintQueue(queue);
    if ((NumExpandedStates % 1000) == 0) {
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
    if (flag == 0)
      predecessorActions = node->PrevAction;
    else 
      predecessorActions = node->BestPrevActions;
    while(predecessorActions) {
      predecessorAction = predecessorActions->Node;
      predecessor = predecessorAction->PrevState;
      state = predecessor->StateNo;
      change = FBackup(predecessor);
      newprio = predecessor->h + predecessor->hg;
      if ((change > 0.01) && (!inQueue[state]) && (state != -1)) {
      //if ((change > gEpsilon) && (!inQueue[state]) && (state != -1)) {
      //if ((change > 1000 * gEpsilon) && (!inqueue(queue, state)) && (state != -1)){
      //if ((change > 0.1) && (!InQueue(queue, state)) && (state != -1)) {
        InsertQueue(queue, state, 1.0 / newprio);
        inQueue[state] = 1;
        NumInQueues++;
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

 
