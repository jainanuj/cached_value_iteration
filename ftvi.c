/* ftvi.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "track.h"
#include "graph.h"
#include "atvi.h"
#include "backup.h"
#include "solve.h"
#include "cache_aware_vi.h"

#include "top.h"
//#include "top.h"
#define MAX_ITER   10000
#define SIZE_THRESHOLD 1000
#define RATIO_THRESHOLD 0.6        //Initially was 0.97 -- ANUJ

#define RECURSE_MOD 600000
#define COMP_SIZE_THRESHOLD  500  //10000  //2

static int initsearch(int, int);
static void accplaning(int);
static void expandsolution(struct StateNode*);
static void buildtop();
static void solve_component(int);
static void f_reachability(int s);
void f_viter(struct StateListNode *list, int MaxIter, int round, int component_size);
static void f_dfs(int w);
static void f_dfsR(int w);
void c_dfs(int w, int round);
void c_dfsR(int w, int round);

/* global variables */
static int trial;
static int stateExpanded;
static int stateExpandedTrial;
static int scnt;
static int cnt;
static struct StateListNode **stateListNode;

static int local_cnt;
static int local_scnt;

double init_search_error;

static int *id_ftv;         //Changed from non static to static. Changed name to -ftv. Anuj
static int *postI_ftv;      //Changed from non static to static. Changed name to -ftv. Anuj
static int *postR_ftv;      //Changed from non static to static. Changed name to -ftv. Anuj
static int *reachable_ftv;
static int *reachableC_ftv;
static int *sizes_ftv;

int *state_hash;
int *inv_state_hash;
int *local_id;
int *local_postI;
int *local_postR;
int *local_reachableC;
int *local_sizes;
int biggest_size = 0;
struct StateListNode **local_stateListNode;
static int recurseCount = 0;

void FTVI(int version)
{
  trial = 0;
  int success;
  success = initsearch(gLTrials, gUTrials);
  if (success != 1) {
    accplaning(version);
//    TVI();
/*
    struct StateListNode *list;
    list = CreateStateList();
    AppendStateList(list, Start);
    int state;
    for ( state = 0; state < gNumStates; state++)
      if (StateIndex[state]->Terminal != 1)
        AppendStateList(list, StateIndex[state]);
    f_viter ( list , 200000 );
    printf("\n%f", Start->f);
*/
  }
}

void total_actions()
{
  int s;
  long actions = 0;
  struct StateNode *state;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;

  for ( s = 0; s < gNumStates; ++s ) {
    state = StateIndex[s];
      state->num_actions = 0;
    for (actionListNode = state->Action;
         actionListNode;
         actionListNode = actionListNode->Next)
    {
      actionNode = actionListNode->Node;
      if ( !actionNode->Dominated )
      {
        actions++;
          state->num_actions++;
      }
    }
  }
  printf("\nTotal actions %ld per state %f", actions, (double)actions/gNumStates);
}


int initsearch(int lTrials, int uTrials)
{
  struct StateNode *state, *node;
  struct ActionNode *action;
  double diff, maxdiff, val, wavg, time;
  int i, steps;
  char c;
  struct timeval t, t1;
  
  stateExpanded = 0;
  total_actions();
    wavg = Start->f;
  for ( i = 0; i < uTrials/lTrials; ++i )
  {
    Backup(Start);
    wavg = Start->f;
      printf("Starting next search batch\n");
    for ( ; trial < (i+1)*lTrials; ++trial )
    {
      if ( trial > i*lTrials )
        init_search_error = 0.0;
    
      recurseCount = 0;
      expandsolution(Start);
      if ( trial % 100 == 0 )
        printf("\nInitial %d trials  f: %f fprime: %f error: %f", trial, Start->f, Start->fprime, init_search_error);
      if ( init_search_error < gEpsilon )
      {
        printf("\nState expanded: %d", stateExpanded);
        gettimeofday(&t, NULL);
        time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
        printf("\nInitial %d trials( %f secs.)  f: %f", trial, time, Start->f);
        printf("\nInitsearch ( %f secs.)  Converged!\n", time);
        return 1;
      }

    }
      double ratio = 0;
      if (wavg > Start->f)
          ratio = Start->f/wavg;
      else
          ratio = wavg / Start->f;
    printf("\nOld v: %f new v: %f change ratio of initial state value is %f", wavg, Start->f, ratio);
    if ( ratio > RATIO_THRESHOLD )
    {
      ++i;
      break;
    }
  }

  printf("\nState expanded: %d", stateExpanded);
  gettimeofday(&t, NULL);
  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
  printf("\nInitial %d trials( %f secs.)  f: %f", i*lTrials, time, Start->f);
    printf("\nTotal number of backups %ld", numBackups);
  buildtop();       //Create components.
  gettimeofday(&t1, NULL);
  time = (t1.tv_sec - t.tv_sec) + (float)(t1.tv_usec - t.tv_usec) / 1000000;
  printf("\nelapsed time for building topological structure: %f", time);

  total_actions();
  return 0;
}

