/* frtdp.c */
/* This file implements FRTDP algorithm by Smith & Simmons (AAAI06') */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "stack.h"
#include "backup.h"
#include "solve.h"
#include "frtdp.h"

#include <stdio.h>

/* functions */
static void trialRecurse(int s, double occupancy, int d, int depth);
void trackUpdateQuality(double q, int d, int depth);

/* global variables */
static double qprev, qcurr;
static int nprev, ncurr;
static float kd;

/* Focused Real Time Dynamic Programming 
   always starts at the Start state
*/
void FRTDP(int d, double epsilon)
{
  int depth;
  int trial_num = 0;
  int cont = 0;
  
  kd = 1.1;
  depth = d;
  //while ((StateIndex[0]->fprime - StateIndex[0]->f) > epsilon) {
  //while (Backup(StateIndex[0]) > epsilon) {
  while (cont < 1000) {
    if (Backup(StateIndex[0]) < epsilon)
      cont++;
    else
      cont = 0;
  
    printf("\n%f", StateIndex[0]->f);
    trial_num++;
    qprev = 0.0;
    qcurr = 0.0;
    nprev = 0;
    ncurr = 0;
    
    trialRecurse(0, 1, 0, depth);
    if ((qcurr / ncurr) > (qprev / nprev))
      depth = (int)depth * kd;
      
    printf("\n%d ( %f secs.)", 
	   trial_num, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    //printf("\n%f %f", StateIndex[0]->fprime, StateIndex[0]->f);  
  }
  
  Backup(Start);
  printf("\n%d ( %f secs.)  f: %f Converged!\n",
  trial_num, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
}

void trialRecurse(int s, double occupancy, int d, int depth)
{
  int succ, nexts = 0;
  double delta, tempw, maxw, quality, prob = 0.0, bestprob = 0.0;
  struct StateNode *state, *prevState, *succState, *nextState;
  struct StateDistribution *succStates;
  struct ActionNode *greedyAction, *prevAction, *action;
  struct StateDistribution *nextStates;
  struct ActionListNode  *prevActions, *actions;
    
  state = StateIndex[s];
  if (state->Terminal == 1)
    return;
  Backup(state);
  printf("\nState: %d %f", s, state->f);
  delta = state->fprime - state->f;
  
  greedyAction = state->BestAction;
  maxw = 0.0;
  for(nextStates = greedyAction->NextState; nextStates; nextStates = nextStates->Next) {
    nextState = nextStates->State;
    tempw = 0.0;
    for (prevActions = nextState->PrevAction; prevActions; prevActions = prevActions->Next) {
      prevAction = prevActions->Node;
      prevState = prevAction->PrevState;
      if (prevState->w > 0) {
        action = prevState->BestAction;
	for (succStates = action->NextState; succStates; succStates = succStates->Next) {
	  succState = succStates->State;
	  if ((succState->StateNo != s) && (succState->StateNo == nextState->StateNo)) {
	    prob = succStates->Prob;
	    tempw += prevState->w * prob;
	    break;
	  }
	}
      }
    }
    nextState->w = tempw;
    //printf("\n%d %f", nextState->StateNo, nextState->w);
    if (tempw >= maxw) {
      maxw = tempw;
      nexts = nextState->StateNo;
      bestprob = prob;
    }
  }
  quality = delta * occupancy;
  trackUpdateQuality(quality, d, depth);
  
  if ((delta < 0) || (d > depth))
    return;
  
  occupancy *= bestprob;      
  trialRecurse(nexts, occupancy, d+1, depth);
  //Backup(state);
  printf("\nState: %d %f", s, state->f);
}

void trackUpdateQuality(double q, int d, int depth)
{
  if (d > (depth / kd)) {
    qcurr += q;
    ncurr++;
  }
  else {
    qprev += q;
    nprev++;
  }
}
