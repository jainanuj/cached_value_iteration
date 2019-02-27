/* graph.c -- state space */

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "track.h"

/****************** Global data structrues *********************/

/* vector of size gNumStates holds pointers to each state node */
struct StateNode**  StateIndex;

/* start and goal states */
struct StateNode*   Start;
struct StateNode*   Goal;
long numBackups;

/********************* StateNode functions **********************/

struct StateNode *CreateStateNode( void )
{
  struct StateNode *node;

  node = (struct StateNode*)malloc(sizeof(struct StateNode));
  /* set default values */
    node->Action = NULL;
    node->BestAction = NULL;
    node->BestPrevAction = NULL;
    node->BestPrevActions = NULL;
    node->Description = NULL;
    node->PrevAction = NULL;
    node->Pi = NULL;
    node->Converged=0;
    node->Expanded=-1;
    node->f=0.0;
    node->fprime=0.0;
    node->fWeight=0.0;
    node->g=0.0;
    node->goalProb=0.0;
    node->h=0.0;
    node->hg=0.0;
    node->meanFirstPassage=0.0;
    node->StateNo=-1;
    node->Terminal=-1;
    node->Update=-1;
    node->Visited=-1;
    node->VPIheuristic=0.0;
    node->w=0.0;

  return( node );
}

/********************* ActionNode functions *********************/

struct ActionNode *CreateActionNode( void )
{
 struct ActionNode *node;

 node = (struct ActionNode*)malloc(sizeof(struct ActionNode));
 /* set default values */
    node->Description = NULL;
    node->NextState = NULL;
    node->PrevState = NULL;
    node->Dominated=0;
    node->StateNo=-1;
 return( node );
}

/******************** StateDistribution functions ***************/

struct StateDistribution *CreateStateDistribution( void )
{
  struct StateDistribution *x;

  x = (struct StateDistribution*)malloc(sizeof(struct StateDistribution));
  /* set default values */
    x->Next = NULL;
    x->State = NULL;
    x->Prob = 0.0;
  return( x );
}

/******************* StateListNode functions ********************/

struct StateListNode *CreateStateList(void)
{
  struct StateListNode *list;

  list = (struct StateListNode*)malloc((unsigned)sizeof(struct StateListNode));
  list->Node = NULL;
  list->Next = NULL;
  return(list);
}

void AppendStateList(struct StateListNode *list, struct StateNode *node)
/* Append node to front of state list */
{
  struct StateListNode *newStateList;
  
  if (!list->Node) { /* consider possibility of empty list */ 
    list->Node = node;
      list->Next = NULL;
  }
  else {
    newStateList = (struct StateListNode*)
      malloc((unsigned)sizeof(struct StateListNode));       //Fixed the BIGBUG sizeof(sizeof(
    newStateList->Node = list->Node;
    newStateList->Next = list->Next;
    list->Node = node;
    list->Next = newStateList;
  }
}

void DeleteStateList(struct StateListNode *list)
/* Delete list but don't delete states in list */
{
  struct StateListNode *temp;
  
   while( list != NULL ) {
      temp = list;
      list = list->Next;
      free( temp );
   }
}

void PrintStateList(struct StateListNode *list)
/* print the state numbers in a list */
{
  printf("\n");
  while( list != NULL ) {
    printf("%d\t", list->Node->StateNo);
    list = list->Next;
  }
}

/* Add function to check for empty state list */

void DisplayStateList(struct StateListNode *list)
{
  struct StateListNode *node;

  for (node = list; node; node = node->Next) {
    printf("\nindex %d State %s f %f",
	   node->Node->StateNo,
	   node->Node->Description,
	   node->Node->f);
    if (node->Node->Terminal != 1)
      printf(" best action %d", node->Node->BestAction->ActionNo);
    printf(" meanFP %f", node->Node->meanFirstPassage);
  }
  printf("\nPress return"); getchar();
}

/********************* ActionListNode functions ********************/

struct ActionListNode *CreateActionList(void)
{
  struct ActionListNode *list;

  list = (struct ActionListNode*)
    malloc((unsigned)sizeof(struct ActionListNode));
  list->Node = NULL;
  list->Next = NULL;
  return(list);
}

void AppendActionList(struct ActionListNode *list, struct ActionNode *node)
/* Append node to front of action list */
{
  struct ActionListNode *newActionList;
  
  if (!node) return;
  if (!list->Node) { /* consider possibility of empty list */ 
    list->Node = node;
      list->Next = NULL;
  }
  else {
    newActionList = (struct ActionListNode*)
    malloc((unsigned)sizeof(struct ActionListNode));      //BIGBUG-malloc((unsigned)sizeof(sizeof(struct ActionListNode)));
    newActionList->Node = list->Node;
    newActionList->Next = list->Next;
    list->Node = node;
    list->Next = newActionList;
  }
}

void RemoveActionList(struct ActionListNode *list, struct ActionNode *node)
/* Remove a node from action list */
{
  struct ActionListNode *newlist, *plist;
  struct ActionNode *newNode;
  int count;
  
  if (!list->Node) { /* consider possibility of empty list */ 
    return;
  }
  if (!node) {
    return;
  }
  if (IsSameActionNode(list->Node, node)) {
    newlist = list;
//    list = list->Next;
    list->Node = list->Next->Node;
    list->Next = list->Next->Next;
    free(newlist);
    return;
  }
  plist = list;
  newlist = list->Next;
  //count = 0;
  if (!newlist)
    return;
  //while (newlist->Node) {
  while (newlist) {  
    /*count++;
    if (count == 5)
      return;*/
    if (IsSameActionNode(newlist->Node, node)) {
//      plist->Next = newlist->Next;
      plist->Next->Node = newlist->Next->Node;
      plist->Next->Next = newlist->Next->Next;
      free(newlist);
      return;
    }
    plist = newlist;
    newlist = newlist->Next;
  }
  return;
}

int IsSameActionNode(struct ActionNode *node1, struct ActionNode *node2)
/* if same, return 1, else return 0 */
{
  if ((node1->StateNo == node2->StateNo) && (node1->ActionNo == node2->ActionNo))
    return 1;
  return 0;
}

void DeleteActionList(struct ActionListNode *list)
/* Delete list but don't delete actions in list */
{
  struct ActionListNode *node;

  for (node = list; node; node = node->Next)
      free(node);
}

void printaction(struct ActionNode* action) {
  printf("State: %d Action: %d\n", action->StateNo, action->ActionNo);
}
