/* stack.c -- implementation of an integer stack */

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

/****************** Global data structrues *********************/


/********************* Global functions **********************/

struct intStack* CreateStack( void )
{
  struct intStack *stack;
  
  stack = (struct intStack*)malloc(sizeof(struct intStack));
  stack->Number = -1;
  stack->Next = NULL;
  return( stack );
  //return NULL;
}

void deletestack(struct intStack *stack)
{
  struct intStack* tempStack;

  while( stack->Number != -1 ) {
    tempStack = stack;
    stack = stack->Next;
    free( tempStack );
  }
  free(stack);
}

int top(struct intStack* stack)
{
  return stack->Number;
}

void push(struct intStack* stack, int num)
{
  struct intStack *newStack;
  
  newStack = (struct intStack*)malloc(sizeof(struct intStack));
  newStack->Number = stack->Number;
  newStack->Next = stack->Next;
  stack->Number = num;
  stack->Next = newStack;
  //printstack(stack); 
}

int pop(struct intStack* stack)
{
  struct intStack *oldStack, *next;
  int value;
  
  if (stack->Number != -1) {
    value = stack->Number;
    //oldStack = stack;
    next = stack->Next;
    stack->Number = next->Number;
    stack->Next = next->Next;
    free(next);
    //printstack(stack);
    return value;
  }
  else
    return -1;
}

int empty(struct intStack* stack)
{
  if (stack->Number == -1)
    return 1;
  else
    return 0;
}

int instack(struct intStack* stack, int num)
{
  struct intStack* tempStack;
  
  for (tempStack = stack; tempStack->Number != -1; tempStack = tempStack->Next) {
    if (tempStack->Number == num)
      return 1;
  }
  return 0;
}

void printstack(struct intStack* stack)
{
  struct intStack* tempStack;
  
  printf("\n");
  for (tempStack = stack; tempStack->Number != -1; tempStack = tempStack->Next) {
    printf("%d\n", tempStack->Number);
  }
}
