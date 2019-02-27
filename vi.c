/* vi.c */
#include <stdio.h>
#include <time.h>
#include "solve.h"
#include "track.h"
#include "graph.h"
#include "vi.h"
#include "backup.h"
#include "stdio.h"

struct StateListNode *CreateAllStateList(void)
/* Create list of all non-goal states */
{
  int state;
  struct StateListNode *list;

  list = CreateStateList();
  AppendStateList(list, Start);
  for (state = 0; state < gNumStates; state++)
    AppendStateList(list, StateIndex[state]);
  return(list);
}

struct StateListNode *CreateAllStateListWithoutGoal(void)
/* Create list of all non-goal states */
{
  int state;
  struct StateListNode *list;

  list = CreateStateList();
  AppendStateList(list, Start);
  for (state = 0; state < gNumStates; state++)
    if (StateIndex[state]->Terminal != 1)
      AppendStateList(list, StateIndex[state]);
  return(list);
}

void ValueIteration (struct StateListNode *list, int MaxIter)
{
  int                       Iter, i;
  double                    diff, maxdiff, /* Bellman residual */
                            error;
  struct StateListNode     *stateListNode;
  double time, threshold = 300.0;
  
  for (Iter = 0; Iter < MaxIter; Iter++) {
    maxdiff = 0.0;
    for (stateListNode = list; 
	 stateListNode; 
	 stateListNode = stateListNode->Next)
      if ((stateListNode->Node->Terminal != 1) && (stateListNode->Node->Terminal != 5)) {
        if ( !gElimination )
          diff = Backup(stateListNode->Node);
        else
          diff = BackupTwo(stateListNode->Node);
	if (diff > maxdiff) {
	  maxdiff = diff;
        }
      }
    time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
    if ( time > threshold ) {
      printf("\nf: %f fprime: %f", Start->f, Start->fprime);
      printf("\n%d ( %f secs.)  f: %f Unconverged\n", Iter, time, Start->f);
      return;
    }
    if ( (gMethod == vi) || (gMethod == vie) ) {
      error = maxdiff;
/*
      if ( gDiscount < 1.0 ) 
	error = (maxdiff * gDiscount)/(1.0 - gDiscount);
      else
	error = (maxdiff * Start->meanFirstPassage);
*/
//        printf("\n%3d ( %f secs.)  f: %lf  Error bound: %lf", Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, error);
//        printf("\n%3d ( %f secs.)  f: %lf  fprime: %lf  Error bound: %lf", Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, Start->fprime, error);
	     
      if ( error < gEpsilon ) {
//        printf("\nf: %f", Start->f);
        printf("\nf: %f fprime: %f", Start->f, Start->fprime);
        printf("\n%d ( %f secs.)  f: %f Converged\n", Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f);
	return;
      }
    }
  }
}

