/* rtdp.c */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "backup.h"
#include "solve.h"
#include "brtdp.h"
#include "stack.h"
#include "statequeue.h"

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;

/* Real Time Dynamic Programming 
   always starts at the Start state
*/
void BRTDP(int nTrial)
{
  struct StateNode *state;
  double diff, maxdiff, val, time, threshold = 300.0;
  int k;

  for(trial = 1; trial <= nTrial; ++trial)  {
    //printf("\n%5d ( %f secs.)  f: lower %f upper %f", trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, Start->fprime);
    //fflush(0);
    struct intStack *stateStack = CreateStack();
    state = Start;
    for(k = 0; k < 10000; ++k) {
      //printf("\nstate %d", state->StateNo);
      //fflush(0);
      if ( state != Start )
        push(stateStack, state->StateNo);
      if ( (state->Terminal == 1) || (state->Terminal == 5) )
        break; 
      state->Expanded++;
//      BackupTwo(state);
      state = BRTDPNextState(state);
    }
//printf("\nsearch finished");
//fflush(0);

    while( !empty(stateStack) ) {
      int state_no = pop(stateStack);
      BackupTwo(StateIndex[state_no]);
    }
//    BackupTwo(Start);
//printf("\nbackward backup finished");
//fflush(0);
    deletestack(stateStack);

    if ( (Start->fprime - Start->f) < 2 * gEpsilon )
//    if ( (Start->fprime - Start->f) < gEpsilon )
      break;
    time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
    if ( time > threshold )
      break;
  }
  printf("\nBRTDP done. \n");
  printf("\n%d ( %f secs.)  f: %f Converged!\n", trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  
}

struct StateNode* BRTDPNextState(struct StateNode *currentState)
{
  double r, p, B = 0.0;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  greedyAction = currentState->BestAction;
  nextState = greedyAction->NextState;
  for (nextState = greedyAction->NextState; nextState; nextState = nextState->Next) {
//printf("\n next state: %d", nextState->State->StateNo);
    B += nextState->State->fprime - nextState->State->fWeight;
  }
  if ( B < ((Start->fprime - Start->fWeight) / 10) )
    return Goal;
  for(;;) {
    p = 0.0;
    r = drand48();
    for (nextState = greedyAction->NextState; nextState; nextState = nextState->Next) {
      p += (nextState->State->fprime - nextState->State->fWeight) / B;
      if( r <= p )
        return nextState->State;
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in BRTDPNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}

void CreateBetterHeuristic(void)
/****** This doesn't yet work correctly for discounted MDPs ******/
{
  struct StateListNode *CurrentList, *NewList, *node;
  struct StateNode *snode, *stateNode;
  struct ActionListNode *prev, *actionList;
  struct StateDistribution *nextState;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode, *oldBestAction;
  int state, numReachable, consistent = 1;
  double max_value = 9999.99, temp, cur_best;

  struct StateQueue *queue = CreateQueue(gNumStates);
  int *inqueue = (int*) malloc(gNumStates*sizeof(int));
  int *outqueue = (int*) malloc(gNumStates*sizeof(int));
  for (state = 0; state < gNumStates; state++) {
    snode = StateIndex[state];
    if (snode->Terminal == 1) {
      InsertQueue(queue, state, 1.0);
      inqueue[state] = 1;
      snode->goalProb = 1.0;
    }
    else
      inqueue[state] = 0;
    outqueue[state] = 0;
  }
  while ( !IsEmpty(queue) ) {
    int stateNo = OutQueue(queue);
//printf("\nOutqueue: %d", stateNo);
    outqueue[stateNo] = 1;
    snode = StateIndex[stateNo];
    if ( snode->Pi ) {
      snode->goalProb = snode->Pi->goalProb;
      snode->w = snode->Pi->w;
//printf("\n %d %f", stateNo, snode->w);
    }

    // traverse its predecessors
    for (prev = snode->PrevAction; prev && consistent; prev = prev->Next) {
      stateNode = prev->Node->PrevState;
      int nodeNo = stateNode->StateNo;
      if ( outqueue[nodeNo] )
        continue;
      double prob, pri, oripri;
      for (nextState = prev->Node->NextState; nextState; nextState = nextState->Next) {
        if (nextState->State == snode) {
          prob = nextState->Prob;
          break;
        }
      }
      prev->Node->w += snode->w * prob;
      prev->Node->goalProb += snode->goalProb * prob;
      pri = prev->Node->goalProb;
      oripri = StateValue(queue, nodeNo);
//printf("\n state %d prob %f pri %f oripri %f", nodeNo, prob, pri, oripri);
      if (pri > oripri) {
        stateNode->Pi = prev->Node;
        int index = InQueue(queue, nodeNo);
//printf("\n state %d index %d", nodeNo, index);
        if (index == -2)
          InsertQueue(queue, nodeNo, pri);
        else
          IncreaseValue(queue, index, pri);
      }
    }
  }

  // calculate upper bound
  int i;
  double lambda = 0.0;
  for (i = 0; i < gNumStates; i++) {
    double min_lambda = 9999.99;
    snode = StateIndex[i];
    if ( (snode->Terminal == 1) || (snode->Terminal == 5) )
      continue;
    for (actionListNode = snode->Action; actionListNode; actionListNode = actionListNode->Next) {
      actionNode = actionListNode->Node;
      double sumw = 0.0, sump = 0.0, local_lambda;
      for (nextState = actionNode->NextState; nextState; nextState = nextState->Next) {
        sumw += nextState->Prob * nextState->State->w;
        sump += nextState->Prob * nextState->State->goalProb;
      }
printf("\n  %f %f", sumw, snode->w);
      if (snode->goalProb < sump)
        local_lambda = 0.0;
      else if (1.0 + sumw + 1e-10 < snode->w)
        local_lambda = 9999.99;
      else
        local_lambda = (1.0 + sumw - snode->w) / (sump - snode->goalProb);
      if (local_lambda < min_lambda)
        min_lambda = local_lambda;
    }
printf("\n min_lambda %f lambda %f", min_lambda, lambda);
    if (min_lambda > lambda)
      lambda = min_lambda;
  }
  lambda = 0.0;
printf("\n lambda %f", lambda);

  for (i = 0; i < gNumStates; i++) {
    snode = StateIndex[i];
    if ( snode-> Terminal != 5 )
      snode->fprime = snode->w + (1-snode->goalProb) * lambda;
//printf(" %d %f", snode->fprime);
  }
return;
  

  for (i = 0; i < gNumStates; i++) {
    StateIndex[i]->f *= gHDiscount;
    StateIndex[i]->h *= gHDiscount;
    gInitValueSum += StateIndex[i]->f;
    if (StateIndex[i]->f > StateIndex[i]->fprime + 1e-6)
      printf("\nInconsistency at state %d %f %f", i, StateIndex[i]->f, StateIndex[i]->fprime);
    if ((StateIndex[i]->Terminal == 5) && (StateIndex[i]->f < max_value - 1e-6) )
      printf("\nError with deadend state value at state %d %f %f", i, StateIndex[i]->f, StateIndex[i]->fprime);
  }

  DeleteStateList(CurrentList);
  printf("\nNumber of goal reachable states: %d", numReachable);

  if (StateIndex[gInitialState]->f == max_value) {
    printf("\nInitial state cannot reach goal");
    gQuitFlag = 1;
  }
}


