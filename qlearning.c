/* qlearning.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "track.h"
#include "graph.h"
#include "qlearning.h"
#include "backup.h"
#include "solve.h"
#include "math.h"

static struct StateNode* SimulateNextState(struct StateNode *currentState);

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;

/* Real Time Dynamic Programming 
   always starts at the Start state
*/
void QLearning(int nTrial)
{
  struct StateNode *state;
  struct ActionNode *action;
  double diff, maxdiff , val, wavg, weight, time;
  int i, steps;
  char c;
  struct timeval t;
  
  stateExpanded=0;
  weight = 0.05;
  wavg = Start->f;
  for (trial = 0; ; trial++)  {
    if ((trial % 1000) == 1 ) { 
      for (i = 0; i < gNumStates; i++)
        BackupusingQ(StateIndex[i]);
      BackupusingQ(Start);
      diff = Start->f - wavg;
      wavg += weight * diff;
      gettimeofday(&t, NULL);
	  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
	  printf("\n%5d ( %f secs.)  f: %f\tdiff: %f", trial, time, Start->f, diff);
      //printf("\n%5d ( %f secs.)  f: %f\tdiff: %f", trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, diff);
      if ((wavg > 0.1) && ((diff * diff) < gEpsilon)) {
        printf(" Converge\n");
        return;
      }
    }
    stateExpandedTrial = 0;
    state = Start;
    for (steps = 0; steps < 100; steps++) {
      if ((state->Terminal == 1) || (state->Terminal == 5))
        break; 
      stateExpanded++;
      action = state->BestAction;
      if (action)
        BackupQ(action);
      state = SimulateNextState(state);
    }
  }
  printf("\nQ-Learning done. \n");
  
}

static struct StateNode* SimulateNextState(struct StateNode *currentState)
{
  double r,p;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  greedyAction=currentState->BestAction;
  nextState=greedyAction->NextState;
  for(;;) {
    p = 0.0;
    r = drand48();
    for (nextState = greedyAction->NextState;
         nextState;
         nextState = nextState->Next) {
      p += nextState->Prob;
      if(r <= p ) return nextState->State;
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in SimulateNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}