static void accplaning(int version)
{
  int round, i, j, steps;
  struct StateListNode *list;
  struct StateNode *state, *head;
  struct ActionNode *action;
  double diff, maxdiff, val, wavg, time, residual, avgdiff, backupnum;
  struct timeval t;
    int solved_component = 0, unreachable_component = 0;

  scnt = id_ftv[gInitialState] + 1;
    if (version == 1)
        printf("\nSolving components, no subdividing. Solving %d components\n", scnt);
#ifdef __TEST__
    print_back_list(stateListNode[0], sizes_ftv[0], "verify_list_0");
#endif
    
  for (round = 1; round < scnt; round++)
  {
    if (reachableC_ftv[round])
    {
        solved_component++;
      int cur_size = sizes_ftv[round];
      if (version == 1)
      {
        if (cur_size > biggest_size)
          biggest_size = cur_size;
        list = stateListNode[round];
        f_viter(list, MAX_ITER, round, cur_size);
      }
      else
        solve_component(round);
    }
    else
        unreachable_component++;
  }
  Backup(Start);
  printf("\nBiggest component size: %d", biggest_size);
  gettimeofday(&t, NULL);
  time = (t.tv_sec - gInitialT.tv_sec) + (float)(t.tv_usec - gInitialT.tv_usec) / 1000000;
  printf("\n%d ( %f secs.)  f: %f Converged!", round, time, Start->f);
    printf("\nNumber of solved components=%d and unreachable components=%d", solved_component, unreachable_component);
  printf("\n");
    
}

void checkSuccs(struct StateNode* node)
{
    for (struct ActionListNode* aList = node->Action; aList; aList = aList->Next)
    {
        struct ActionNode* aNode = aList->Node;
        int numSuccs = 0;
        for (struct StateDistribution *succ = aNode->NextState; succ; succ = succ->Next)
        {
            numSuccs++;
        }
        if (numSuccs != 3)
        {
            printf("\nFor state = %d, action: %d; num of successors are: %d", node->StateNo,
               aNode->ActionNo, numSuccs);
        }
    }
}

static void expandsolution(struct StateNode* node)
{
  struct StateDistribution *successor;
  double error;

  if ( (node->Terminal == 5) || (node->Converged) )
    return;

    recurseCount++;
    if (recurseCount % RECURSE_MOD == 0)
        printf("\nRecursion count in expandsolution is: %d", recurseCount);
 
  if ( !node->Expanded ) 
    init_search_error = 1;
    
    //checkSuccs(node);       //Check if all actions of this state node have 3 successors.

  /* Assume successor states are sorted in order of probability */
  for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next)
  {
    
    /* Stop if already visited this iteration or goal state */
    if ( (successor->State->Expanded < trial) && (successor->State->Terminal != 1) )
    {
      /* If already expanded, just mark as visited this iteration */
        if ( !successor->State->Expanded )          //Moved this up one statement - ANUJ
            init_search_error = 1;
      successor->State->Expanded = trial;
      expandsolution(successor->State);
    }
      //Else Just move on to the next state if already visited
  }

  error = BackupTwo(node);
  if ( error > init_search_error )
    init_search_error = error;

}

