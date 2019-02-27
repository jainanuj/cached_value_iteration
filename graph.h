/* graph.h */

/* forward declaration of structures 
struct StateNode;
struct ActionNode;
struct StateDistribution;
struct StateListNode;
struct ActionListNode;*/

/* declaration of structures */
#ifndef graph_h
#define graph_h

struct StateNode {
  int                     StateNo;
    int                   num_actions;          //VERIFY-ANUJ
    int                   comp_state_num;       //VERIFY-ANUJ
    int                   component_id;         //VERIFY-ANUJ
  char*                   Description;
  double                  f;
  /* added for frtdp begin */
  double                  fprime;
  double                  w;
  /* added for frtdp end */
  double                  fWeight;
  double                  g;
  /* added for fdp begin */
  double                  hg;
  /* added for fdp end */
  /* added for IPS begin */
  double                  goalProb; /* prob of reaching the goal */
  /* added for IPS end */
  /* added for BRTDP begin */
  struct ActionNode       *Pi;
  /* added for BRTDP end */
  /* added for VPIRTDP begin */
  double                     VPIheuristic; /* VPI heuristic value */
  /* added for VPIRTDP end */
  double                  h;
  double                  meanFirstPassage; /* upper bound on steps to goal */
  int                     Terminal;
  int                     Expanded;
  int                     Update;
  int                     Visited;
  int                     Converged; /* used for ftvi only */
  struct ActionNode      *BestAction;
  struct ActionNode      *BestPrevAction;
  /* the set of actions that are 1) the best action of some state; 2) reach the current state */
  struct ActionListNode  *BestPrevActions; 
  struct ActionListNode  *Action;
  struct ActionListNode  *PrevAction;
};
/* forward declaration of StateNode functions */
struct StateNode *CreateStateNode( void );

struct ActionNode {
  int                        StateNo; /* used to distinguish different actions */
  int                        ActionNo;
  int                        Visited;
  /* added for FTVI begin */
  int                        Dominated; /* whether it is already suboptimal (=1) */
  /* added for FTVI end */
  /* added for BRTDP begin */
  double                     goalProb; /* prob of reaching the goal */
  double                     w;
//  double                     lambda;
  /* added for BRTDP end */
  char                      *Description;
  double                     q;
  double                     Cost; /* Immediate cost */
  struct StateDistribution  *NextState;
  struct StateNode          *PrevState;
};
/* forward declaration of ActionNode functions */
struct ActionNode *CreateActionNode( void );
void printaction(struct ActionNode* action);

struct StateDistribution {
  struct StateNode          *State;
  double                     Prob;
  struct StateDistribution  *Next;
};
/* forward declaration of StateDistribution functions */
struct StateDistribution *CreateStateDistribution( void );

struct StateListNode {
  struct StateNode      *Node;
  struct StateListNode  *Next;
};
/* forward declaration of StateListNode functions */
struct StateListNode *CreateStateList(void);
void   AppendStateList(struct StateListNode *list, struct StateNode *node);
void   DeleteStateList(struct StateListNode *list);
void   PrintStateList(struct StateListNode *list);
void   DisplayStateList(struct StateListNode *list);

struct ActionListNode {
  struct ActionNode     *Node;
  struct ActionListNode *Next;
};
/* forward declaration of ActionListNode functions */
struct ActionListNode *CreateActionList(void);
void   AppendActionList(struct ActionListNode *list, struct ActionNode *node);
void   RemoveActionList(struct ActionListNode *list, struct ActionNode *node);
void   DeleteActionList(struct ActionListNode *list);
int    IsSameActionNode(struct ActionNode *node1, struct ActionNode *node2);

/* external declaration of global data structures */
extern struct StateNode **StateIndex; 
extern struct StateNode  *Start;
extern struct StateNode  *Goal;
extern long numBackups;

#endif

