/* rtdp.c */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "backup.h"
#include "solve.h"
#include "rtdp.h"

#include <stdio.h>

/* forward declaration of static functions */
static struct StateNode* SimulateNextState(struct StateNode *currentState);

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;

/* Real Time Dynamic Programming 
   always starts at the Start state
*/
void RTDP(int nTrial)
{
  struct StateNode *state;
  double diff,maxdiff,val;
  stateExpanded=0;
  for(trial = 0; trial<nTrial;trial++)  {
    if(trial%50==1 ) { 
      printf("\n%5d ( %f secs.)  f: %f",
	     trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
    }
    stateExpandedTrial=0;
    state=Start;
    for(;;) {
      if(state->Terminal == 1) break; 
      state->Expanded++;
      Backup(state);
      state = SimulateNextState(state);
    }
  }
  printf("\nRTDP done. \n");
  
}

static struct StateNode* SimulateNextState(struct StateNode *currentState)
{
  double r,p;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  greedyAction=currentState->BestAction;
  nextState=greedyAction->NextState;
  for(;;) {
    p=0.0;
    r=drand48();
    for(nextState=greedyAction->NextState;
	nextState;
	nextState=nextState->Next) {
      p += nextState->Prob;
      if(r <= p ) return nextState->State;
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in SimulateNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}

