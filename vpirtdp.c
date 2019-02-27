/* vpirtdp.c */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "backup.h"
#include "solve.h"
#include "vpirtdp.h"
#include "brtdp.h"
#include "stack.h"
#include "statequeue.h"

/* forward declaration of static functions */
static struct StateNode* VPIRTDPNextState(struct StateNode *currentState);

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;

/* Real Time Dynamic Programming 
   always starts at the Start state
*/
void VPIRTDP(int nTrial, double largest_diff)
{
  struct StateNode *state;
  double diff, maxdiff, val, time, threshold = 300.0;
  int k;

//  CreateBetterHeuristic();
//  nTrial = 1;
  for(trial = 1; trial <= nTrial; ++trial)  {
//    printf("\n%5d ( %f secs.)  f: %f %f %f", trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, Start->fprime, Start->fprime - Start->f);
    struct intStack *stateStack = CreateStack();
    state = Start;
    for(k = 0; k < 10000; ++k) {
//    for(k = 0; k < 10; ++k) {
//printf("\nstate %d", state->StateNo);
      if ( state != Start )
        push(stateStack, state->StateNo);
      if ( (state->Terminal == 1) || (state->Terminal == 5) )
        break; 
      state->Expanded++;
//      BackupTwo(state);
      double diff = state->fprime - state->f;

//      if (diff > largest_diff * 0.95)
      double r = drand48();
      if ( (state->StateNo <= 0) || (r < 1e-3) ) {
//printf("\nhere1");
//fflush(0);
        state = BRTDPNextState(state);
      }
      else {
//printf("\nhere2");
//fflush(0);
        state = VPIRTDPNextState(state);
      }
    }

    while( !empty(stateStack) ) {
      int state_no = pop(stateStack);
      BackupTwo(StateIndex[state_no]);
    }
//    BackupTwo(Start);
    deletestack(stateStack);

    if ( (Start->fprime - Start->f) < 2 * gEpsilon )
//    if ( (Start->fprime - Start->f) < gEpsilon )
      break;
    time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
    if ( time > threshold )
      break;
  }
  printf("\nVPIRTDP done. \n");
  printf("\n%d ( %f secs.)  f: %f Converged!\n", trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  
}

static struct StateNode* VPIRTDPNextState(struct StateNode *currentState)
{
  double r, p, B = 0.0;
  struct ActionNode *greedyAction, *action;
  struct StateDistribution *nextState1, *nextState2;
  struct StateNode *state1, *state2;
  struct ActionListNode *actions;

  // recompute the Q values
  actions = currentState->Action;
  for ( action = actions->Node; actions; actions = actions->Next ) {
    QValue(action);
  }
  BestAction(currentState);
  greedyAction = currentState->BestAction;
  nextState1 = greedyAction->NextState;

  for (nextState1 = greedyAction->NextState; nextState1; nextState1 = nextState1->Next) {
//    state1->VPIheuristic = 0;
  }

  for (nextState1 = greedyAction->NextState; nextState1; nextState1 = nextState1->Next) {

    // compute the VPI gain
    double lower, upper, lowerQ, upperQ;
    // preparation
    state1 = nextState1->State;
    lower = state1->fWeight;
    upper = state1->fprime;
    lowerQ = greedyAction->q;
    upperQ = greedyAction->q + (upper - lower) * nextState1->Prob;

//printf("\n lower %lf upper %lf", lowerQ, upperQ);
//fflush(0);
    // consider all other actions
    actions = currentState->Action;
    double biggest_area = 0.0;
    for ( action = actions->Node; actions; actions = actions->Next ) {
      double lowerQtemp = -1.0, upperQtemp = -1.0;
      for (nextState2 = action->NextState; nextState2; nextState2 = nextState2->Next) {
        state2 = nextState2->State;
        if ( state1 == state2 ) {
          lowerQtemp = action->q;
          upperQtemp = action->q + (upper - lower) * nextState2->Prob;
          break;
        }
      }
      if ( lowerQtemp < 0 ) {
        lowerQtemp = action->q;
        upperQtemp = action->q;
      }
//printf("\n  lower %lf upper %lf", lowerQtemp, upperQtemp);
//fflush(0);
      // compute the gain
      if (upperQtemp < upperQ) { // gain is positive
        double diff1, diff2, length, area;
        diff1 = upperQ - upperQtemp;
        diff2 = lowerQtemp - lowerQ;
        length = diff1 * (upper - lower) / (diff1 + diff2);
        area = diff1 * length / 2;
        if (area > biggest_area)
          biggest_area = area;
//printf("\n  length %lf height %lf", length, diff1);
//fflush(0);
      }
    }
//printf("\nstate %d heuristic %lf", state1->StateNo, biggest_area);

    // set state VPI heuristics
    state1->VPIheuristic += biggest_area;
    B += biggest_area;
//printf("\nstate %d", state1->StateNo);
//printf("\n %f %f", state1->VPIheuristic, B);
//fflush(0);
  }
  if ( B < ((Start->fprime - Start->fWeight) / 10) )
    return Goal;

  for(;;) {
    p = 0.0;
    r = drand48();
    for (nextState1 = greedyAction->NextState; nextState1; nextState1 = nextState1->Next) {
      state1 = nextState1->State;
      p += state1->VPIheuristic / B;
//printf("\nstate %d", state1->StateNo);
//printf("\n %f %f", state1->VPIheuristic, B);
      if( r <= p )
        return state1;
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in VPIRTDPNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}


