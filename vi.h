/* vi.h */
#ifndef __VI__
#define __VI__

struct StateListNode *CreateAllStateList(void);
struct StateListNode *CreateAllStateListWithoutGoal(void);
void ValueIteration (struct StateListNode *list, int MaxIter);

#endif
