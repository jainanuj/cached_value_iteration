/* lrtdp.c */
/* This file implements LRTDP algorithm by Bonet & Geffner (ICAPS-03) */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "stack.h"
#include "backup.h"
#include "solve.h"
#include "lrtdp.h"

/* forward declaration of static functions */
static struct StateNode* SimulateNextState(struct StateNode *currentState);

//Added by Anuj
static int lrtdptrial(int state, int trial_num, double epsilon);


/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;

int *solved;
int *invisited, *inopen, *inclosed;


void LRTDP(int s0, double epsilon)
{
  struct StateNode *state;
  double diff, maxdiff, val, time, threshold = 300.0;
  struct intStack *stack;
  int s, trial_num;
  
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  inopen = (int*)malloc((unsigned)gNumStates * sizeof(int));
  inclosed = (int*)malloc((unsigned)gNumStates * sizeof(int));
  invisited = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++) {
    solved[s] = 0;
    inopen[s] = 0;
    inclosed[s] = 0;
    invisited[s] = 0;
  }

  trial_num = 0;
  while ( !solved[gInitialState] ) {
    lrtdptrial(gInitialState, ++trial_num, epsilon);
    time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
    if ( time > threshold )
      break;
  }
    
  Backup(Start);
  printf("\n%d ( %f secs.)  f: %f Converged!\n", trial_num, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);

}

static struct StateNode* SimulateNextState(struct StateNode *currentState)
{
  double r,p;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  greedyAction = currentState->BestAction;
  nextState = greedyAction->NextState;
  for(;;) {
    p=0.0;
    r=drand48();
    for(nextState=greedyAction->NextState;
	nextState;
	nextState=nextState->Next) {
      p += nextState->Prob;
      if( r <= p ) return nextState->State;
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in SimulateNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}

int lrtdptrial(int state, int trial_num, double epsilon)
{
  struct intStack* visited;
  struct StateNode* node;
  int s, steps = 0;

/*  
  if(trial_num % 1000 == 1 ) { 
    Backup(Start);
    printf("\n%d ( %f secs.)  f: %f", trial_num, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
    fflush(0);
  }
*/  

  visited = CreateStack();
  for (s = 0; s < gNumStates; s++) {
    invisited[s] = 0;
  }

//  while ( !solved[state] ) {
  while ( (!solved[state]) && (steps <= 500) ) {
    ++steps;
    if ( !invisited[state] ) {
      push(visited, state);
      invisited[state] = 1;
    }
    node = StateIndex[state];
//printf("\n%d %d", state, node->Terminal);
//fflush(0);
    if ((node->Terminal == 1) || (node->Terminal == 5))
      break;      
    Backup(node);
    node = SimulateNextState(node);
    state = node->StateNo;
  }

  while( !empty(visited) ) {
    s = pop(visited);
    node = StateIndex[s];
    if ( !checksolved(s, epsilon) )
      break;
  }

  deletestack(visited);
  return 1;
}

int checksolved(int state, double epsilon)
{
  int rv, s;
  struct intStack *open, *closed;
  struct StateNode *node;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;

  // terminal states are solved states 
  if ((StateIndex[state]->Terminal == 1) || (StateIndex[state]->Terminal == 5)) {
    solved[state] = 1;
    return 1;
  }

  rv = 1;
  open = CreateStack();
  closed = CreateStack();
  
  if ( !solved[state] ) {
    push(open, state);
    inopen[state] = 1;
  }

  while ( !empty(open) ) {
    s = pop(open);
    inopen[s] = 0;
    push(closed, s);
    inclosed[s] = 1;
    
    node = StateIndex[s];
    if (Backup(node) > epsilon) {
      rv = 0;
      continue;
    }
      
    // expand state
    greedyAction = node->BestAction;
    if ( !greedyAction )
      continue;
    for(nextState = greedyAction->NextState;
	nextState;
	nextState = nextState->Next) {
      node = nextState->State;
      s = node->StateNo;
//      if ( (!solved[s]) && (!instack(closed, s)) && (!instack(open, s)) ) {
      if ( (!solved[s]) && (!inopen[s]) && (!inclosed[s]) ) {
        push(open, s);
        inopen[s] = 1;
      }
    }
  }
  
  if (rv == 1) {
    // label relevant states
    while ( !empty(closed) ) {
      s = pop(closed);
      inclosed[s] = 0;
      solved[s] = 1;
    }
  }
  else {
    // update states with residuals and ancestors
    while ( !empty(closed) ) {
      s = pop(closed);
      inclosed[s] = 0;
      node = StateIndex[s];
      Backup(node);
    }
  }

  deletestack(open);
  deletestack(closed);
  return rv;
}
