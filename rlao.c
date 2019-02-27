/* lao.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "lao.h"
#include "backup.h"
#include "solve.h"

#include "vi.h"

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

/* forward function declarations */
static void   ExpandSolution(struct StateNode *node);
static void   rExpandSolution(struct StateNode *node);
static void   ExpandNode(struct StateNode *node);
static struct StateListNode *FindAncestors(struct StateListNode *ExpandedStateList);
static void   DisplaySolution( void );
static int    ConvergenceTest( void );
static void   ConvergenceTestRecursive( struct StateNode *node);
static int    rConvergenceTest( void );
static void   rConvergenceTestRecursive( struct StateNode *node);
//static struct StateListNode *DisplaySolutionRecursive(struct StateNode *node);
static void DisplaySolutionRecursive(struct StateNode *node);

void rLAOstar(void)
{
  struct StateListNode *AncestorList;
  struct StateNode *node;

  NumExpandedStates = 0;
  /* First expand nodes along most likely path to goal */
  NumExpandedStatesIter = 0;
  for (node = Start; 
       (node->Terminal != 1) && (node->Expanded < 1); 
       node = node->BestAction->NextState->State)
    {
      node->Expanded = 1;
      //printf("\nhere %d", node->StateNo);
      printf("\nExpand node %d", node->StateNo);
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
    
    //printf("\nhere1");
	     

    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    ExpandedStateList = CreateStateList();
    rExpandSolution(Goal);
    //rExpandSolution( Start );

    /* Call convergence test if solution graph has no unexpanded tip states */
    //if ((NumExpandedStatesIter == 0) && rConvergenceTest()) {
    if ((NumExpandedStatesIter == 0) && rConvergenceTest()) {
      printf("  Converged!");
      break;
    }

    /* Dynamic programming step */
    /* (Skip if backups were performed in expansion step) */
    if( gBackupMethod == separate)
      {
	/* Update state costs and best partial solution */
	NumAncestorStatesIter = 0;
	/* the following condition is necessary in case no nodes are
	   expanded but the convergence test fails */
	if (NumExpandedStatesIter > 0) { 
	  AncestorList = FindAncestors(ExpandedStateList);
	  /* note that expanded nodes are included as ancestors */
	  printf("  ancestors: %d", NumAncestorStatesIter);
	  printf("  explicit graph: %d", NumExpandedStates);
	  ValueIteration(AncestorList, 3);
	  DeleteStateList(AncestorList);
	}
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

void rExpandSolution(struct StateNode *node)
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */

{
  struct StateNode *predecessor;
  struct ActionNode *predecessorAction;
  struct ActionListNode *predecessorActions;

  /* If not yet expanded, expand here.
     Then consider successor nodes for expansion. */
  if (node->Expanded == 0) {
    ExpandNode(node);
    if(gBackupMethod == combine) 
      Backup(node);
  }

  //predecessorAction = node->BestPrevAction;
  //if (predecessorAction) {
  
  predecessorActions = node->PrevAction;  
  while(predecessorActions) {
    predecessorAction = predecessorActions->Node;
    predecessor = predecessorAction->PrevState;
    //predecessor = node->BestPrevAction->PrevState;
    //printf("\nstate: %d", predecessor->StateNo);
    /* Stop if already visited this iteration or start state */
    if ((predecessor->Expanded < Iteration) && 
	(predecessor != Start)) {
      
      /* If already expanded, just mark as visited this iteration */
      if (predecessor->Expanded > 0)
	predecessor->Expanded = Iteration;
      rExpandSolution(predecessor);
    }
    predecessorActions = predecessorActions->Next;
  }
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
      /*
      if ((successor->State->Update < Iteration) &&
	  ((successor->State->Expanded == Iteration) ||
	   (successor->State->Terminal > 0))) {
      */
	successor->State->Update = Iteration;
	DisplaySolutionRecursive(successor->State);
      }
  }
}

int ConvergenceTest( void )
/* From start state, perform depth-first search of all states 
   visited by best solution. Mark each state when it is visited
   so that it is visited only once each pass. For each state:
   -- perform backup
   -- update Bellman residual
   If visit unexpanded state, exit with FALSE. 
   If error bound <= epsilon, exit with TRUE.
   Otherwise repeat.
*/
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
  if (node->Terminal == 1)
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
  if ( Diff > Residual ) Residual = Diff;
}

int rConvergenceTest( void )
/* From start state, perform depth-first search of all states 
   visited by best solution. Mark each state when it is visited
   so that it is visited only once each pass. For each state:
   -- perform backup
   -- update Bellman residual
   If visit unexpanded state, exit with FALSE. 
   If error bound <= epsilon, exit with TRUE.
   Otherwise repeat.
*/
{
  double error;
  double oldf;
  //double oldf2;

  ExitFlag = 0;
  do {
    //oldf2 = oldf;
    oldf = Start->f;
    Iteration++;
    NumSolutionStates = 0;
    Residual = 0.0;
    ConvergenceTestRecursive( Start );
    if ( gDiscount < 1.0 )
      error = (gDiscount * Residual)/(1.0 - gDiscount);
    else
      error = Start->meanFirstPassage * Residual;
    printf("  Error bound: %f", error);
    printf("\n%d ( %f secs.)", 
	   Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);
    printf("  f: %f", Start->f);
  } while (!ExitFlag && (error > gEpsilon));
  //if (ExitFlag)
  if ((Start->f - oldf) > gEpsilon)
    return( 0 );
  else
    return( 1 );
}

static void rConvergenceTestRecursive( struct StateNode *node )
{
  struct StateDistribution *successor;
  double Diff;

  /* Count this node as part of best solution */
  NumSolutionStates++; 

  /* Terminate recursion at goal node */
  if (node->Terminal == 1)
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
  if ( Diff > Residual ) Residual = Diff;
}