static void buildtop(void)
{
  int state, round;
  struct StateNode     *node;
  struct StateListNode *list, *newlist;
  struct ActionNode    *actionNode;
  
  id_ftv = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postI_ftv = (int*)malloc((unsigned)gNumStates * sizeof(int));
  postR_ftv = (int*)malloc((unsigned)gNumStates * sizeof(int));
  reachable_ftv = (int*)malloc((unsigned)gNumStates * sizeof(int));

  for (state = 0; state < gNumStates; state++)
  {
    reachable_ftv[state] = 0;
  }
    printf("\nGoing to build components");
//  f_reachability(0);
    recurseCount = 0;
    f_reachability(gInitialState);
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id_ftv[state] = -1;
  for (state = 0; state < gNumStates; state++)
    if (id_ftv[state] == -1)
    {
        recurseCount = 0;
      f_dfsR(state);
    }
  for (state = 0; state < gNumStates; state++)
    postR_ftv[state] = postI_ftv[state];
  
  cnt = 0;
  scnt = 0;
  for (state = 0; state < gNumStates; state++)
    id_ftv[state] = -1;
  for (state = gNumStates - 1; state >= 0; state--)
  {
      if (postR_ftv[state] >= 0 && postR_ftv[state] < gNumStates)
      {
        if (id_ftv[postR_ftv[state]] == -1)
        {
            recurseCount = 0;
            f_dfs(postR_ftv[state]);
            scnt++;
        }
      }
      else
      {
          printf("Bad Error!!!. postR_ftv[state] is: %d for state: %d\n",postR_ftv[state], state);
      }
  }
  free(postI_ftv);
  free(postR_ftv);
      
  printf("\nComponents: %d", scnt);
  sizes_ftv = (int*)malloc(scnt * sizeof(int));
  // debugging  
  stateListNode = (struct StateListNode**)malloc(scnt * sizeof(struct StateListNode*));
  reachableC_ftv = (int*)malloc(scnt * sizeof(int));
  
  for (round = 0; round < scnt; round++)
  {
    stateListNode[round] = (struct StateListNode*)malloc(sizeof(struct StateListNode));
    stateListNode[round]->Node = NULL;
    stateListNode[round]->Next = NULL;
    reachableC_ftv[round] = 0;
    sizes_ftv[round] = 0;
  }
    
  for (state = 0; state < gNumStates; ++state)
  {
    // ignore unreachable and deadend states
    if ( (gLTrials < gUTrials) && !reachable_ftv[state] )
      continue;
    int component = id_ftv[state];
    reachableC_ftv[component] = 1;
    ++sizes_ftv[component];
    list = stateListNode[component];
    node = StateIndex[state];
    if (!list->Node)
    {
      list->Node = node;
    }
    else
    {
      newlist = (struct StateListNode*)malloc((unsigned)sizeof(struct StateListNode));
      newlist->Node = list->Node;
      newlist->Next = list->Next;
      list->Node = node;
      list->Next = newlist;
    }
      node->component_id = component;
  }
  //free(id);
  //free(reachable);
}

void validate_actionNode(struct ActionNode* actionNode)
{
    struct StateNode *prev_state = NULL;
    struct StateDistribution *stateDist = NULL;
    struct ActionListNode *actionListNode = NULL;
    struct ActionNode* prev_actionNode = NULL;

    if (actionNode == NULL)
    {
        printf("Action is NULL");
        exit(1);
    }
    if (actionNode->StateNo >= gNumStates)
    {
        printf("Bad Action as state is > maxStates");
        exit(1);
    }
    
    prev_state = actionNode->PrevState;
    for (actionListNode = prev_state->PrevAction; actionListNode; actionListNode = actionListNode->Next)
    {
        prev_actionNode = actionListNode->Node;     //Making sure list is well formed.
        for (stateDist = prev_actionNode->NextState; stateDist; stateDist = stateDist->Next)
        {
            if (stateDist->State->StateNo == actionNode->StateNo)
                return;
        }
    }
    printf("Some bad error with graph formation at state: %d", actionNode->StateNo);
    exit(1);
}

