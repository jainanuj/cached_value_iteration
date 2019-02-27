/* pvi.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "stack.h"
#include "graph.h"
#include "lao.h"
#include "backup.h"
#include "solve.h"
#include "statequeue.h"

//#include "lrtdp.h"

#define MAX 999.99
/* Static variables */
static int Iteration;
static int NumExpandedStatesIter; // number of states expanded this iteration
static int NumAncestorStatesIter; 
static int NumExpandedStates; // number of states expanded since start of alg
static int NumSolutionStates;
static struct StateListNode *ExpandedStateList;
static int CountBackups;
static double Residual;
static int ExitFlag;

int *solved;
int flag = 0;
float elapsedTime;

/* forward function declarations */
static void   ExpandSolution(struct StateNode *node);
static void   pExpandSolution(struct StateNode *node);
static void   n_pExpandSolution(struct StateNode *node);
static double   bpExpandSolution(struct StateNode *node);
static void   ExpandNode(struct StateNode *node);
static struct StateListNode *FindAncestors(struct StateListNode *ExpandedStateList);
static void   DisplaySolution( void );
static int    ConvergenceTest( void );
static void   ConvergenceTestRecursive( struct StateNode *node);
static int    rConvergenceTest( double* );
static void   rConvergenceTestRecursive( struct StateNode *node);
//static struct StateListNode *DisplaySolutionRecursive(struct StateNode *node);
static void DisplaySolutionRecursive(struct StateNode *node);
static void   MultipleUpdate( struct StateNode *node);

static int check(int state, double epsilon);


void PVI(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;
  
  double f;
  double* fp;
  int i, s;
  f = -1.0;
  fp = &f;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;

  /*NumExpandedStates = 0;
  NumExpandedStatesIter = 0;
  for (i = 0; i < gNumStates; i++) {
    StateIndex[i]->f = MAX;
    StateIndex[i]->g = MAX;
  }
  Goal->f = 0.0;
  Goal->g = 0.0;
  Start->f = MAX;
  Start->g = MAX;*/
  CreateHeuristic();
    
  for (Iteration = 1; ;Iteration++) {

    printf("\n%d ( %f secs.)", 
           Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    
    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    ExpandedStateList = CreateStateList();
    //pExpandSolution(Goal);
    //n_pExpandSolution(Goal);
    
    /* Call convergence test if solution graph has no unexpanded tip states */
    if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp)) {
    //if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp) && Iteration >= 100) {
      printf("  Converged!");
      break;
    }

    DeleteStateList(ExpandedStateList);
    /* NumSolutionStates includes GOAL and WALL */
    printf("  ExpandedStates: %d", NumExpandedStatesIter);
  }
  printf("\nf: %f", Start->f);
  /*for (i = 0; i < gNumStates; i++) {
    printf("\nValue of state %d: %f", i, StateIndex[i]->f);
  }*/
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
         NumSolutionStates, NumExpandedStates);
}

void NPVI(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;
  
  double f;
  double* fp;
  f = -1.0;
  fp = &f;
  int s;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;

  NumExpandedStates = 0;
  /* First expand nodes along most likely path to goal */
  NumExpandedStatesIter = 0;
  for (node = Start; 
       (node->Terminal != 1) && (node->Expanded < 1); 
       node = node->BestAction->NextState->State)
    {
      node->Expanded = 1;
      //printf("\nhere %d", node->StateNo);
      //printf("\nExpand node %d", node->StateNo);
      Backup( node );
      NumExpandedStatesIter++;
      NumExpandedStates++;
    }
  printf("\nNum expanded states along most probable path to goal: %d", NumExpandedStatesIter);
  
  //printf("\n f: %f", Start->f);

  for (Iteration = 1; ;Iteration++) {

    printf("\n%d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    
    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    ExpandedStateList = CreateStateList();
    //pExpandSolution(Goal);
    n_pExpandSolution(Goal);
    
    /* Call convergence test if solution graph has no unexpanded tip states */
    if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp)) {
    //if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp) && Iteration >= 100) {
      printf("  Converged!");
      break;
    }

    DeleteStateList(ExpandedStateList);
    /* NumSolutionStates includes GOAL and WALL */
    printf("  ExpandedStates: %d", NumExpandedStatesIter);
  }
  /* write solution to file? */
  /*
  DisplaySolution();
  */
  printf("\nf: %f", Start->f);
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
	 NumSolutionStates, NumExpandedStates);
}

