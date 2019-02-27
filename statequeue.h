/* statequeue.h */

#define NEG_INF -100

struct QueueNode {
  int StateNo;
  double Value;
};

struct StateQueue {
  int Size;
  struct QueueNode **queueNode;
  int *index; // the state of each position
  int *rindex;  // the position of each state        
};

struct ListNode {
  int StateNo;
};

struct StateList {
  int Capicity;
  int Size;
  int Head;
  int Tail;
  struct ListNode **listNode;
};

/* functions for state queue */
int Left( int );
int Right( int );
int Parent( int );
double Value(struct StateQueue *queue, int i);
struct StateQueue* CreateQueue( int );
int QueueTop(struct StateQueue *queue);
double QueueTopValue(struct StateQueue *queue);
void IncreaseValue(struct StateQueue *queue, int i, double value);
void InsertQueue(struct StateQueue *queue, int state, double change);
//void removequeue(struct StateQueue* queue, int state);
void MaxHeapify(struct StateQueue *queue, int i);
int OutQueue(struct StateQueue *queue);
int IsEmpty(struct StateQueue *queue);
int InQueue(struct StateQueue *queue, int state);
void PrintQueue(struct StateQueue *queue);
double StateValue(struct StateQueue *queue, int state);

/* functions for state list */
struct StateList* CreateList( int );
int IsEmptyList(struct StateList *list);
void AppendState(struct StateList *list, int state);
int OutList(struct StateList *list);
int ListSize(struct StateList *list);
void PrintList(struct StateList *list);
void FreeList(struct StateList *list);