void f_dfs(int w)
{
  int nodeNumber = 0;
  struct StateNode *state = NULL;
  struct StateListNode *list = NULL;
  struct ActionListNode *actionListNode = NULL;
  struct ActionNode *actionNode = NULL;
  struct StateDistribution *nextState = NULL;
   
    if (w < gNumStates && w >= 0)
    {
        id_ftv[w] = scnt;
        state = StateIndex[w];
    }
    else
        return;
    
    recurseCount++;
    if (recurseCount % RECURSE_MOD == 0)
        printf("\nRecursion count in f_dfs is: %d", recurseCount);

  if ( (state->Terminal == 1) || (state->Terminal == 5) )
    return;
  
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next)
  {
    actionNode = actionListNode->Node;
      //validate_actionNode(actionNode);
      if (actionNode->Dominated == 1)
          continue;
    for (nextState = actionNode->NextState; nextState; nextState = nextState->Next)
    {
        //nextState = actionNode->NextState;
        if (nextState != NULL)
        {
            nodeNumber = nextState->State->StateNo;
            if (nodeNumber >= gNumStates || nodeNumber < 0)
            {
                printf("\nBad Error!!!!. Node number is somehow greater than total number of states.\n");
                printf("Happening with State: %d\n", actionNode->StateNo);
                //exit(1);
            }
            else if (id_ftv[nodeNumber] == -1)
                f_dfs(nodeNumber);
        }
    }
  }
  postI_ftv[cnt++] = w;
}
 
void f_dfsR(int w)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateNode *prevState;
   
  if (w >= 0 && w < gNumStates)
  {
    id_ftv[w] = scnt;
      state = StateIndex[w];
  }
    else
        return;

    recurseCount++;
    if (recurseCount % RECURSE_MOD == 0)
        printf("\nRecursion count in f_dfsR is: %d", recurseCount);
  
  for (actionListNode = state->PrevAction;
       actionListNode;
       actionListNode = actionListNode->Next)
  {
    actionNode = actionListNode->Node;
    if (actionNode->Dominated == 1)
      continue;
    prevState = actionNode->PrevState;
      nodeNumber = prevState->StateNo;
      if (nodeNumber < 0 || nodeNumber >= gNumStates)
      {
          printf("\nBad Error in f_dfsR!!!!. Node number is: %d out of bounds.\n", actionNode->StateNo);
//          printf("Happening with State: %d\n", actionNode->StateNo);
          //exit(1);
      }
      else
      {
          if (id_ftv[nodeNumber] == -1)
              f_dfsR(nodeNumber);
      }
  }
  postI_ftv[cnt++] = w;
}

//Marks the state reachable in array reachable_ftv if the state can be reached from initialstate.
void f_reachability(int s)
{
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;

    if (s < 0 || s >= gNumStates)
    {
        printf("This is the ERROR in f_reachability!!!!. s is out of bounds = %d\n", s);
        return; //exit(1);
    }
    recurseCount++;
    if (recurseCount % RECURSE_MOD == 0)
        printf("\nRecursion count in f_reachability is: %d", recurseCount);

    state = StateIndex[s];
    
    //checkSuccs(state);
    
  reachable_ftv[s] = 1;
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    if (actionNode->Dominated == 1)
      continue;
    for (nextState = actionNode->NextState;
	 nextState;
	 nextState = nextState->Next)
    {
      //nextState = actionNode->NextState;
      int nodeNumber = nextState->State->StateNo;
        if(nodeNumber >= 0 && nodeNumber < gNumStates)
        {
            if (reachable_ftv[nodeNumber] == 0)
                f_reachability(nodeNumber);
        }
        else
            printf("This is the ERROR in f_reachability!!!!. Nodenumber out of bounds = %d\n", nodeNumber);
    }
  }
}

void search_inside_component(struct StateNode* node, int round, int trial)
{
  struct StateDistribution *successor;

  if ( (node->Terminal == 5) || (node->Converged) )
    return;
  
  if (node->Expanded == 0) 
    BackupTwo(node);
  ++stateExpanded;
  
  for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next) {
    
    // terminate search if out of the component
    if (id_ftv[successor->State->StateNo] != round)
      continue;
    /* Stop if already visited this iteration or goal state */
    if ((successor->State->Expanded < trial) && (successor->State->Terminal != 1)) {
      /* If already expanded, just mark as visited this iteration */
      successor->State->Expanded = trial;
      search_inside_component(successor->State, round, trial);
    }
  }
  BackupTwo(node);
}

void c_dfs(int w, int round)
{
  int nodeNumber;
  struct StateNode *state, *newstate;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateDistribution *nextState;
   
  local_id[w] = local_scnt;
  state = StateIndex[state_hash[w]];
  
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    if (actionNode->Dominated)
      continue;
    for (nextState = actionNode->NextState;
	 nextState;
	 nextState = nextState->Next) {
      newstate = nextState->State;
      // if out of the component, quit
      if (id_ftv[newstate->StateNo] != round)
        continue;
      nodeNumber = inv_state_hash[newstate->StateNo];
      if (local_id[nodeNumber] == -1)
        c_dfs(nodeNumber, round);
    }
  }
  local_postI[local_cnt++] = w;
}
 
