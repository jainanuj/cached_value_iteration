/* stack.h */

struct intStack
{
  int                Number;
  struct intStack*   Next;            
}; 

struct intStack *CreateStack( void );
void deletestack(struct intStack *stack);
int top(struct intStack* stack);
void push(struct intStack* stack, int num);
int pop(struct intStack *stack);
int empty(struct intStack* stack);
int instack(struct intStack* stack, int num);
void printstack(struct intStack* stack);