void BPVI(void)
{
  struct StateNode *node;
  
  double f;
  double* fp;
  double residual;
  f = -1.0;
  fp = &f;
  int s;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;

  CreateHeuristic(); 

  for (Iteration = 1; ;Iteration++) {

    printf("\n%d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    
    /* Expand best partial solution */
    residual = bpExpandSolution(Goal);
    
    if (residual < gEpsilon) {
      printf("  Converged!");
      break;
    }

    /* NumSolutionStates includes GOAL and WALL */
    printf("  Residual: %f", residual);
  }
  printf("\nf: %f", Start->f);
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
	 NumSolutionStates, NumExpandedStates);
}

void MBPVI(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;
  struct StateNode *predecessor;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  struct StateList *list;
  
  double f;
  double* fp;
  double residual;
  f = -1.0;
  fp = &f;
  int s, state, firstBackup;
  int* inQueue;
  
  CreateHeuristic();

  //MultipleUpdate(Goal);  
  list = (struct StateList*)CreateList(gNumStates + 1);
  inQueue = (int*)malloc((gNumStates + 1) * sizeof(int));
  for (state = 0; state <= gNumStates; state++) {
    inQueue[state] = 0;
  }
  firstBackup = 1;
  AppendState(list, Goal->StateNo);
  while (!IsEmptyList(list)) { 
    state = OutList(list);
    if (state >= 0) {
      node = StateIndex[state];
      inQueue[state] = 0;
    }
    else {
      node = Start;
      inQueue[gNumStates] = 0;
    }    
    residual = Backup(node);
    if (firstBackup) {
      firstBackup = 0;
      residual = 1.0;
    }
    if ((numBackups % 1000) == 0) {
      elapsedTime = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
      if (elapsedTime > 3600) {
        return;
      }
      printf("\n%ld ( %f secs.)", 
            numBackups, elapsedTime);
      printf("  f: %f", Start->f);
    }
    //if (residual > gEpsilon) {
    //if (residual > 0) {
      predecessorActions = node->BestPrevActions;
      //predecessorActions = node->PrevAction;
      while (predecessorActions) {
        predecessorAction = predecessorActions->Node;
        predecessor = predecessorAction->PrevState;
        state = predecessor->StateNo;
        if (state >= 0) {
          if (!inQueue[state]) {
            AppendState(list, state);
            inQueue[state] = 1;
          }
        }
        else {
          if (!inQueue[gNumStates]) {
            AppendState(list, state);
            inQueue[gNumStates] = 1;
          }
        }
        predecessorActions = predecessorActions->Next;
      }
    //}
  }

  printf("\nf: %f", Start->f);
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
	 NumSolutionStates, NumExpandedStates);
}

void MultipleUpdate(struct StateNode *node)
{
  struct ActionListNode *predecessorActions;
  struct ActionNode *predecessorAction;
  struct StateNode *predecessor;
  double Diff;
  if (node == Goal)
    Diff = 1.0;
  else
    Diff = Backup(node);
  if (Diff > gEpsilon) {
    predecessorActions = node->BestPrevActions;
    while (predecessorActions) {
      predecessorAction = predecessorActions->Node;
      predecessor = predecessorAction->PrevState;
      MultipleUpdate(predecessor);
      predecessorActions = predecessorActions->Next;
    }
  }

}  