void c_dfsR(int w, int round)
{
  int nodeNumber;
  struct StateNode *state;
  struct StateListNode *list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateNode *prevState;
   
  local_id[w] = local_scnt;
  state = StateIndex[state_hash[w]];
  
  for (actionListNode = state->PrevAction;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    if (actionNode->Dominated)
      continue;
    prevState = actionNode->PrevState;
    // if out of the component, quit
    if (id_ftv[prevState->StateNo] != round)
      continue;
    nodeNumber = inv_state_hash[prevState->StateNo];
    if (local_id[nodeNumber] == -1)
      c_dfsR(nodeNumber, round);
  }
  local_postI[local_cnt++] = w;
}

void solve_component(int round)
{
  int local_round;
  int size = sizes_ftv[round];
  struct StateNode *node, *stateNode;
  struct StateListNode *list, *local_list, *new_list;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode;
  struct StateNode *prevState;

  list = stateListNode[round];
  if (size < SIZE_THRESHOLD) {
    if (size > biggest_size)
      biggest_size = size;
    f_viter(list, MAX_ITER, round, size);
    return;
  }

  // initialization
  state_hash = (int*)malloc(size * sizeof(int));
  inv_state_hash = (int*)malloc(gNumStates * sizeof(int));
  local_id = (int*)malloc(size * sizeof(int));
  local_postI = (int*)malloc(size * sizeof(int));
  local_postR = (int*)malloc(size * sizeof(int));
  int state;

  // record all states
  int state_num, trial;
  struct StateListNode *stateListNode;
  for (state_num = 0, stateListNode = list; stateListNode; stateListNode = stateListNode->Next, ++state_num) {
    int state_id = stateListNode->Node->StateNo;
    state_hash[state_num] = state_id;
    inv_state_hash[state_id] = state_num;
  }

  // search
  trial = 0;
  while (trial < 100) {
    for (stateListNode = list; stateListNode; stateListNode = stateListNode->Next, ++trial) {
      stateNode = stateListNode->Node;
      // if it is a state connected from a state with lower topological order, then search
      for (actionListNode = stateNode->PrevAction;
           actionListNode;
           actionListNode = actionListNode->Next) {
        actionNode = actionListNode->Node;
        if (actionNode->Dominated)
          continue;
        prevState = actionNode->PrevState;
        if (id_ftv[prevState->StateNo] > id_ftv[stateNode->StateNo]) {
          search_inside_component(stateNode, round, trial);
          break;
        }
      }
    }
  }

  // build top structure
  local_cnt = 0;
  local_scnt = 0;
  for (state = 0; state < size; state++)
    local_id[state] = -1;
  for (state = 0; state < size; state++)
    if (local_id[state] == -1)
      c_dfsR(state, round);
  for (state = 0; state < size; state++) {
    local_postR[state] = local_postI[state];
  }
  
  local_cnt = 0;
  local_scnt = 0;
  for (state = 0; state < size; state++)
    local_id[state] = -1;
  for (state = size - 1; state >= 0; state--)
    if (local_id[local_postR[state]] == -1) {
      c_dfs(local_postR[state], round);
      local_scnt++;
    }

  if (local_scnt > 1) {
    // generate local node lists
    local_stateListNode = (struct StateListNode**)malloc(local_scnt * sizeof(struct StateListNode*));
    local_reachableC = (int*)malloc((unsigned)local_scnt * sizeof(int));
    local_sizes = (int*)malloc((unsigned)local_scnt * sizeof(int));

    for (local_round = 0; local_round < local_scnt; local_round++) {
      local_stateListNode[local_round] = (struct StateListNode*)malloc(sizeof(struct StateListNode));
      local_stateListNode[local_round]->Node = NULL;
      local_stateListNode[local_round]->Next = NULL;
      local_reachableC[local_round] = 0;
      local_sizes[local_round] = 0;
    }
    
    for (state_num = 0, stateListNode = list; stateListNode; stateListNode = stateListNode->Next, ++state_num) {
      local_list = local_stateListNode[local_id[state_num]];
      node = stateListNode->Node;
      // ignore unreachable and deadend states
      if (reachable_ftv[node->StateNo])
        local_reachableC[local_id[state_num]] = 1;
      ++local_sizes[local_id[state_num]];
      if (!list->Node) {
        list->Node = node;
      }
      else {
        new_list = (struct StateListNode*)malloc(sizeof(struct StateListNode));
        new_list->Node = local_list->Node;
        new_list->Next = local_list->Next;
        local_list->Node = node;
        local_list->Next = new_list;
      }
    }

    // computation
    for (local_round = 0; local_round < local_scnt; local_round++) {
      if (!local_reachableC[local_round])
        continue;
      if (local_sizes[local_round] > biggest_size)
        biggest_size = local_sizes[local_round];
      local_list = local_stateListNode[local_round];
      f_viter(local_list, MAX_ITER, round, local_sizes[local_round]);
    }

    // free some used memory
    free(local_reachableC);
    free(local_sizes);
    for (local_round = 0; local_round < local_scnt; local_round++) {
      DeleteStateList(local_stateListNode[local_round]);
    }
    free(local_stateListNode);
  }

  else {
    if (size > biggest_size)
      biggest_size = size;
    f_viter(list, MAX_ITER, round, size);
  }

  // free additional memory
  free(state_hash);
  free(inv_state_hash);
  free(local_id);
  free(local_postI);
  free(local_postR);

}

