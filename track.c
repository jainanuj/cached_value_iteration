/* track.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "solve.h"
#include "graph.h"
#include "lao.h"
#include "vi.h"
#include "backup.h"
#include "components.h"
#include "setgoal.h"

#define MAX 9999.99

double gInitValueSum = 0.0;

/* The racetrack input file is generated by a pearl script called
   build.pl which is run on a map of the track.

   Format for racetrack input file:
     first line: 
         total number of states
	 discount factor
     second line: 
         list of start states (equal probability of starting in each)
     subsequent lines: 
         state index
	 state description
	 action index
	 0 if regular state, 1 if GOAL, 2 if WALL
	 (goal probability -- not used, is 1.0 for GOAL, otherwise 0.0)
	 next state if action succeeeds
	 next state if action fails

   After generating input file using pearl script, make the following
   changes: 
    -- add the first two lines
    -- change terminal value for WALL from 1 to 2 so that the wall 
       can be distinguished from the goal state.

   Note that hitting the wall causes a transition to a start state.

   The start state (-1) and wall state have zero cost transition to
   uniform probability distribution over possible start states.
*/

int random_start(void)
{
  srand(time(NULL));
  return rand() % gNumStates;
}

void readRacetrackMDP (void)
{
  FILE*                     file;
  char                      description[15];
  char                      str[30]; /* for reading lines of input file */
  int                       state, action, nextStateSuccess, nextStateFail,
                            numFields, terminal, start[6], i;
  float                     goalProb;
  struct StateNode*         stateNode;
  struct ActionNode*        actionNode;
  struct ActionListNode*    actionList;
  struct StateDistribution* Distribution;
  double                    majorProb = gRTProb;
  double                    minorProb = 1.0 - majorProb;

  file = fopen(gInputFileName, "r");
  if ( file == NULL) {
    printf("Data file %s not found!\n", gInputFileName);
    exit(1);
  }

  fgets(str, 100, file); 
  sscanf(str, "%d %lf", &gNumStates, &gDiscount); 
  gNumActions = 9; /* racetrack problem has 9 actions */

  /* for testing purposes only */
  /*printf("\nSize of state: %d", sizeof(struct StateNode*));
  printf("\nSize of action: %d", sizeof(struct ActionNode*));
  printf("\nSize of state distribution: %d", sizeof(struct StateDistribution*));*/
  
  /* allocate vector of state nodes */
  StateIndex = 
    (struct StateNode**)malloc((gNumStates+1)*sizeof(struct StateNode*));
  for (state = 0; state < gNumStates; state++) {
    StateIndex[state] = CreateStateNode();
    StateIndex[state]->StateNo = state;
    /* also allocate memory for state description */
    StateIndex[state]->Description = (char*)malloc(15*sizeof(char));
    /* set initial action lists to NULL */
    StateIndex[state]->Action = NULL;
    StateIndex[state]->PrevAction = NULL;
    StateIndex[state]->BestPrevActions = NULL;
    StateIndex[state]->BestPrevAction = NULL;
    /* Initial heuristic cost-to-go is zero */
    StateIndex[state]->f = 0.0;
    StateIndex[state]->fprime = MAX;
    StateIndex[state]->goalProb = 0.0;
    StateIndex[state]->Converged = 0;
    if (state == 0)
      StateIndex[state]->w = 1.0;
    else
      StateIndex[state]->w = 0.0;
  }

  /* build start state */
  fgets(str, 100, file); 
  numFields = sscanf(str, "%d %d %d %d %d %d",
          &start[0], &start[1], &start[2], &start[3], &start[4], &start[5]);
  Start = CreateStateNode();
  Start->StateNo = -1;
  Start->Description = "START";
  Start->f = 0.0;
  Start->fprime = MAX;
  //Start->Terminal = 0;
  Start->Terminal = 3;	/* for special use only */
  Start->Expanded = 0;
  Start->Visited = 0;
  Start->Converged = 0;
  Start->Action = CreateActionList();
  Start->Action->Next = NULL;
  Start->Action->Node = Start->BestAction = CreateActionNode();
  Start->Action->Node->StateNo = -1;
  Start->Action->Node->ActionNo = -1;
  Start->Action->Node->Visited = 0;
  Start->Action->Node->Dominated = 0;
  Start->Action->Node->Cost = 0.0;
  Start->Action->Node->Dominated = 0;
  Start->Action->Node->PrevState = Start;
  Start->Action->Node->NextState = CreateStateDistribution();
  Distribution = Start->Action->Node->NextState;
  Distribution->State = StateIndex[start[0]];
  Distribution->Prob = 1.0/numFields;
  StateIndex[start[0]]->PrevAction = CreateActionList();
  AppendActionList(StateIndex[start[0]]->PrevAction, Start->BestAction);
  StateIndex[start[0]]->BestPrevActions = CreateActionList();
  AppendActionList(StateIndex[start[0]]->BestPrevActions, Start->BestAction);
  StateIndex[start[0]]->BestPrevAction = Start->BestAction;
  for (i = 1; i < numFields; i++) {
    Distribution->Next = CreateStateDistribution();
    Distribution = Distribution->Next;
    Distribution->State = StateIndex[start[i]];
    Distribution->Prob = 1.0/numFields;
    StateIndex[start[i]]->PrevAction = CreateActionList();
    AppendActionList(StateIndex[start[i]]->PrevAction, Start->BestAction);
    StateIndex[start[i]]->BestPrevActions = CreateActionList();
    AppendActionList(StateIndex[start[i]]->BestPrevActions, Start->BestAction);
    StateIndex[start[i]]->BestPrevAction = Start->BestAction;
    printf("%d", start[i]);
  }
  Distribution->Next = NULL;
  
  /* each input line begins with state number and contains 
     transition probs for a state-action pair */
  while (!feof(file)) {
    fgets(str, 100, file); 
    numFields = sscanf(str, "%d %s %d %d %f %d %d",
           &state,
           description,
           &action,
           &terminal,
           &goalProb,
           &nextStateSuccess,
           &nextStateFail);
    stateNode = StateIndex[state];
    strncpy(stateNode->Description, description, 15);
    stateNode->Terminal = terminal;
    stateNode->Visited = 0;
    stateNode->Converged = 0;

    /* Only non-terminal states have next states. In a non-terminal 
       state, every action has two next states and a cost of 1.0. */
    if (stateNode->Terminal == 0) {

      /* allocate memory for action and add to list of actions */
      actionList = CreateActionList();
      actionNode = CreateActionNode();
      actionNode->StateNo = state;
      actionNode->ActionNo = action;
      actionNode->Visited = 0;
      actionNode->Dominated = 0;
      actionList->Node = actionNode;
      /* insert new action node at beginning of list */
      actionList->Next = stateNode->Action;
      stateNode->Action = actionList;
      /* create best action is it doesn't already exist */
      if (!stateNode->BestAction){
        //printf("\nhere1");
        stateNode->BestAction = actionNode;
        StateIndex[nextStateSuccess]->BestPrevAction = actionNode;
      }
      /* create best prev action here */
      /*  */

      /* Update PrevAction of the first successor state */
      /* Allocate memory for next state distributions */
      actionNode->NextState = CreateStateDistribution();
      actionNode->NextState->State = StateIndex[nextStateSuccess];
      /* Also add pointers from next states back to this action. */
      actionList = CreateActionList();
      actionList->Node = actionNode;
      actionList->Next = StateIndex[nextStateSuccess]->PrevAction;
      StateIndex[nextStateSuccess]->PrevAction = actionList;
      /*actionList = CreateActionList();
      actionList->Node = actionNode;
      actionList->Next = StateIndex[nextStateSuccess]->BestPrevActions;
      StateIndex[nextStateSuccess]->BestPrevActions = actionList;*/
      if (numFields == 7) { /* 2 possible next states */
        /* actions have intended effect with probability 0.9 */
        /* actions have intended effect with probability gRTProb */
//        actionNode->NextState->Prob = 0.9;
        actionNode->NextState->Prob = majorProb;
        actionNode->NextState->Next = CreateStateDistribution();
        actionNode->NextState->Next->State = StateIndex[nextStateFail];
        actionNode->NextState->Next->Prob = 0.1;
        actionNode->NextState->Next->Prob = minorProb;
        actionNode->NextState->Next->Next = NULL;
        /* Also add pointer from next states back to this action. */
        actionList = CreateActionList();
        actionList->Node = actionNode;
        actionList->Next = StateIndex[nextStateFail]->PrevAction;
        StateIndex[nextStateFail]->PrevAction = actionList;
        /*actionList = CreateActionList();
        actionList->Node = actionNode;
        actionList->Next = StateIndex[nextStateFail]->BestPrevActions;
        StateIndex[nextStateFail]->BestPrevActions = actionList;*/
      }
      else { /* 1 possible next state */
        actionNode->NextState->Prob = 1.0;
        actionNode->NextState->Next = NULL;
      }
      /* cost of every action is 1.0 */
      actionNode->Cost = 1.0;
      actionNode->w = 1.0;
      /* pointer to state in which action is taken */
      actionNode->PrevState = stateNode;
    }
    else if (stateNode->Terminal == 1) {
      stateNode->f = 0.0;
      stateNode->fprime = MAX;
      stateNode->goalProb = 1.0;
      Goal = stateNode;
    }
    else if (stateNode->Terminal == 2) { /* wall */
      stateNode->f = 0.0;
      stateNode->fprime = MAX;
      stateNode->Action = CreateActionList();
      stateNode->Action->Next = NULL;
      stateNode->Action->Node = stateNode->BestAction = CreateActionNode();
      stateNode->Action->Node->ActionNo = -1;
      stateNode->Action->Node->Cost = 0.0;
      stateNode->Action->Node->PrevState = stateNode;
      stateNode->Action->Node->NextState = 
	Start->Action->Node->NextState;
      /* make WALL parent of START */
      actionList = CreateActionList();
      actionList->Node = stateNode->Action->Node;
      actionList->Next = Start->PrevAction;
      Start->PrevAction = actionList;
    }
  }
  fclose(file);
  
  /*for (state = 0; state < gNumStates; state++) {
    struct ActionListNode *bestActionList = StateIndex[state]->BestPrevActions;
    printf("\nState: %d", state);
    while (bestActionList) {
      printf("\n  %d %d", bestActionList->Node->PrevState->StateNo, bestActionList->Node->ActionNo);
      bestActionList = bestActionList->Next;
    }
  }*/
}