void LPVI(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;
  int s;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;
  
  double f;
  double* fp;
  f = -1.0;
  fp = &f;

  CreateHeuristic();
  
  for (Iteration = 1; ;Iteration++) {

    printf("\n%d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    
    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    flag = 1;
    ExpandedStateList = CreateStateList();
    //pExpandSolution(Goal);
    
    /* Call convergence test if solution graph has no unexpanded tip states */
    if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp)) {
    //if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp) && Iteration >= 100) {
      printf("  Converged!");
      break;
    }

    DeleteStateList(ExpandedStateList);
    /* NumSolutionStates includes GOAL and WALL */
    printf("  ExpandedStates: %d", NumExpandedStatesIter);
  }
  /* write solution to file? */
  /*
  DisplaySolution();
  */
  printf("\nf: %f", Start->f);
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
	 NumSolutionStates, NumExpandedStates);
}

void LBPVI(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;
  int s;
  solved = (int*)malloc((unsigned)gNumStates * sizeof(int));
  for (s = 0; s < gNumStates; s++)
    solved[s] = 0;
  
  double f;
  double* fp;
  f = -1.0;
  fp = &f;

  NumExpandedStates = 0;
  /* First expand nodes along most likely path to goal */
  NumExpandedStatesIter = 0;
  for (node = Start; 
       (node->Terminal != 1) && (node->Expanded < 1); 
       node = node->BestAction->NextState->State)
    {
      node->Expanded = 1;
      Backup( node );
      NumExpandedStatesIter++;
      NumExpandedStates++;
    }
  printf("\nNum expanded states along most probable path to goal: %d", NumExpandedStatesIter);
  
  //Backup(StateIndex[7198]);
  for (Iteration = 1; ;Iteration++) {

    printf("\n%d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
    
    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    flag = 1;
    ExpandedStateList = CreateStateList();
    bpExpandSolution(Goal);
    
    /* Call convergence test if solution graph has no unexpanded tip states */
    if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp)) {
    //if ((NumExpandedStatesIter == 0) && rConvergenceTest(fp) && Iteration >= 100) {
      printf("  Converged!");
      break;
    }

    DeleteStateList(ExpandedStateList);
    /* NumSolutionStates includes GOAL and WALL */
    printf("  ExpandedStates: %d", NumExpandedStatesIter);
  }
  /* write solution to file? */
  /*
  DisplaySolution();
  */
  printf("\nf: %f", Start->f);
  printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
	 NumSolutionStates, NumExpandedStates);
}

void pExpandSolution(struct StateNode *node)
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */

{
  struct StateNode *predecessor;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  int oldindex;
  double residual = 999.99;

  /* If not yet expanded, expand here.
     Then consider successor nodes for expansion. */
  if (node->Expanded == 0) {
  //if (node->Expanded <= Iteration) {
    ExpandNode(node);
    if((gBackupMethod == combine) && !solved[node->StateNo]) {
      //residual = FBackup(node);
      residual = Backup(node);
    }
  }
  
  /* added for labelling */
  if ((gMethod == lpvi) && (residual < gEpsilon) && (node->StateNo >= 0)) {
//    checksolved(node->StateNo, gEpsilon);     //ANUJ guessing it can be check.
      check(node->StateNo, gEpsilon);
  }
  /*if ((gMethod == lpvi) && flag) {
    flag = checksolved(node->StateNo, gEpsilon);
  }*/

  predecessorActions = node->BestPrevActions;
  while (predecessorActions) {
    predecessorAction = predecessorActions->Node;
    predecessor = predecessorAction->PrevState;
    //if (predecessor->StateNo == oldindex)
      //break;
    /* Stop if already visited this iteration or start state */
    if (predecessor->Expanded < Iteration) { 
      
      /* If already expanded, just mark as visited this iteration */
      if (predecessor->Expanded > 0)
        predecessor->Expanded = Iteration;
      pExpandSolution(predecessor);
    }
    predecessorActions = predecessorActions->Next;
    oldindex = predecessor->StateNo;
  }
}
  