#define CACHED_TVI

void f_viter(struct StateListNode *list, int MaxIter, int round, int component_size)
{
  int                       Iter;
  double                    diff, maxdiff, /* Bellman residual */
                            error = 0.0, time = 0.0, threshold = 50000.0;
  struct StateListNode     *stateListNode;
    clock_t     compStartTime = 0;
  
#ifdef __TEST__
    char *verify_list = (char *)malloc(sizeof(char) * strlen("verify_list_11"));
    sprintf(verify_list, "verify_list_%d", round);
    print_back_list(list, component_size, verify_list);
#endif
    

#ifdef CACHED_TVI
    if (component_size > COMP_SIZE_THRESHOLD) //(#of states in the round SCC)
    {
        compStartTime = clock();
        error = cache_aware_vi(list, MaxIter, round, component_size);
        time = (float)(clock()-compStartTime)/CLOCKS_PER_SEC;
        printf("Component in Cached Iter completed in: %f secs\n", time);
    }
    
    else
    {
#endif
        //for (Iter = 0; Iter < MaxIter; Iter++)
        Iter = 0;
        time = 0;
        
#ifndef CACHED_TVI
        if (component_size > COMP_SIZE_THRESHOLD) //(#of states in the round SCC)
            compStartTime = clock();
#endif
        do
        {
            maxdiff = 0.0;
            for (stateListNode = list; stateListNode && stateListNode->Node; stateListNode = stateListNode->Next)
            {
                if ((stateListNode->Node->Terminal != 1) && (stateListNode->Node->Terminal != 5))
                {
                    diff = BackupTwo_conv(stateListNode->Node);
                    if (diff > maxdiff)
                        maxdiff = diff;
                }
            }
//            if ( gDiscount < 1.0 )
//                error = (maxdiff * gDiscount)/(1.0 - gDiscount);
//            else
                error = maxdiff;
            Iter++;
            if (Iter % 100000 == 0)
                time = (float)(clock()-gStartTime)/CLOCKS_PER_SEC;
            if ( (error < gEpsilon) || (time > threshold) )
            {
                if (time > threshold)
                {
                    printf("Passed time threshold. Couldn't solve comp: %d in %d iterations. Error=%f\n",round, Iter, error);
                    printf("The size of component: %d is %d\n",round,sizes_ftv[round]);
                }
                if (component_size > COMP_SIZE_THRESHOLD)
                {
                    save_resulting_list(list, "non_cached_op", round, component_size);
#ifndef CACHED_TVI
                    time = (float)(clock()-compStartTime)/CLOCKS_PER_SEC;
                    printf("Component in unCached Iter completed in: %f secs\n", time);
#endif
                }
                return;
            }
        } while (error > gEpsilon);

#ifdef CACHED_TVI
    }
#endif
    
    if (error > gEpsilon)
    {
        printf("Couldn't solve component: %d, in %d iterations. Error=%f; Time=%f\n", round, MaxIter, error,time);
        printf("The size of component: %d is %d\n",round,sizes_ftv[round]);
    }
}