void readRanMDP (void)
{
  FILE*                     file;
  char                      description[15];
  char                      str[1024]; /* for reading lines of input file */
  int                       state, action, nextStateSuccess, nextStateFail,
                            numFields, terminal, start[6], i, succs,
			    next[50], state_num;
  float                     goalProb, prob[50];
  struct StateNode*         stateNode;
  struct ActionNode*        actionNode;
  struct ActionListNode*    actionList;
  struct StateDistribution* Distribution, *nextStates;

  file = fopen(gInputFileName, "r");
  if ( file == NULL) {
    printf("Data file %s not found!\n", gInputFileName);
    exit(1);
  }

  fgets(str, 1024, file); 
  sscanf(str, "%d %lf", &gNumStates, &gDiscount); 
  gNumActions = 9; /* racetrack problem has 9 actions */

  /* allocate vector of state nodes */
  StateIndex = 
    (struct StateNode**)malloc((gNumStates+1)*sizeof(struct StateNode*));
  for (state = 0; state < gNumStates; state++) {
    StateIndex[state] = CreateStateNode();
    StateIndex[state]->StateNo = state;
    /* also allocate memory for state description */
    StateIndex[state]->Description = (char*)malloc(15*sizeof(char));
    /* set initial action lists to NULL */
    StateIndex[state]->Action = NULL;
    StateIndex[state]->PrevAction = NULL;
    StateIndex[state]->BestPrevActions = NULL;
    StateIndex[state]->BestPrevAction = NULL;
    /* Initial heuristic cost-to-go is zero */
    StateIndex[state]->f = 0.0;
    StateIndex[state]->fprime = MAX;
    StateIndex[state]->goalProb = 0.0;
    StateIndex[state]->Converged = 0;
    if (state == 0)
      StateIndex[state]->w = 1.0;
    else
      StateIndex[state]->w = 0.0;
  }

  /* build start state */
  fgets(str, 1024, file); 
  numFields = sscanf(str, "%d %d %d %d %d %d",
	   &start[0], &start[1], &start[2], &start[3], &start[4], &start[5]);
  Start = CreateStateNode();
  Start->StateNo = -1;
  Start->Description = "START";
  Start->f = 0.0;
  Start->fprime = MAX;
  //Start->Terminal = 0;
  Start->Terminal = 3;	/* for special use only */
  Start->Expanded = 0;
  Start->Visited = 0;
  Start->Converged = 0;
  Start->Action = CreateActionList();
  Start->Action->Next = NULL;
  Start->Action->Node = Start->BestAction = CreateActionNode();
  Start->Action->Node->ActionNo = -1;
  Start->Action->Node->Cost = 0.0;
  Start->Action->Node->Visited = 0;
  Start->Action->Node->Dominated = 0;
  Start->Action->Node->PrevState = Start;
  Start->Action->Node->NextState = CreateStateDistribution();
  Distribution = Start->Action->Node->NextState;
  Distribution->State = StateIndex[start[0]];
  Distribution->Prob = 1.0/numFields;
  StateIndex[start[0]]->PrevAction = CreateActionList();
  AppendActionList(StateIndex[start[0]]->PrevAction, Start->BestAction);
  StateIndex[start[0]]->BestPrevActions = CreateActionList();
  AppendActionList(StateIndex[start[0]]->BestPrevActions, Start->BestAction);
  StateIndex[start[0]]->BestPrevAction = Start->BestAction;
  for (i = 1; i < numFields; i++) {
    Distribution->Next = CreateStateDistribution();
    Distribution = Distribution->Next;
    Distribution->State = StateIndex[start[i]];
    Distribution->Prob = 1.0/numFields;
    StateIndex[start[i]]->PrevAction = CreateActionList();
    AppendActionList(StateIndex[start[i]]->PrevAction, Start->BestAction);
    StateIndex[start[i]]->BestPrevActions = CreateActionList();
    AppendActionList(StateIndex[start[i]]->BestPrevActions, Start->BestAction);
    StateIndex[start[i]]->BestPrevAction = Start->BestAction;
  }
  Distribution->Next = NULL;
  
  /* each input line begins with state number and contains 
     transition probs for a state-action pair */
  while (!feof(file)) {
    fgets(str, 1024, file);
    //printf("%s", str);
    numFields = sscanf(str, "%d %d %d %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f %d %f",
	   &state,
	   &terminal,
	   &action,
	   &next[0],
	   &prob[0],
	   &next[1],
	   &prob[1],
	   &next[2],
	   &prob[2],
	   &next[3],
	   &prob[3],
	   &next[4],
	   &prob[4],
	   &next[5],
	   &prob[5],
	   &next[6],
	   &prob[6],
	   &next[7],
	   &prob[7], 
	   &next[8],
	   &prob[8],
	   &next[9],
	   &prob[9],
	   &next[10],
	   &prob[10],
	   &next[11],
	   &prob[11],
	   &next[12],
	   &prob[12],
	   &next[13],
	   &prob[13],
	   &next[14],
	   &prob[14],
	   &next[15],
	   &prob[15],
	   &next[16],
	   &prob[16],
	   &next[17],
	   &prob[17],
	   &next[18],
	   &prob[18],
	   &next[19],
	   &prob[19],
	   &next[20],
	   &prob[20],
	   &next[21],
	   &prob[21],
	   &next[22],
	   &prob[22],
	   &next[23],
	   &prob[23],
	   &next[24],
	   &prob[24],
	   &next[25],
	   &prob[25],
	   &next[26],
	   &prob[26],
	   &next[27],
	   &prob[27],
	   &next[28],
	   &prob[28],
	   &next[29],
	   &prob[29],
	   &next[30],
	   &prob[30],
	   &next[31],
	   &prob[31],
	   &next[32],
	   &prob[32],
	   &next[33],
	   &prob[33],
	   &next[34],
	   &prob[34],
	   &next[35],
	   &prob[35],
	   &next[36],
	   &prob[36],
	   &next[37],
	   &prob[37],
	   &next[38],
	   &prob[38],
	   &next[39],
	   &prob[39]);
    stateNode = StateIndex[state];
    //strncpy(stateNode->Description, description, 15);
    stateNode->Terminal = terminal;
    stateNode->Visited = 0;

    /* Only non-terminal states have next states. */
    if (stateNode->Terminal == 0) {
      /* allocate memory for action and add to list of actions */
      actionList = CreateActionList();
      actionNode = CreateActionNode();
      actionNode->StateNo = state;
      actionNode->ActionNo = action;
      actionNode->Visited = 0;
      actionNode->Dominated = 0;
      actionList->Node = actionNode;
      /* insert new action node at the beginning of actionList */
      actionList->Next = stateNode->Action;
      stateNode->Action = actionList;
      /* create best action if it doesn't already exist */
      
      numFields -= 3;
      succs = numFields / 2;
      //printf("%d\n", succs);
      state_num = 0;
      while (succs > 0) { /* reading successor states one by one*/
        nextStates = CreateStateDistribution();
        nextStates->Prob = prob[state_num];
        nextStates->State = StateIndex[next[state_num]];
	nextStates->Next = actionNode->NextState;
	actionNode->NextState = nextStates;
	state_num++;
	succs--;
      }
      /* Also add pointer from next states back to this action. */
      succs = numFields / 2;
      state_num = 0;
      while (succs > 0) { /* reading successor states one by one*/
	actionList = CreateActionList();
	actionList->Node = actionNode;
	actionList->Next = StateIndex[next[state_num]]->PrevAction;
	StateIndex[next[state_num]]->PrevAction = actionList;
	if (!stateNode->BestAction) {
	  actionList = CreateActionList();
	  actionList->Node = actionNode;
	  actionList->Next = StateIndex[next[state_num]]->BestPrevActions;
	  StateIndex[next[state_num]]->BestPrevActions = actionList;
	}
	if (!StateIndex[next[state_num]]->BestPrevAction)
          StateIndex[next[state_num]]->BestPrevAction = actionNode;
	state_num++;
	succs--;
      }
      if (!stateNode->BestAction){
        stateNode->BestAction = actionNode;
      }
      actionNode->Cost = 1.0;
      actionNode->w = 1.0;
      actionNode->PrevState = stateNode;
    }
    else if (stateNode->Terminal == 1) {
      stateNode->f = 0.0;
      stateNode->fprime = MAX;
      stateNode->goalProb = 1.0;
      Goal = stateNode;
    }
  }
  /* debuging begin 
  for (i = 0; i < gNumStates; i++) {
    printf("%d:\n", StateIndex[i]->StateNo);
    actionList = StateIndex[i]->PrevAction;
    while (actionList) {
      printf("%d %d\n", actionList->Node->ActionNo, actionList->Node->PrevState->StateNo);
      actionList = actionList->Next;
    }
    printf("\n");
  }*/
  /* debugging end*/
  fclose(file);
  printf("Reading complete\n");
}