void n_pExpandSolution(struct StateNode *node)
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */

{
  struct StateNode *predecessor;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  int oldindex = 0;
  double residual = 999.99;

  /* If not yet expanded, expand here.
     Then consider successor nodes for expansion. */
  if (node->Expanded == 0) {
  //if (node->Expanded <= Iteration) {
    ExpandNode(node);
  }
  
  /* added for labelling */
  if ((gMethod == lpvi) && (residual < gEpsilon) && (node->StateNo >= 0)) {
//    checksolved(node->StateNo, gEpsilon);     //ANUJ guessing it can be just check.
      check(node->StateNo, gEpsilon);
  }
  /*if ((gMethod == lpvi) && flag) {
    flag = checksolved(node->StateNo, gEpsilon);
  }*/

  predecessorActions = node->BestPrevActions;
  while (predecessorActions) {
    predecessorAction = predecessorActions->Node;
    predecessor = predecessorAction->PrevState;
    if (predecessor->StateNo == oldindex)
      break;
    /* Stop if already visited this iteration or start state */
    if ((predecessor->Expanded < Iteration) &&
        (predecessor != Start)) {
      
      /* If already expanded, just mark as visited this iteration */
      if (predecessor->Expanded > 0)
        predecessor->Expanded = Iteration;
      residual = Backup(predecessor);
      if (residual > gEpsilon)
        pExpandSolution(predecessor);
    }
    predecessorActions = predecessorActions->Next;
    oldindex = predecessor->StateNo;
  }
}
  
double bpExpandSolution(struct StateNode *node)
{
  struct StateNode *predecessor;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;
  struct StateList *list;
  int oldindex, state;
  double residual = 0.0;
  double res = 999.99;

  list = (struct StateList*)CreateList(gNumStates + 1);
  AppendState(list, node->StateNo);
  while (!IsEmptyList(list)) { 
    state = OutList(list);
    if (state >= 0)
      node = StateIndex[state];
    else
      node = Start;
    if (node == Start) {
      //printf("\nDelete start state");
      //fflush(0);
    }
    res = Backup(node);
    if (res > residual)
      residual = res;
    predecessorActions = node->BestPrevActions;
    while (predecessorActions) {
      predecessorAction = predecessorActions->Node;
      predecessor = predecessorAction->PrevState;
      state = predecessor->StateNo;
      if (predecessor->Update < Iteration) {
        predecessor->Update = Iteration;
        AppendState(list, state);
        if (state == -1) {
          //printf("\nInsert start state");
          //fflush(0);
        }
      }
      predecessorActions = predecessorActions->Next;
    }
  }
  return residual;
}
  
void ExpandSolution(struct StateNode *node)
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */

{
  struct StateDistribution *successor;

  /* If not yet expanded, expand here.
     Then consider successor nodes for expansion. */
  if (node->Expanded == 0) {
    ExpandNode(node);
    if(gBackupMethod == combine) 
      Backup(node);
  }

  /* Assume successor states are sorted in order of probability */
  for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next) {
    
    /* Stop if already visited this iteration or goal state */
    if ((successor->State->Expanded < Iteration) && 
	(successor->State->Terminal != 1)) {
      
      /* If already expanded, just mark as visited this iteration */
      if (successor->State->Expanded > 0)
	successor->State->Expanded = Iteration;
      ExpandSolution(successor->State);
    }
  }
  /* Possibly perform backup when backtrack to this node */
  if( gBackupMethod == combine )
    Backup(node);
}

void ExpandNode(struct StateNode *node)
{
  node->Expanded = Iteration;
  /* set heuristic value */
  if (gBackupMethod == separate)
    AppendStateList(ExpandedStateList, node);
  NumExpandedStatesIter++;
  NumExpandedStates++;
}

