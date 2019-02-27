#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "graph.h"
#include "components.h"
#include "solve.h"
#include "track.h"

#include "vi.h"

int *id;
int *postI;
int *postR;
int scnt, cnt;

void Bdfs(int w)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;
   
  id[w] = scnt;
  state = StateIndex[w];
  
  if ( (state->Terminal == 1) || (state->Terminal == 5) )
    return;
  actionNode = state->BestAction;
  if (!actionNode)
    return;
  for (nextState = actionNode->NextState; nextState; nextState = nextState->Next) {
    nodeNumber = nextState->State->StateNo;
    if (id[nodeNumber] == -1)
      Bdfs(nodeNumber);
  }
  postI[cnt++] = w;
}
 
void BdfsR(int w)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateNode *prevState;
   
  id[w] = scnt;
  state = StateIndex[w];

  for (actionListNode = state->BestPrevActions;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    prevState = actionNode->PrevState;
      nodeNumber = prevState->StateNo;
      if (id[nodeNumber] == -1)
        BdfsR(nodeNumber);
  }
  postI[cnt++] = w;
}

void regenerate_best_prev_actions()
{
  int s;
  struct StateNode *state, *next_state;
  struct ActionListNode *actionListNode, *actionList;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;

  for (s = 0; s < gNumStates; ++s) {
    StateIndex[s]->BestPrevActions = NULL;
  }

  for (s = 0; s < gNumStates; ++s) {
    state = StateIndex[s];
    actionNode = state->BestAction;
    if (!actionNode)
      continue;
    for (nextState = actionNode->NextState; nextState; nextState = nextState->Next) {
      next_state = nextState->State;
      // add this action to the bestPrevAction of the successor state
      actionList = CreateActionList();
      actionList->Node = actionNode;
      actionList->Next = next_state->BestPrevActions;
      next_state->BestPrevActions = actionList;
    }
  }
}

void generate_components()
{
  cnt = 0;
  scnt = 0;
  int state;

  id = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postI = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postR = (int*)malloc((unsigned)gNumStates * sizeof(int));

  regenerate_best_prev_actions();
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = 0; state < gNumStates; state++)
    if (id[state] == -1)
      BdfsR(state);
  for (state = 0; state < gNumStates; state++)
    postR[state] = postI[state];
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id[state] = -1;
  for (state = gNumStates - 1; state >= 0; state--)
    if (id[postR[state]] == -1) {
      Bdfs(postR[state]);
      scnt++;
    }
  
  printf("\nTotal number of components in optimal policy: %d", scnt);
  free(id);
  free(postI);
  free(postR);
}

double optimality()
{
  int i;
  double optimal_sum = 0.0;

  ValueIteration (CreateAllStateListWithoutGoal(), 1000);
  for (i = 0; i < gNumStates; i++) {
    optimal_sum += StateIndex[i]->f;
  }

  return gInitValueSum / optimal_sum;
}

void dump_optimal_values()
{
  char out_file_name[30];
  FILE *outf;
  int begin, end, i;

printf("\nDump_optimal_values()");
  begin = 0;
//  begin = strchr(gInputFileName, '/')- gInputFileName + 1;
  end = strchr(gInputFileName, '.') - gInputFileName;
  for (i = 0; i < end - begin; ++i)
    out_file_name[i] = gInputFileName[i+begin];
  sprintf(&out_file_name[end-begin], "_val.dat");

  outf = fopen(out_file_name, "w");
  ValueIteration (CreateAllStateListWithoutGoal(), 1000);
  for (i = 0; i < gNumStates; ++i) {
    fprintf(outf, "%f\n", StateIndex[i]->f);
    StateIndex[i]->f = 0.0;
    StateIndex[i]->h = 0.0;
    StateIndex[i]->g = 0.0;
    Start->f = 0.0;
  }
  fclose(outf);
}

double CreateDiscountedOptimalHeuristics()
{
  char in_file_name[30], str[164];
  FILE *inf;
  int begin, end, i;
  double diff, largest_diff = 0.0;
  struct StateNode *state;

  begin = 0;
  end = strchr(gInputFileName, '.') - gInputFileName;
  for (i = 0; i < end - begin; ++i)
    in_file_name[i] = gInputFileName[i+begin];
  sprintf(&in_file_name[end-begin], "_val.dat");
  printf("\n%s", in_file_name);
  fflush(0);

  inf = fopen(in_file_name, "r");
  double value;
  for (i = 0; i < gNumStates; ++i) {
    bzero(str, 64);
    fgets(str, 64, inf); 
    state = StateIndex[i];
    sscanf(str, "%lf", &value); 
    state->f = value * gHDiscount;
    state->h = value * gHDiscount;
    diff = state->fprime - state->f;
    if ( diff > largest_diff )
      largest_diff = diff;
  }
  fclose(inf);

  return largest_diff;
}
