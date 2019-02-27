/* tql.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "track.h"
#include "graph.h"
#include "tql.h"
#include "backup.h"
#include "solve.h"
#include "math.h"

static void inilearning(int nTrial);
static void acclearning(int nTrial);
static struct StateNode* SimulateNextState(struct StateNode*, int **successors, int **predecessors);
static void addSuccessor(int, int, int**, int**);
static void printSuccessors(int **successors, int **predecessors);
static void buildtop(int**, int**);
static void reachability(int s, int** successors);
static void dfs(int w, int** successors);
static void dfsR(int w, int** predecessors);

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;
static int scnt;
static int cnt;
static struct StateListNode **stateListNode;

int *id;
int *postI;
int *postR;
int *reachable;
int *reachableC;
int **successors;
int **predecessors;
  

/* Real Time Dynamic Programming 
   always starts at the Start state
*/
void TQL(int nTrial)
{
  inilearning(nTrial);
  acclearning(nTrial);
}

void inilearning(int nTrial)
{
  struct StateNode *state;
  struct ActionNode *action;
  double diff, maxdiff, val, wavg, time;
  int i, steps;
  char c;
  struct timeval t;
  
  successors = (int**)malloc(gNumStates * sizeof(int*));
  predecessors = (int**)malloc(gNumStates * sizeof(int*));
  for (i = 0; i < gNumStates; i++) {
    successors[i] = (int*)malloc((MAX_SUCCS + 1) * sizeof(int*));
    successors[i][0] = 0;
  }
  for (i = 0; i < gNumStates; i++) {
    predecessors[i] = (int*)malloc((MAX_SUCCS + 1) * sizeof(int*));
    predecessors[i][0] = 0;
  }
  stateExpanded = 0;
  wavg = Start->f;
  for (trial = 0; trial < nTrial; trial++)  {
    if ((trial % 1000) == 1 ) { 
      for (i = 0; i < gNumStates; i++)
        BackupusingQ(StateIndex[i]);
      BackupusingQ(Start);
      diff = Start->f - wavg;
      wavg += WEIGHT * diff;
      printf("\n%5d ( %f secs.)  f: %f\tdiff: %f",
           trial, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, diff);
      if ((wavg > 0.1) && (diff * diff) < 1000000 * gEpsilon) {
        printf("\n");
        //printSuccessors(successors, predecessors);
        buildtop(successors, predecessors);
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
      state = SimulateNextState(state, successors, predecessors);
    }
  }
}

static void acclearning(int nTrial)
{
  int round, i, j, steps;
  struct StateListNode *list;
  struct StateNode *state, *head;
  struct ActionNode *action;
  double diff = 0.0, maxdiff, val, wavg, time, residual, avgdiff, backupnum;
  struct timeval t;

  scnt = id[0] + 1;
  for (round = 1; round < scnt; round++) {
    if (reachableC[round]) {
      printf("\nround %d", round);
      list = stateListNode[round];      
      /* learning begin here */
      /*for (list = stateListNode[round];
           list;
           list = list->Next) {*/
		//residual = 1.0;
        for (i = 0; i < 200; i++) {
          state = list->Node;
          head = state;
		  backupnum = 0;
          for (steps = 0; steps < 100; steps++) {
            if ((state->Terminal == 1) || (state->Terminal == 5) || (id[state->StateNo] < id[head->StateNo]))
              break;
            stateExpanded++;
            action = state->BestAction;
            if (action)
              diff = BackupQ(action);
			backupnum++;
			avgdiff += diff;
            state = SimulateNextState(state, successors, predecessors);
          }
		  avgdiff /= backupnum;
		  //residual = 0.9 * residual + 0.1 * avgdiff;
		  if (avgdiff < 0.01)
		    break;
        }
      //}
    }
  }
  wavg = Start->f;
  for (j = 0; ; j++) { 
    state = Start;
    if (j % 1000 == 500) {
      BackupusingQ(Start);
      diff = Start->f - wavg;
      wavg += WEIGHT * diff;
      gettimeofday(&t, NULL);
	  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
	  printf("\n%5d ( %f secs.)  f: %f\tdiff: %f", j, time, Start->f, diff);
      //printf("\n%5d ( %f secs.)  f: %f\tdiff: %f", j, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, diff);
      if ((wavg > 0.1) && ((diff * diff) < gEpsilon)) {
        printf(" Converge\n");
        return;
      }
    }
    for (steps = 0; steps < 100; steps++) {
      if ((state->Terminal == 1) || (state->Terminal == 5))
        break;
      stateExpanded++;
      action = state->BestAction;
      if (action)
        BackupQ(action);
      state = SimulateNextState(state, successors, predecessors);
    }
  }
  for (j = 0; j < gNumStates; j++)
    BackupusingQ(StateIndex[j]);
  BackupusingQ(Start);
  printf("\n%3d ( %f secs.)  f: %f",
       round, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
  
}

static struct StateNode* SimulateNextState(struct StateNode *currentState, int** successors, int **predecessors)
{
  double r, p;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  greedyAction = currentState->BestAction;
  nextState = greedyAction->NextState;
  for(;;) {
    p = 0.0;
    r = drand48();
    for (nextState = greedyAction->NextState;
         nextState;
         nextState = nextState->Next) {
      p += nextState->Prob;
      if(r <= p ) {
        if (currentState->StateNo != -1)
          addSuccessor(currentState->StateNo, nextState->State->StateNo, successors, predecessors);
        return nextState->State;
      }
    }
  }
  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in SimulateNextState!");
  printf("\np=%f, r=%f\n",p,r);
  exit(0);
}

static void addSuccessor(int s1, int s2, int **successors, int **predecessors)
{
  int succs, predes, i;
  
  if ((successors[s1][0] == MAX_SUCCS) || (predecessors[s2][0] == MAX_SUCCS))
    return;
  for (i = 1; i <= successors[s1][0]; i++) {
    if (successors[s1][i] == s2)
      return;
  }
  successors[s1][0]++;
  succs = successors[s1][0];
  successors[s1][succs] = s2;
  predecessors[s2][0]++;
  predes = predecessors[s2][0];
  predecessors[s2][predes] = s1;
} 

static void printSuccessors(int **successors, int **predecessors)
{
  int i, j, succs, predes;
  for (i = 0; i < gNumStates; i++) {
    printf("\n%3d\t", i);
    succs = successors[i][0];
    printf("%3d ", succs);
    for (j = 1; j <= succs; j++)
      printf("%3d ", successors[i][j]);
  }
  for (i = 0; i < gNumStates; i++) {
    printf("\n%3d\t", i);
    predes = predecessors[i][0];
    printf("%3d ", predes);
    for (j = 1; j <= predes; j++)
      printf("%3d ", predecessors[i][j]);
  }
}

static void buildtop(int **successors, int **predecessors)
{
  int state, round;
  struct StateNode     *node;
  struct StateListNode *list, *new;
  struct ActionNode    *actionNode;
  
  id = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postI = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postR = (int*)malloc((unsigned)gNumStates * sizeof(int));
  reachable = (int*)malloc((unsigned)gNumStates * sizeof(int));

  for (state = 0; state < gNumStates; state++) {
    reachable[state] = 0;
  }
  reachability(0, successors);  
  /*for (state = 0; state < gNumStates; state++) {
    printf("%d\n", reachable[state]);
  }*/
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = 0; state < gNumStates; state++)
    if (id[state] == -1)
      dfsR(state, predecessors);
  for (state = 0; state < gNumStates; state++)
    postR[state] = postI[state];
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = gNumStates - 1; state >= 0; state--)
  //for (state = 0; state < gNumStates; state++)
    if (id[postR[state]] == -1) {
      dfs(postR[state], successors);
      scnt++;
    }
      
  printf("\nComponents: %d", scnt);
  // debugging  
  stateListNode = (struct StateListNode**)malloc(scnt * sizeof(struct StateListNode*));
  reachableC = (int*)malloc(scnt * sizeof(int));
  
  for (round = 0; round < scnt; round++) {
    stateListNode[round] = (struct StateListNode*)malloc(sizeof(struct StateListNode));
    stateListNode[round]->Node = NULL;
    stateListNode[round]->Next = NULL;
    reachableC[round] = 0;
  }
    
  for (state = gNumStates - 1; state >= 0; state--) {
    //printf("\n%d %d", state, id[state]);
    list = stateListNode[id[state]];
    node = StateIndex[state];
    if (reachable[state] && (successors[state][0] > 0))
      reachableC[id[state]] = 1;
    if (!list->Node) {
      list->Node = node;
    }
    else {
      new = (struct StateListNode*)malloc((unsigned)sizeof(struct StateListNode));
      new->Node = list->Node;
      new->Next = list->Next;
      list->Node = node;
      list->Next = new;
    }
  }
}

static void reachability(int s, int** successors)
{  
  int i, nodeNumber;
  reachable[s] = 1;
  for (i = 1; i < successors[s][0]; i++) {
    nodeNumber = successors[s][i];
    if (reachable[nodeNumber] == 0)
      reachability(nodeNumber, successors);
  }
}

static void dfs(int w, int** successors)
{
  int i, nodeNumber;
  
  id[w] = scnt;
  for (i = 1; i <= successors[w][0]; i++) {
    nodeNumber = successors[w][i];
    if (id[nodeNumber] == -1)
      dfs(nodeNumber, successors);
  }
  postI[cnt++] = w;
}
 
static void dfsR(int w, int** predecessors)
{
  int i, nodeNumber;
  
  id[w] = scnt;
  for (i = 1; i <= predecessors[w][0]; i++) {
    /* take the reverse graph instead */
    nodeNumber = predecessors[w][i];
    if (id[nodeNumber] == -1)
      dfsR(nodeNumber, predecessors);
  }
  postI[cnt++] = w;
}