struct StateListNode *FindAncestors(struct StateListNode *ExpandedStateList)
{
  struct StateListNode *AddList, *NewAddList = NULL, *AncestorList, *node;
  struct ActionListNode *prev;

  AddList = CreateStateList();
  AncestorList = CreateStateList();

  /* Initial AddList consists of all expanded states */
  for (node = ExpandedStateList; node; node = node->Next) {
    node->Node->Update = Iteration;
    AppendStateList(AddList, node->Node);
  }

  /* Find ancestor states that need to be updated */
  while (AddList->Node) { /* check for empty list */
    NewAddList = CreateStateList();
    /* For each state added to Z ... */
    for (node = AddList; node; node = node->Next) {
      /* ... and for each parent of that state ... */
      for (prev = node->Node->PrevAction; prev; prev = prev->Next) {
	/* only add a parent along a marked action arc */
	/* also, parent must be expanded */
	if ((prev->Node == prev->Node->PrevState->BestAction) &&
	    (prev->Node->PrevState->Expanded > 0))
	  /* don't add parent that is already in ancestor list */
	  if (prev->Node->PrevState->Update < Iteration) {
	    AppendStateList(NewAddList, prev->Node->PrevState);
	    prev->Node->PrevState->Update = Iteration;
	  }
      }
      AppendStateList(AncestorList, node->Node);
      NumAncestorStatesIter++;
    }
    DeleteStateList(AddList);
    AddList = NewAddList;
    NewAddList = NULL;
  }
  DeleteStateList(AddList);
  DeleteStateList(NewAddList);
  return(AncestorList);
}

void DisplaySolution( void )
{
  NumSolutionStates = 0;
  Iteration++;
  DisplaySolutionRecursive( Start );
}


//static struct StateListNode *DisplaySolutionRecursive(struct StateNode *node)
static void DisplaySolutionRecursive(struct StateNode *node)
/* Set NumSolutionStates to zero before calling this recursive function */
{
  struct StateDistribution *successor;

  printf("\nindex %d State %s f %f",
	 node->StateNo,
	 node->Description,
	 node->f);
  NumSolutionStates++;
  if (node->Terminal == 0) {
    printf(" best action %d", node->BestAction->ActionNo);
    
    for (successor = node->BestAction->NextState;
	 successor;
	 successor = successor->Next)
      if (successor->State->Update < Iteration) {
	successor->State->Update = Iteration;
	DisplaySolutionRecursive(successor->State);
      }
  }
}

int ConvergenceTest( void )
{
  double error;

  ExitFlag = 0;
  do {
    Iteration++;
    NumSolutionStates = 0;
    Residual = 0.0;
    ConvergenceTestRecursive( Start );
    if ( gDiscount < 1.0 )
      error = (gDiscount * Residual)/(1.0 - gDiscount);
    else
      error = Start->meanFirstPassage * Residual;
    printf("  Error bound: %f", error);
    printf("\n%3d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
  } while (!ExitFlag && (error > gEpsilon));
  if (ExitFlag)
    return( 0 );
  else
    return( 1 );
}

static void ConvergenceTestRecursive( struct StateNode *node )
{
  struct StateDistribution *successor;
  double Diff;

  /* Count this node as part of best solution */
  NumSolutionStates++; 

  /* Terminate recursion at goal node */
  if ((node->Terminal == 1) || (node->Terminal == 5))
    return;

  /* Exit convergence test if best solution has unexpanded node */
  if (!node->Expanded) 
    ExitFlag = 1;

  /* Recursively consider unvisited successor nodes */
  for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next)
    if (successor->State->Update < Iteration) {
      successor->State->Update = Iteration;
      ConvergenceTestRecursive(successor->State);
    }

  /* Do backup to improve value of node */
  Diff = Backup(node);
  if ((gMethod == lpvi) && (Diff < gEpsilon) && (node->StateNo >= 0) && !solved[node->StateNo] && (Iteration < 30)) {
    check(node->StateNo, gEpsilon);
  }
  if ( Diff > Residual ) Residual = Diff;
}