void readRanMDPGoals(void)
{
  readRanMDP();
  loadgoal(gGoalNum);
}

void readMDP (void)
{
  FILE*                     file;
  char                      description[15];
  char                      str[1024]; /* for reading lines of input file */
  int                       state, numActions, action, nextStateSuccess, nextStateFail,
                            numFields, terminal, start[6], i, succs, numSuccs, succ,
			    next[50], state_num, isTerminal;
  float                     goalProb, prob, reward;
  struct StateNode*         stateNode;
  struct ActionNode*        actionNode;
  struct ActionListNode*    actionList;
  struct StateDistribution* Distribution, *nextStates;

  file = fopen(gInputFileName, "r");
  if ( file == NULL) {
    printf("Data file %s not found!\n", gInputFileName);
    exit(1);
  }

//  fgets(str, 1024, file);
//  sscanf(str, "%d", &gNumStates);
    fscanf(file, "%d\n", &gNumStates);
  gDiscount = 1.0;

  /* allocate vector of state nodes */
  StateIndex = (struct StateNode**)malloc((gNumStates+1)*sizeof(struct StateNode*));
  for (state = 0; state < gNumStates; state++) {
    StateIndex[state] = CreateStateNode();
    StateIndex[state]->StateNo = state;
    /* also allocate memory for state description */
    StateIndex[state]->Description = (char*)malloc(15*sizeof(char));
    /* set initial action lists to NULL */
    StateIndex[state]->Action = NULL;
    StateIndex[state]->PrevAction = NULL;
    StateIndex[state]->BestPrevActions = NULL;
    StateIndex[state]->BestPrevAction = NULL;
    /* Initial heuristic cost-to-go is zero */
    StateIndex[state]->f = 0.0;
    StateIndex[state]->fprime = MAX;
    StateIndex[state]->goalProb = 0.0;
    StateIndex[state]->Converged = 0;
    if (state == 0)
      StateIndex[state]->w = 1.0;
    else
      StateIndex[state]->w = 0.0;
  }

  /* build start state */
  numFields = 1;
//  start[0] = 0; // all IPC problems, sap problems
//  start[0] = 1750; // mcar100, mcar300, mcar500 p2
//  start[0] = 100; // sap problems
//  start[0] = 101781; // mcar500 p1
//  start[0] = 80000; // dap20
  int initial = random_start();
  start[0] = initial;
  if (gInitialState >= 0)
    start[0] = gInitialState;
  else
    gInitialState = initial;
  printf("\nInitial state: %d", start[0]);
  Start = CreateStateNode();
  Start->StateNo = -1;
  Start->Description = "START";
  Start->f = 0.0;
  Start->fprime = MAX;
  //Start->Terminal = 0;
  Start->Terminal = 3;	/* for special use only */
  Start->Expanded = 0;
  Start->Action = CreateActionList();
  Start->Action->Next = NULL;
  Start->Action->Node = Start->BestAction = CreateActionNode();
  Start->Action->Node->ActionNo = -1;
  Start->Action->Node->Cost = 0.0;
  Start->Action->Node->PrevState = Start;
  Start->Action->Node->NextState = CreateStateDistribution();
  Distribution = Start->Action->Node->NextState;
  Distribution->State = StateIndex[start[0]];
  Distribution->Prob = 1.0;
  StateIndex[start[0]]->PrevAction = CreateActionList();
  AppendActionList(StateIndex[start[0]]->PrevAction, Start->BestAction);
  StateIndex[start[0]]->BestPrevAction = Start->BestAction;
  for (i = 1; i < numFields; i++) {
    Distribution->Next = CreateStateDistribution();
    Distribution = Distribution->Next;
    Distribution->State = StateIndex[start[i]];
    Distribution->Prob = 1.0/numFields;
    StateIndex[start[i]]->PrevAction = CreateActionList();
    AppendActionList(StateIndex[start[i]]->PrevAction, Start->BestAction);
    StateIndex[start[i]]->BestPrevActions = CreateActionList();
    AppendActionList(StateIndex[start[i]]->BestPrevActions, Start->BestAction);
    StateIndex[start[i]]->BestPrevAction = Start->BestAction;
  }
  Distribution->Next = NULL;
  
  /* each input line begins with state number the number of actions it has */
  for (state = 0; state < gNumStates; state++) {
    //printf("\nLoading state %d", state);
    isTerminal = 1;
    fscanf(file, "%d %d\n", &state, &numActions);
    stateNode = StateIndex[state];
    
    if (numActions == 0) {
      stateNode->Terminal = 5;
      continue;
    }

      stateNode->num_actions = numActions;
    for (action = 0; action < numActions; action++) {
      fscanf(file, "%f %d ", &reward, &numSuccs);
      if (reward > 0) {
        //printf("\nState number: %d Reward: %f", state, reward);
        stateNode->Terminal = 1;
        stateNode->goalProb = 1.0;
        Goal = stateNode;
        continue;
      }
      
      else if (numSuccs == 0) {
	if ((action == (numActions - 1)) && (isTerminal == 1)) {
          //printf("\nState number: %d Reward: %f", state, reward);
          stateNode->Terminal = 5;
          stateNode->f = MAX;
	  stateNode->h = MAX;
          stateNode->fprime = MAX;
	}
      }
      
      else {        //Its a regular node with a regular action. Parse all successors in the action
        //printf("\nHere state: %d", stateNode->StateNo);
        isTerminal = 0;
        stateNode->Terminal = 0;
        /* allocate memory for action and add to list of actions */
        actionList = CreateActionList();
        actionNode = CreateActionNode();
	actionNode->StateNo = state;
        actionNode->ActionNo = action;
        actionList->Node = actionNode;
        /* insert new action node at the beginning of actionList */
        actionList->Next = stateNode->Action;
        stateNode->Action = actionList;
        /* create best action if it doesn't already exist */
        if (!stateNode->BestAction) 
	  stateNode->BestAction = actionNode;
      
        for (succs = 0; succs < numSuccs; succs++) {
          fscanf(file, "%d %f ", &succ, &prob);
          nextStates = CreateStateDistribution();
          nextStates->Prob = prob;
          nextStates->State = StateIndex[succ];
	  nextStates->Next = actionNode->NextState;
	  actionNode->NextState = nextStates;
      
	  actionList = CreateActionList();
	  actionList->Node = actionNode;
	  actionList->Next = StateIndex[succ]->PrevAction;
	  StateIndex[succ]->PrevAction = actionList;
          
	  if (!StateIndex[succ]->BestPrevAction) {
            /*actionList = CreateActionList();
            actionList->Node = actionNode;
            actionList->Next = StateIndex[succ]->BestPrevActions;
            StateIndex[succ]->BestPrevActions = actionList;*/
            StateIndex[succ]->BestPrevAction = actionNode;
	  }
        }
        fscanf(file, "\n");
      
        actionNode->Cost = 1.0;
        actionNode->w = 1.0;
        actionNode->PrevState = stateNode;
      }
    }
  }
  /*for (i = 0; i < gNumStates; i++) {
    if (StateIndex[i]->Terminal == 5)
      printf("\n%d %d", StateIndex[i]->StateNo, StateIndex[i]->Terminal);
  }*/
  fclose(file);
  printf("\nReading complete\n");
  fflush(0);
}

