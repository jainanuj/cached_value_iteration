/* atvi.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "track.h"
#include "graph.h"
#include "atvi.h"
#include "backup.h"
#include "solve.h"
#define MAX_ITER   1000

extern void viter(struct StateListNode*, int);


static void initlearning(int nTrial);
static void initsearch(int nTrial);
static void accplaning();
static void expandsolution(struct StateNode*, int**, int**, int record);
static struct StateNode* SimulateNextState(struct StateNode*, int **successors, int **predecessors, int record);
static void addSucc(int, int);
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
void ATVI()
{
  // number of initial trials
//  int nTrials = 10000;
//  initlearning(nTrials);
  int nTrials = 100;
  initsearch(nTrials);
  accplaning();
}

void initlearning(int nTrials)
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
  for (trial = 0; trial < nTrials; trial++)  {
    state = Start;
    for (steps = 0; steps < 10000; steps++) {
      if ((state->Terminal == 1) || (state->Terminal == 5))
        break; 
      stateExpanded++;
      if (trial % 10 == 0)
        Backup(state);
      state = SimulateNextState(state, successors, predecessors, 1);
    }
  }
  printf("\nState expanded: %d", stateExpanded);
  printf("\nInitial trials( %f secs.)  f: %f", time, Start->f);
  buildtop(successors, predecessors);
}

void initsearch(int nTrials)
{
  struct StateNode *state, *node;
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
  for (trial = 0; trial < nTrials; trial++)  {
    int record = 1;
    if (trial > .9 * nTrials)
      record = 1;
//    printf("\nTrial %d record %d expanded %d", trial, record, stateExpanded);
//    fflush(0);
    expandsolution(Start, successors, predecessors, record);
  }

  printf("\nState expanded: %d", stateExpanded);
  gettimeofday(&t, NULL);
  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
  printf("\nInitial trials( %f secs.)  f: %f", time, Start->f);
//  printSuccessors(successors, predecessors);
  buildtop(successors, predecessors);
}

static void accplaning()
{
  int round, i, j, steps;
  struct StateListNode *list;
  struct StateNode *state, *head;
  struct ActionNode *action;
  double diff, maxdiff, val, wavg, time, residual, avgdiff, backupnum;
  struct timeval t;

  scnt = id[0] + 1;
  for (round = 1; round < scnt; round++) {
    if (reachableC[round]) {
      list = stateListNode[round];
      viter(list, MAX_ITER);
      gettimeofday(&t, NULL);
      time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
      printf("\n%5d ( %f secs.)  f: %f", round, time, Start->f);
    }
  }
  Backup(Start);
  gettimeofday(&t, NULL);
  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
  printf("\n%5d ( %f secs.)  f: %f Converged!", round, time, Start->f);
  printf("\n"); 
}

static void expandsolution(struct StateNode* node, int **successors, int **predecessors, int record)
{
  struct StateDistribution *successor;

  if (node->Terminal == 5)
    return;
  
  if (node->Expanded == 0) 
    Backup(node);
  ++stateExpanded;
  
  /* Assume successor states are sorted in order of probability */
  for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next) {
    
    /* Stop if already visited this iteration or goal state */
    if ((successor->State->Expanded < trial) && (successor->State->Terminal != 1)) {
      /* If already expanded, just mark as visited this iteration */
      successor->State->Expanded = trial;
      if ( (record) && (node->StateNo != -1) ) {
//        printf("\nAdd");
        addSuccessor(node->StateNo, successor->State->StateNo, successors, predecessors);
//        addSucc(node->StateNo, successor->State->StateNo);
      }
      expandsolution(successor->State, successors, predecessors, record);
    }
  }
  Backup(node);
}

static struct StateNode* SimulateNextState(struct StateNode *currentState, int **successors, int **predecessors, int record)
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
        if ( (record) && (currentState->StateNo != -1) )
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

static void addSucc(int s1, int s2)
{
  int succs, predes, i;
  
  if ((successors[s1][0] == MAX_SUCCS) || (predecessors[s2][0] == MAX_SUCCS))
    return;
  for (i = 1; i <= successors[s1][0]; i++) {
    if (successors[s1][i] == s2)
      return;
  }
  succs = ++successors[s1][0];
  successors[s1][succs] = s2;
  predes = ++predecessors[s2][0];
  predecessors[s2][predes] = s1;
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
  succs = ++successors[s1][0];
  successors[s1][succs] = s2;
  predes = ++predecessors[s2][0];
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
    
//  for (state = gNumStates - 1; state >= 0; --state) {
  for (state = 0; state < gNumStates; ++state) {
    //printf("\n%d %d", state, id[state]);
    list = stateListNode[id[state]];
    node = StateIndex[state];
    // ignore unreachable and deadend states
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
/*
** mark all reachable states by setting "reachable" to be 1
*/
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

