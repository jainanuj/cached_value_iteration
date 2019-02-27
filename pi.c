/* pi.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "solve.h"
#include "graph.h"
#include "math.h"

#include "backup.h"

static void PolicyEvaluation(struct StateListNode *list)
/* Compute value function for current policy */
{
  float **a, **b;
  int stateNum = 0, i, j, NumSolutionStates;
  struct StateListNode     *stateListNode;
  struct StateDistribution *nextState;

  /* Determine policy size */
  NumSolutionStates = 0;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next) {
    //if (stateListNode->Node->Terminal != 1) {
      NumSolutionStates++;
      /* Number solution states using update field */
      stateListNode->Node->Update = NumSolutionStates;
    //}
  }
      
  /* allocate matrices and initialize coefficient matrix to zeros 
     (wish there were way to avoid double loop for initialization) */
  a=matrix(1,NumSolutionStates,1,NumSolutionStates);
  b=matrix(1,NumSolutionStates,1,1);
  for (i=1;i<=NumSolutionStates;i++) {
    for (j=1;j<=NumSolutionStates;j++) {
      a[i][j] = 0.0;
    }
    b[i][1] = 0.0;
  }
  
  /* load equations into matrices */
  /* a is square matrix of coefficients */
  /* b is vector of constants */
  //stateNum = 1;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next) {
    stateNum = stateListNode->Node->Update;
    if (stateListNode->Node->Terminal != 1) {
      for (nextState = stateListNode->Node->BestAction->NextState;
	   nextState;
	   nextState = nextState->Next)
	a[stateNum][nextState->State->Update] = - gDiscount * nextState->Prob;
      a[stateNum][stateNum] += 1.0;
      b[stateNum][1] = stateListNode->Node->BestAction->Cost;
      //stateNum++;
    }
    else {
      a[stateNum][stateNum] = 1.0;
      //b[stateNum][1] = 1.0;
      //printf("\n%d", stateNum);   
    }
  }

  /*printf("\n");
  for (i = 1; i <= NumSolutionStates; i++) {
    for (j = 1; j <= NumSolutionStates; j++) {
      printf("%3f ", a[i][j]);
    }
    printf("%3f\n", b[i][1]);
  }*/
    
  printf("\nEnter gaussj with %d solution states", stateNum);
  /* gaussj returns unknown value function in vector b */
  gaussj(a, NumSolutionStates, b, 1);
  printf("\nExit gaussj"); fflush(0);

  /* Copy updated value function to state array */
  //stateNum = 1;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next)
    if (stateListNode->Node->Terminal != 1) {
      /*stateListNode->Node->f = b[stateNum][1];
      stateNum++; */
      stateListNode->Node->f = b[stateListNode->Node->Update][1];
    }
    else
      stateListNode->Node->f = 1.0;
  for (i = 1; i <= NumSolutionStates; i++) {
    printf("%f ", b[i][1]);
  }
  printf("\n");
  printf("Start f: %f", Start->f);

  free_matrix(a,1,NumSolutionStates,1,NumSolutionStates);
  free_matrix(b,1,NumSolutionStates,1,1);
}

void PolicyIteration(struct StateListNode *list)
{
  int    Iter, stateNum, improveFlag, MaxIter;
  double diff, maxdiff, error;
  struct StateListNode   *stateListNode;
  
  MaxIter = 10;
  Iter = 0;
/*  do {
    PolicyEvaluation( list );
    printf(".");

    improveFlag = 0;
    for (stateListNode = list; 
	 stateListNode; 
	 stateListNode = stateListNode->Next)
      if (stateListNode->Node->Terminal != 1) { 
	diff = Backup(stateListNode->Node);
	if (diff > gEpsilon)
	  improveFlag = 1;
      }
    printf("\n%3d ( %f secs.)  f: %f  Error bound: %f", 
	   Iter++, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, error);

  } while (improveFlag == 0);*/
  for (Iter = 0; Iter < MaxIter; Iter++) {
    PolicyEvaluation( list ); 
  
    maxdiff = 0.0;
    for (stateListNode = list; 
	 stateListNode; 
	 stateListNode = stateListNode->Next)
      if (stateListNode->Node->Terminal != 1) { 
	diff = Backup(stateListNode->Node);
	if (diff > maxdiff)
	  maxdiff = diff;
      }
    //printf("\nMaxdiff: %f", maxdiff);
      if ( gDiscount < 1.0 ) 
	error = (maxdiff * gDiscount)/(1.0 - gDiscount);
      else
	error = (maxdiff * Start->meanFirstPassage);
      printf("\n%d ( %f secs.)  f: %f  Error bound: %f", 
	     Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, error);	     
    }

}