void readWetfloorMDP (void)
{
  FILE*                     file;
  char                      description[15];
  char                      str[1024]; /* for reading lines of input file */
  int                       state, action, nextStateSuccess, nextStateFail,
                            numFields, terminal, start[6], i;
  int                       succ1, succ2, succ3, succ4;
  double                    f, fCost, prob1, prob2, prob3, prob4;
  float                     goalProb;
  struct StateNode*         stateNode;
  struct ActionNode*        actionNode;
  struct ActionListNode*    actionList;
  struct StateDistribution* Distribution;

  printf("\nLoading wet floor problem...");
  fflush(0);
  file = fopen(gInputFileName, "r");
  if ( file == NULL) {
    printf("Data file %s not found!\n", gInputFileName);
    exit(1);
  }

  fgets(str, 100, file); 
  sscanf(str, "%d %lf", &gNumStates, &gDiscount); 
  gNumActions = 9; /* racetrack problem has 9 actions */

  /* allocate vector of state nodes */
  StateIndex = 
    (struct StateNode**)malloc((gNumStates+1)*sizeof(struct StateNode*));
  for (state = 0; state < gNumStates; state++) {
    StateIndex[state] = CreateStateNode();
    StateIndex[state]->StateNo = state;
    /* also allocate memory for state description */
    StateIndex[state]->Description = (char*)malloc(15*sizeof(char));
    /* set initial action lists to NULL */
    StateIndex[state]->Action = NULL;
    StateIndex[state]->PrevAction = NULL;
    StateIndex[state]->BestPrevActions = NULL;
    StateIndex[state]->BestPrevAction = NULL;
    /* Initial heuristic cost-to-go is zero */
    StateIndex[state]->f = 0.0;
    StateIndex[state]->fprime = MAX;
    StateIndex[state]->goalProb = 0.0;
    StateIndex[state]->Converged = 0;
    if (state == 0)
      StateIndex[state]->w = 1.0;
    else
      StateIndex[state]->w = 0.0;
  }

  /* build start state */
  fgets(str, 100, file); 
  numFields = sscanf(str, "%d %d %d %d %d %d",
	   &start[0], &start[1], &start[2], &start[3], &start[4], &start[5]);
  Start = CreateStateNode();
  Start->StateNo = -1;
  Start->Description = "START";
  Start->f = MAX;
  Start->g = MAX;
  Start->fprime = MAX;
  //Start->Terminal = 0;
  Start->Terminal = 3;	/* for special use only */
  Start->Expanded = 0;
  Start->Visited = 0;
  Start->Action = CreateActionList();
  Start->Action->Next = NULL;
  Start->Action->Node = Start->BestAction = CreateActionNode();
  Start->Action->Node->StateNo = -1;
  Start->Action->Node->ActionNo = -1;
  Start->Action->Node->Visited = 0;
  Start->Action->Node->Cost = 0.0;
  Start->Action->Node->PrevState = Start;
  Start->Action->Node->NextState = CreateStateDistribution();
  Distribution = Start->Action->Node->NextState;
  Distribution->State = StateIndex[start[0]];
  Distribution->Prob = 1.0/numFields;
  StateIndex[start[0]]->PrevAction = CreateActionList();
  AppendActionList(StateIndex[start[0]]->PrevAction, Start->BestAction);
  StateIndex[start[0]]->BestPrevActions = CreateActionList();
  AppendActionList(StateIndex[start[0]]->BestPrevActions, Start->BestAction);
  StateIndex[start[0]]->BestPrevAction = Start->BestAction;
  for (i = 1; i < numFields; i++) {
    Distribution->Next = CreateStateDistribution();
    Distribution = Distribution->Next;
    Distribution->State = StateIndex[start[i]];
    Distribution->Prob = 1.0/numFields;
    StateIndex[start[i]]->PrevAction = CreateActionList();
    AppendActionList(StateIndex[start[i]]->PrevAction, Start->BestAction);
    StateIndex[start[i]]->BestPrevActions = CreateActionList();
    AppendActionList(StateIndex[start[i]]->BestPrevActions, Start->BestAction);
    StateIndex[start[i]]->BestPrevAction = Start->BestAction;
    printf("%d", start[i]);
  }
  Distribution->Next = NULL;

  /* each input line begins with state number and contains 
     transition probs for a state-action pair */
  while (!feof(file)) {
    bzero(str, 1024);
    fgets(str, 1024, file); 
    if (strlen(str) == 0)
      break;
    bzero(description, 15);
    numFields = sscanf(str, "%d %s %d %d %d %lf %d %lf %d %lf %d %lf", &state, description, &action, &terminal, &succ1, &prob1, &succ2, &prob2, &succ3, &prob3, &succ4, &prob4);
    stateNode = StateIndex[state];
    //strncpy(stateNode->Description, description, 15);
    stateNode->Terminal = terminal;
    stateNode->Visited = 0;

    /* Only non-terminal states have next states. In a non-terminal 
       state, every action has two next states and a cost of 1.0. */
    if (stateNode->Terminal == 0) {
      /* allocate memory for action and add to list of actions */
      actionList = CreateActionList();
      actionNode = CreateActionNode();
      actionNode->StateNo = state;
      actionNode->ActionNo = action;
      actionNode->Visited = 0;
      actionList->Node = actionNode;
      /* insert new action node at beginning of list */
      actionList->Next = stateNode->Action;
      stateNode->Action = actionList;
      /* create best action is it doesn't already exist */
      if (!stateNode->BestAction){
        stateNode->BestAction = actionNode;
      }
      actionNode->NextState = CreateStateDistribution();
      actionNode->NextState->State = StateIndex[succ1];

      actionList = CreateActionList();
      actionList->Node = actionNode;
      actionList->Next = StateIndex[succ1]->PrevAction;
      StateIndex[succ1]->PrevAction = actionList;
      if (!StateIndex[succ1]->BestPrevAction) 
        StateIndex[succ1]->BestPrevAction = actionNode;

      if (numFields > 5) { /* 4 possible next states */
        actionNode->NextState->Prob = prob1;
        actionNode->NextState->Next = CreateStateDistribution();
        actionNode->NextState->Next->State = StateIndex[succ2];
        actionNode->NextState->Next->Prob = prob2;
        actionNode->NextState->Next->Next = CreateStateDistribution();
        actionNode->NextState->Next->Next->State = StateIndex[succ3];
        actionNode->NextState->Next->Next->Prob = prob3;
        actionNode->NextState->Next->Next->Next = CreateStateDistribution();
        actionNode->NextState->Next->Next->Next->State = StateIndex[succ4];
        actionNode->NextState->Next->Next->Next->Prob = prob4;
        actionNode->NextState->Next->Next->Next->Next = NULL;


        actionList = CreateActionList();
        actionList->Node = actionNode;
        actionList->Next = StateIndex[succ2]->PrevAction;
        StateIndex[succ2]->PrevAction = actionList;
        if (!StateIndex[succ2]->BestPrevAction) 
          StateIndex[succ2]->BestPrevAction = actionNode;

        actionList = CreateActionList();
        actionList->Node = actionNode;
        actionList->Next = StateIndex[succ3]->PrevAction;
        StateIndex[succ3]->PrevAction = actionList;
        if (!StateIndex[succ3]->BestPrevAction) 
          StateIndex[succ3]->BestPrevAction = actionNode;

        actionList = CreateActionList();
        actionList->Node = actionNode;
        actionList->Next = StateIndex[succ4]->PrevAction;
        StateIndex[succ4]->PrevAction = actionList;
        if (!StateIndex[succ4]->BestPrevAction) 
          StateIndex[succ4]->BestPrevAction = actionNode;
      }
      else { /* 1 possible next state */
        actionNode->NextState->Prob = 1.0;
        actionNode->NextState->Next = NULL;
      }
      /* cost of every action is 1.0 */
      actionNode->Cost = 1.0;
      actionNode->w = 1.0;
      /* pointer to state in which action is taken */
      actionNode->PrevState = stateNode;
    }
    else if (stateNode->Terminal == 1) {
      stateNode->f = 0.0;
      stateNode->g = 0.0;
      Goal = stateNode;
    }
  }
  fclose(file);
  
}

