/* statequeue.c -- implementation of an action queue */

#include <stdio.h>
#include <stdlib.h>
#include "statequeue.h"

/****************** Global data structrues *********************/


/********************* Global functions **********************/

int Left( int i)
{
  return 2*i+1;
}

int Right(int i)
{
  return 2*i+2;
}

int Parent(int i)
{
  return (i-1)/2;
}

struct StateQueue* CreateQueue( int number )
{
  struct StateQueue *queue;
  int i;
  
  queue = (struct StateQueue*)malloc(sizeof(struct StateQueue));
  queue->queueNode = (struct QueueNode**)malloc((unsigned)number*sizeof(struct QueueNode*));
  queue->index = (int*)malloc(number*sizeof(int));
  queue->rindex = (int*)malloc(number*sizeof(int));
  for (i = 0; i < number; i++) {
    queue->queueNode[i] = (struct QueueNode*)malloc(sizeof(struct QueueNode));
    queue->index[i] = -2;
    queue->rindex[i] = -2;
  }
  queue->Size = 0;
  return( queue );
  //return NULL;
}

double Value(struct StateQueue *queue, int i)
{
  struct QueueNode *queueNode;
  queueNode = queue->queueNode[i];
  return queueNode->Value;
}

int QueueTop(struct StateQueue* queue)
{
  return queue->queueNode[0]->StateNo;
}

double QueueTopValue(struct StateQueue* queue)
{
  return queue->queueNode[0]->Value;
}

void MaxHeapify(struct StateQueue *queue, int i)
{
  int l, r, largest, tempState, tempindex, state;
  double tempValue;
  
  l = Left(i);
  r = Right(i);
  if ((l < queue->Size) && (Value(queue, l) > Value(queue, i))) {
    largest = l;
    state = queue->index[l];
  }
  else {
    largest = i;
    state = queue->index[i];
  }
  //printf("Size: %d Largest value: %f\n", queue->Size, Value(queue, largest));
  if ((r < queue->Size) && (Value(queue, r) > Value(queue, largest))) {
    largest = r;
    state = queue->index[r];
  }
  
  //printf("Size: %d Largest value: %f\n", queue->Size, Value(queue, largest));
  if (largest != i) {
    tempState = queue->queueNode[i]->StateNo;
    queue->queueNode[i]->StateNo = queue->queueNode[largest]->StateNo;
    queue->index[i] = state;
    queue->rindex[state] = i;
    queue->queueNode[largest]->StateNo = tempState;
    queue->index[largest] = queue->queueNode[largest]->StateNo;
    queue->rindex[queue->queueNode[largest]->StateNo] = largest;
    tempValue = queue->queueNode[i]->Value;
    queue->queueNode[i]->Value = queue->queueNode[largest]->Value;
    queue->queueNode[largest]->Value = tempValue;
    MaxHeapify(queue, largest);
  }
}

void IncreaseValue(struct StateQueue *queue, int i, double value)
{
  int tempState, p, state;
  double tempValue;

  if (value < Value(queue, i)) {
    printf("Increase value error! New key is smaller than the current key\n");
    return;
  }
  queue->queueNode[i]->Value = value;
  p = Parent(i);
  while ((i > 0) && (Value(queue, p) < Value(queue, i))) {
    tempState = queue->queueNode[i]->StateNo;
    queue->queueNode[i]->StateNo = queue->queueNode[p]->StateNo;
    queue->queueNode[p]->StateNo = tempState;
    tempValue = queue->queueNode[i]->Value;
    queue->queueNode[i]->Value = queue->queueNode[p]->Value;
    queue->queueNode[p]->Value = tempValue;
    queue->index[i] = queue->queueNode[i]->StateNo;
    queue->index[p] = queue->queueNode[p]->StateNo;
    queue->rindex[queue->queueNode[i]->StateNo] = i;
    queue->rindex[queue->queueNode[p]->StateNo] = p;
    i = p;
    p = Parent(i);
  }
}

void InsertQueue(struct StateQueue* queue, int state, double value)
{
  int size;
  
  size = queue->Size;
  queue->queueNode[size]->StateNo = state;
  queue->queueNode[size]->Value = NEG_INF;
  queue->index[size] = state;
  queue->rindex[state] = size;
  queue->Size++;
  IncreaseValue(queue, size, value);
}

int OutQueue(struct StateQueue* queue)
{
  int max, size;
  
  size = queue->Size;
  if (size == 0) {
    printf("error OutQueue\n");
    return -2;
  }
  max = queue->queueNode[0]->StateNo;  
  queue->queueNode[0]->StateNo = queue->queueNode[size-1]->StateNo;
  queue->queueNode[0]->Value = queue->queueNode[size-1]->Value;
  queue->index[size] = -2;
  queue->rindex[max] = -2;
  queue->Size--;
  MaxHeapify(queue, 0);
  return max;
}

int IsEmpty(struct StateQueue* queue)
{
  if (queue->Size == 0)
    return 1;
  else
    return 0;
}

int InQueue(struct StateQueue *queue, int state)
{
  return queue->rindex[state];
}

double StateValue(struct StateQueue *queue, int state)
{
  int index = InQueue(queue, state);
  if ( index != -2 )
    return queue->queueNode[index]->Value;

  return 0.0;
}

void PrintQueue(struct StateQueue* queue)
{
  struct QueueNode *queueNode;
  int state, i;
  double value;
  
  printf("\n");
  for (i = 0; i < queue->Size; i++) {
    queueNode = queue->queueNode[i];
    state = queueNode->StateNo;
    value = queueNode->Value;
    printf("State: %d Value: %f\n", state, value);
  }
}

struct StateList* CreateList(int number)
{
  struct StateList *list;
  int i;
  
  list = (struct StateList*)malloc(sizeof(struct StateList));
  list->listNode = (struct ListNode**)malloc((unsigned)number*sizeof(struct ListNode*));
  for (i = 0; i < number; i++) {
    list->listNode[i] = (struct ListNode*)malloc(sizeof(struct ListNode));
  }
  list->Capicity = number;
  list->Size = 0;
  list->Head = 0;
  list->Tail = 0;
  return( list );
}

void FreeList(struct StateList *list)
{
  int i, capicity;
  capicity = list->Capicity;
  for (i = 0; i < capicity; i++) {
    free(list->listNode[i]);
  }
  free(list->listNode);
  free(list);
}

int IsEmptyList(struct StateList *list)
{
  if (list->Size == 0)
    return 1;
  else
    return 0;
}

void AppendState(struct StateList *list, int state)
{
  int tail, size;
  
  size = list->Size;
  tail = list->Tail;
  list->listNode[tail]->StateNo = state;
  list->Size++;
  list->Tail++;
  list->Tail %= list->Capicity;
}

int OutList(struct StateList *list)
{
  int top, head;
  
  head = list->Head;
  if (list->Size == 0)
    return -2;  
  top = list->listNode[head]->StateNo;  
  list->Size--; 
  list->Head++; 
  list->Head %= list->Capicity;
  return top;
}

int ListSize(struct StateList *list)
{
  return list->Size;
}

void PrintList(struct StateList *list)
{
  int index, i;
  
  index = list->Head;
  printf("List size: %d Head: %d Tail: %d\n", list->Size, list->Head, list->Tail);
  for (i = 0; i < list->Size; i++) {
    printf("State: %d\n", list->listNode[index]->StateNo);
    index++;
    index %= list->Capicity;
  }
}