int rConvergenceTest( double* fp )
{
  double error;
  double oldf, diff;
  //double oldf2;

  ExitFlag = 0;
  do {
    fp = &oldf;
    oldf = Start->f;
    Iteration++;
    NumSolutionStates = 0;
    Residual = 0.0;
    //ConvergenceTestRecursive( Start );
    rConvergenceTestRecursive( Goal );
    //printf("  Residual  : %f", Residual);
    if ( gDiscount < 1.0 )
      error = (gDiscount * Residual)/(1.0 - gDiscount);
    else
      error = Start->meanFirstPassage * Residual;
    printf("  Error bound: %f", error);
    printf("\n%d ( %f secs.)", 
           Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
  } while (!ExitFlag && (error > gEpsilon));
  diff = Start->f - *fp;
  if ((Residual > gEpsilon) || (-Residual > gEpsilon))
    return( 0 );
  else
    return( 1 );
}

static void rConvergenceTestRecursive( struct StateNode *node )
{
  struct ActionListNode *predecessorActions;
  struct ActionNode *predecessorAction;
  struct StateNode *predecessor;
  double Diff;

  /* Count this node as part of best solution */
  //printf("\nState: %d", node->StateNo);
  //NumSolutionStates++; 
  node->Update = Iteration;
  
  /* Recursively consider unvisited predecessor nodes */
  predecessorActions = node->BestPrevActions;
  //predecessorActions = node->PrevAction;
  while (predecessorActions) {
    predecessorAction = predecessorActions->Node;
    predecessor = predecessorAction->PrevState;
    if (predecessor->Update < Iteration) {
      rConvergenceTestRecursive(predecessor);
    }
    predecessorActions = predecessorActions->Next;
  }

  /* Do backup to improve value of node */
  //Diff = FBackup(node);
  Diff = Backup(node);
  if ((gMethod == lpvi) && (Diff < gEpsilon) && (node->StateNo >= 0)) {
    check(node->StateNo, gEpsilon);
  }
  //printf("\nBacking up state: %d", node->StateNo);
  if ( Diff > Residual ) Residual = Diff;
}
 
int check(int state, double epsilon) {
  int rv, s;
  double diff;
  struct intStack *open, *closed;
  struct StateNode *node;
  struct ActionNode *greedyAction;
  struct StateDistribution *nextState;
  
  if ((StateIndex[state]->Terminal == 1) || (StateIndex[state]->Terminal == 5)) {
    solved[state] = 1;
    return 1;
  }
  rv = 1;
  open = CreateStack();
  closed = CreateStack();
  
  if (solved[state] == 0)
    push(open, state);
  while (!empty(open)) {
    s = pop(open);
    push(closed, s);
    
    node = StateIndex[s];
    //printf("\nState: %d", s);
    //printf("\nState: %d", s);
    diff = Backup(node);
    //printf("\nhere2");
    //printf("\nhere2");
    if (diff > epsilon) {
      rv = 0;
      continue;
    }
      
    // expand state
    greedyAction = node->BestAction;
    if (greedyAction) {
      nextState = greedyAction->NextState;
      //printf("\nNext state: %d", nextState->State->StateNo);
      for(nextState = greedyAction->NextState;
          nextState;
          nextState = nextState->Next) {
        node = nextState->State;
        s = node->StateNo;
        if ((solved[s] == 0) && (instack(closed, s) == 0) && (instack(open, s) == 0)) {
          push(open, s);
        }
      }
    }
  }
  
  if (rv == 1) {
    // label relevant states
    while (empty(closed) == 0) {
      s = pop(closed);
      solved[s] = 1;
    }
  }
  else {
    // update states with residuals and ancestors
    while (empty(closed) == 0) {
      s = pop(closed);
      node = StateIndex[s];
      Backup(node);
    }
  }
  return rv;
}