void SourceStates(void)
{
  int state;

  for (state = 0; state < gNumStates; state++) {
    if (!StateIndex[state]->PrevAction)
      printf("\nSourceState %d", state);
  }
}
 
double CreateHeuristic(void)
/****** This doesn't yet work correctly for discounted MDPs ******/
{
  struct StateListNode *CurrentList, *NewList, *node;
  struct StateNode *snode, *stateNode;
  struct ActionListNode *prev, *actionList;
  struct StateDistribution *nextState;
  struct ActionListNode *actionListNode;
  struct ActionNode *actionNode, *oldBestAction;
  int state, numReachable, consistent = 1;
  double max_value = 9999.99, temp, cur_best, cost;

  CurrentList = CreateStateList();
  numReachable = 0;

//  SourceStates();

  for (state = 0; state < gNumStates; state++) {
    snode = StateIndex[state];
    if (snode->Terminal == 1) {
      snode->g = 0.0;
      snode->f = 0.0;
      snode->h = 0.0;
      snode->fWeight = 0.0;
      snode->fprime = 0.0;
      numReachable++;
      AppendStateList(CurrentList, snode);
    }
    else {
      snode->g = 0.0;
      snode->f = max_value;
      snode->h = max_value;
      snode->fWeight = max_value;
      snode->fprime = max_value;
    }
  }

  /* Add unvisited states that can be reached by one backward step */
//  while (CurrentList->Node) { /* check for empty list */
  while ( consistent && (CurrentList->Node) ) { /* check for empty list */
    NewList = CreateStateList();
    /* For each state added in previous step ... */
    for (node = CurrentList; node; node = node->Next) {
      /* ... and for each parent of that state ... */
      for (prev = node->Node->PrevAction; prev && consistent; prev = prev->Next) {
        /* If parent has not already been updated, update heuristic and 
           add to list */
        stateNode = prev->Node->PrevState;
        cost = prev->Node->Cost;
        if (stateNode->h >= max_value) {
          stateNode->f = stateNode->h = gDiscount * node->Node->h + cost;
          // lower bound
          stateNode->fWeight = gWeight * stateNode->f;
          // upper bound
          cur_best = max_value;
          for (actionListNode = stateNode->Action; actionListNode; actionListNode = actionListNode->Next) {
            actionNode = actionListNode->Node;
            temp = prev->Node->Cost;
            for (nextState = actionNode->NextState; nextState; nextState = nextState->Next) {
              temp += gDiscount * nextState->Prob * nextState->State->fprime;
            }
            if (temp < cur_best)
              cur_best = temp;
          }
          stateNode->fprime = cur_best;
          /* update the best action */
//          oldBestAction = prev->Node->PrevState->BestAction;
          prev->Node->PrevState->BestAction = prev->Node;
          /* update the best previous action list */
          actionList = CreateActionList();
          actionList->Node = prev->Node;
          actionList->Next = node->Node->BestPrevActions;
          node->Node->BestPrevActions = actionList;
//          RemoveActionList(node->Node->BestPrevActions, oldBestAction);
          AppendStateList(NewList, prev->Node->PrevState);
          numReachable++;
        }
      }
    }
    DeleteStateList(CurrentList);
    CurrentList = NewList;
    NewList = NULL;
  }

  int i;
  double largest_diff = 0.0;
  for (i = 0; i < gNumStates; i++) {
    StateIndex[i]->f *= gHDiscount;
    StateIndex[i]->h *= gHDiscount;
    gInitValueSum += StateIndex[i]->f;
    double diff = StateIndex[i]->fprime - StateIndex[i]->f;
    if ( diff < -gEpsilon )
      printf("\nInconsistency at state %d %f %f", i, StateIndex[i]->f, StateIndex[i]->fprime);
    if ( diff > largest_diff )
      largest_diff = diff;
    if ( (StateIndex[i]->Terminal == 5) && (StateIndex[i]->f < max_value - 1e-6) )
      printf("\nError with deadend state value at state %d %f %f", i, StateIndex[i]->f, StateIndex[i]->fprime);
  }

  DeleteStateList(CurrentList);
  printf("\nNumber of goal reachable states: %d", numReachable);

  if (StateIndex[gInitialState]->f >= max_value) {
    printf("\nInitial state cannot reach goal");
    gQuitFlag = 1;
  }

  return largest_diff;
}

void initMeanFirstPassage (void)
{
  int state;

  for (state = 0; state < gNumStates; state++)
    if (StateIndex[state]->Terminal == 1)
      StateIndex[state]->meanFirstPassage = 0.0;
    else
      StateIndex[state]->meanFirstPassage = 200.0;
}

void initHeuristic (void)
{
  int state, states, s;
  int courses, c;
  int* credits;
  
  courses = 0;
  states = gNumStates;
  while (states != 1) {
    courses++;
    //states /= 4;
    states /= 3;
  }
  credits = (int*)malloc(courses * sizeof(int));
  for (state = 0; state < gNumStates; state++)
    if (StateIndex[state]->Terminal == 1)
      StateIndex[state]->h = 0.0;
    else {
      s = state;
      for (c = 0; c < courses; c++) {
        //credits[c] = s % 4;
        credits[c] = s % 3;
        //s = s / 4;
        s = s / 4;
      }
      StateIndex[state]->h = 0.0;
      for (c = 0; c < courses; c++) {
        //if (credits[c] != 3)
        if (credits[s] != 2)
          StateIndex[state]->h += 1.0;
      }
    }
  
}
