/* backup.c */

#include <time.h>
#include "solve.h"
#include "graph.h"
#include "backup.h"
#include "stdio.h"

double Backup(struct StateNode *state)
/* Returns change of state value */
{
  double                    fCost, gCost, hCost,
                            bestfCost, bestgCost = 0.0, besthCost = 0.0, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage, goalProb, bestGoalProb = 0.0,
                            max_value = 9999.99;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode, *bestAction, *oldBestAction, *oldAction;
  struct StateDistribution *nextState;

  //if (state == Goal)
  if ( (state->Terminal == 1) || (state->Terminal == 5) )
    return 0;

  /* used for pathmax */
  oldfWeightCost = state->fWeight;
  oldfCost = state->f;
  oldBestAction = state->BestAction;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfWeightCost = max_value;
  bestfCost = max_value;
  bestAction = NULL;

  /* Find action that minimizes expected cost */
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    gCost = actionNode->Cost;
    hCost = 0.0;
    goalProb = 0.0;
    //fprimeCost = 0.0;
    for (nextState = actionNode->NextState;
         nextState;
         nextState = nextState->Next) {
      gCost += gDiscount * nextState->Prob * nextState->State->g;
/*
      if (nextState->State->h >= max_value) {
        hCost = max_value;
        break;
      }
      else
*/
        hCost += gDiscount * nextState->Prob * nextState->State->h;
      goalProb = gDiscount * nextState->Prob * nextState->State->goalProb;
    }
    fCost = gCost + hCost;
    fWeightCost = gCost + (gWeight * hCost);
    if (fWeightCost < bestfWeightCost) { /* or (fCost < bestfCost) */
      bestfWeightCost = fWeightCost;
      bestfCost = fCost;
      bestgCost = gCost;
      besthCost = hCost;      
      bestAction = actionNode;
      bestGoalProb = goalProb;
      for (nextState = actionNode->NextState;
           nextState;
           nextState = nextState->Next) {
         nextState->State->BestPrevAction = bestAction;
      }
    }
  }
  
  if (bestAction) {
    state->fWeight = bestfWeightCost;
    state->f = bestfCost;
    state->g = bestgCost;
    state->h = besthCost;
    state->goalProb = bestGoalProb;
    state->BestAction = bestAction;
    
    /* now update the BestPrevActions of the successor states */
    if ((bestAction && oldBestAction) && (bestAction->ActionNo != oldBestAction->ActionNo)) {
      //for (nextState = actionNode->NextState;
      for (nextState = bestAction->NextState;
           nextState;
           nextState = nextState->Next) {
        bestActionListNode = nextState->State->BestPrevActions;
        if (nextState->State && bestActionListNode) {
          //RemoveActionList(bestActionListNode, oldBestAction);
          AppendActionList(bestActionListNode, bestAction);
	}
      }
    }
  }

  /* Update estimate of mean first passage time */
/*
  oldMeanFirstPassage = state->meanFirstPassage;
  if ((state->Terminal != 1) && (state->Terminal != 5)) {
    state->meanFirstPassage = state->BestAction->Cost;
    for (nextState = state->BestAction->NextState;
         nextState;
         nextState = nextState->Next) {
      state->meanFirstPassage += 
        nextState->Prob * nextState->State->meanFirstPassage;
        nextState->State->BestPrevAction = bestAction;
    }
  }
*/

  numBackups++;
  double residual = bestfWeightCost - oldfWeightCost;
  if (residual < 0) {
    residual = -residual;
  }

  //printf("\nstate %d: old value: %f new value: %f residual %f", state->StateNo, oldfWeightCost, bestfWeightCost, residual);
  return residual; /* or (bestfCost - oldfCost) */
}

double BackupTwo(struct StateNode *state)
/* Returns change of state value */
{
  double                    fCost, gCost, hCost, fprimeCost,
                            bestfCost, bestgCost = 0.0, besthCost = 0.0, bestfprimeCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage, goalProb, bestGoalProb = 0.0, residual,
                            max_value = 9999.99, epsilon = 1e-6, fWeightCost2 = 0;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode, *bestAction, *oldBestAction, *oldAction;
  struct StateDistribution *nextState;
  int                       undominated = 0;

  if (state == Goal)
//  if ( (state == Goal) || (state->Converged) )
    return 0;

  /* used for pathmax */
  oldfWeightCost = state->fWeight;
    fWeightCost2 = max_value;
  oldfCost = state->f;
  oldBestAction = state->BestAction;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfWeightCost = max_value;
  bestfprimeCost = max_value;
  bestfCost = max_value;
  bestAction = NULL;

  /* Find action that minimizes expected cost */
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    if (actionNode->Dominated == 1)
      continue;
    undominated++;
    gCost = actionNode->Cost;
    hCost = 0.0;
    goalProb = 0.0;
    fprimeCost = actionNode->Cost;
      fWeightCost2 = 0;
    for (nextState = actionNode->NextState;
         nextState;
         nextState = nextState->Next)
    {
      gCost = gDiscount * nextState->Prob * nextState->State->g;
/*      if (nextState->State->h >= max_value) {
        hCost = max_value;
        fprimeCost = max_value;
        break;
      }
      else {*/
        
        fWeightCost2 += nextState->Prob * nextState->State->fWeight;
        
        hCost += gDiscount * nextState->Prob * nextState->State->h;
        fprimeCost += gDiscount * nextState->Prob * nextState->State->fprime;
//      }
      goalProb = gDiscount * nextState->Prob * nextState->State->goalProb;
    }
    fCost = gCost + hCost;
    fWeightCost = gCost + (gWeight * hCost);
      fWeightCost2 = fWeightCost2 + actionNode->Cost;
    if (fWeightCost > max_value)
      fWeightCost = max_value;
    
    if (fWeightCost2 > max_value)
        fWeightCost2 = max_value;

    if (fWeightCost > state->fprime + epsilon) {
      actionNode->Dominated = 1;                        //Eliminating non-optimal edges - Anuj
    }
    if (fWeightCost2 < bestfWeightCost)
    { /* or (fCost < bestfCost) */
      bestfWeightCost = fWeightCost2;
      bestfCost = fCost;
      bestgCost = gCost;
      besthCost = hCost;      
      bestAction = actionNode;
      bestGoalProb = goalProb;
      for (nextState = actionNode->NextState;
           nextState;
           nextState = nextState->Next)
      {
         nextState->State->BestPrevAction = bestAction;
      }
    }
    if (fprimeCost < bestfprimeCost) {
      bestfprimeCost = fprimeCost;
    }
  }     //End of all actions

  state->fprime = bestfprimeCost;
  
  if (bestAction) {
    state->fWeight = bestfWeightCost;
    state->f = bestfCost;
    state->g = bestgCost;
    state->h = besthCost;
    state->goalProb = bestGoalProb;
    state->BestAction = bestAction;
    
    /* now update the BestPrevActions of the successor states */

    if ((bestAction && oldBestAction) && (bestAction->ActionNo != oldBestAction->ActionNo)) {
      for (nextState = bestAction->NextState;
           nextState;
           nextState = nextState->Next) {
        bestActionListNode = nextState->State->BestPrevActions;
        if (nextState->State && bestActionListNode) {
          AppendActionList(bestActionListNode, bestAction);
	}
      }
    }
  }

  /* Update estimate of mean first passage time */
/*
  oldMeanFirstPassage = state->meanFirstPassage;
  if ((state->Terminal != 1) && (state->Terminal != 5)) {
    state->meanFirstPassage = state->BestAction->Cost;
    for (nextState = state->BestAction->NextState;
         nextState;
         nextState = nextState->Next) {
      state->meanFirstPassage += 
        nextState->Prob * nextState->State->meanFirstPassage;
        nextState->State->BestPrevAction = bestAction;
    }
  }
*/

  residual = bestfprimeCost - bestfWeightCost;
//  if ( (residual < epsilon) && (undominated == 1) )
//    state->Converged = 1;

  numBackups++;
  if (state->StateNo == gInitialState)
    BackupTwo(Start);

  residual = bestfWeightCost - oldfWeightCost;
  if (residual < 0) {
    residual = -residual;
  }

//  printf("\nstate %d: old value: %f new value: %f", state->StateNo, oldfWeightCost, bestfWeightCost);
  return residual; /* or (bestfCost - oldfCost) */
}

double BackupTwo_conv(struct StateNode *state)
/* Returns change of state value */
{
    double    bestfWeightCost, oldfWeightCost,
    residual, max_value = 9999.99, value = 0;
    struct ActionListNode    *actionListNode, *bestActionListNode;
    struct ActionNode        *actionNode, *bestAction, *oldBestAction;
    struct StateDistribution *nextState;
    int                       undominated = 0;
    
    if (state == Goal)
        //  if ( (state == Goal) || (state->Converged) )
        return 0;
    
    /* used for pathmax */
    oldfWeightCost = state->fWeight;
    value = 0;
    oldBestAction = state->BestAction;
    
    /* Initialize to worst possible cost to guarantee improvement */
    bestfWeightCost = max_value;            //max_value (for minimization)
    bestAction = NULL;
    
    /* Find action that maximises expected cost */  //minimises
    for (actionListNode = state->Action;
         actionListNode;
         actionListNode = actionListNode->Next)
    {
        actionNode = actionListNode->Node;
        if (actionNode->Dominated == 1)
            continue;
        undominated++;
        value = 0;

        for (nextState = actionNode->NextState;
             nextState;
             nextState = nextState->Next)
        {
            value += nextState->Prob * nextState->State->fWeight;
            
        }
        value = value + actionNode->Cost;
        
        if (value > max_value)          // >max_value for minimization
            value = max_value;          // =max_value for minim.
        
        if (value < bestfWeightCost)        // < bestfWeightCost for minim
        { /* or (fCost < bestfCost) */
            bestfWeightCost = value;
            bestAction = actionNode;
            for (nextState = actionNode->NextState;
                 nextState;
                 nextState = nextState->Next)
            {
                nextState->State->BestPrevAction = bestAction;
            }
        }
    }     //End of all actions
    if (bestfWeightCost != max_value)           // !=max_value for minim.
        state->fWeight = bestfWeightCost;
    if (bestAction) {
        state->BestAction = bestAction;
        
        /* now update the BestPrevActions of the successor states */
        
        if ((bestAction && oldBestAction) && (bestAction->ActionNo != oldBestAction->ActionNo)) {
            for (nextState = bestAction->NextState;
                 nextState;
                 nextState = nextState->Next) {
                bestActionListNode = nextState->State->BestPrevActions;
                if (nextState->State && bestActionListNode) {
                    AppendActionList(bestActionListNode, bestAction);
                }
            }
        }
    }
    
    numBackups++;
//    if (state->StateNo == gInitialState)
//        BackupTwo(Start);
    
    residual = bestfWeightCost - oldfWeightCost;
    if (residual < 0) {
        residual = -residual;
    }
    
    //  printf("\nstate %d: old value: %f new value: %f", state->StateNo, oldfWeightCost, bestfWeightCost);
    return residual; /* or (bestfCost - oldfCost) */
}

double BackupQ(struct ActionNode* action)
{
  double                    oldqCost, qCost, bestqCost, nextqCost;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode, *bestAction = NULL, *oldBestAction, *oldAction;
  struct StateDistribution *nextState, *state;

  //printf("\nQ-backing up node: %d action %d", action->StateNo, action->ActionNo);
  if (action->PrevState == Goal)
    return 0;
  oldqCost = action->q;
  qCost = action->Cost;

  for (nextState = action->NextState;
       nextState;
       nextState = nextState->Next) {
    /* find the minimum Q-value of the successor state "nextState" */
    if (nextState->State == Goal)
      continue;
    bestqCost = 9999999999.9;

    /* Find action that minimizes expected cost */
    for (actionListNode = nextState->State->Action;
         actionListNode;
         actionListNode = actionListNode->Next) {
      actionNode = actionListNode->Node;
      nextqCost = actionNode->q;
      if (nextqCost < bestqCost) {
        bestqCost = nextqCost;
        bestAction = actionNode;
      }
    }
    nextState->State->BestAction = bestAction;
    qCost += gDiscount * nextState->Prob * bestqCost;
  } 
  action->q = (1.0 - gLearningRate) * oldqCost + gLearningRate * qCost;
  action->Visited++;
  if (action->PrevState->Visited < action->Visited)
    action->PrevState->Visited = action->Visited;
  //printf("\nQ: %f", action->q);
  
  return(action->q - oldqCost);
}

double QValue(struct ActionNode* action)
{
  double                    oldqCost, qCost, bestqCost, nextqCost;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode, *bestAction, *oldBestAction, *oldAction;
  struct StateDistribution *nextState, *state;

  if (action->PrevState == Goal)
    return 0;
  qCost = action->Cost;

  for (nextState = action->NextState;
       nextState;
       nextState = nextState->Next) {
    if (nextState->State == Goal)
      continue;
    qCost += gDiscount * nextState->Prob * nextState->State->fWeight;
  } 
  action->q = qCost;
  
  return action->q;
}

struct ActionNode* BestAction(struct StateNode *state)
{
  double                    fCost, bestfCost, fWeightCost, bestfWeightCost;
  struct ActionListNode    *actionListNode;
  struct ActionNode        *actionNode, *bestAction = NULL;

  if (state == Goal)
    return 0;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfWeightCost = 9999999999.9;
  bestfCost = 9999999999.9;

  /* Find action that minimizes expected cost */
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    fCost = actionNode->q;
    fWeightCost = actionNode->q;
    if (fWeightCost < bestfWeightCost) { /* or (fCost < bestfCost) */
      bestfWeightCost = fWeightCost;
      bestfCost = fCost;
      bestAction = actionNode;
    }
  }
  state->BestAction = bestAction;
  return bestAction;
}

double BackupusingQ(struct StateNode *state)
/* Returns change of state value */
{
  double                    fCost, gCost, hCost, fprimeCost,
                            bestfCost, bestgCost, besthCost, bestfprimeCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode = NULL, *bestAction = NULL, *oldBestAction, *oldAction;
  struct StateDistribution *nextState;

  //printf("\nBacking up node: %d\t", state->StateNo);
  if (state == Goal)
    return 0;
  /* used for pathmax */
  oldfWeightCost = state->fWeight;
  oldfCost = state->f;
  oldBestAction = state->BestAction;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfWeightCost = 9999999999.9;
  bestfCost = 9999999999.9;

  /* Find action that minimizes expected cost */
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    fCost = actionNode->q;
    fWeightCost = actionNode->q;
    if (fWeightCost < bestfWeightCost) { /* or (fCost < bestfCost) */
      bestfWeightCost = fWeightCost;
      bestfCost = fCost;
      bestAction = actionNode;
      for (nextState = actionNode->NextState;
           nextState;
           nextState = nextState->Next) {
         nextState->State->BestPrevAction = bestAction;
      }
    }
  }
  
    state->fWeight = bestfWeightCost;
    state->f = bestfCost;
    state->BestAction = bestAction;
    
    /* now update the BestPrevActions of the successor states */
    if ((bestAction && oldBestAction) && (bestAction->ActionNo != oldBestAction->ActionNo)) {
      for (nextState = actionNode->NextState;
           nextState;
           nextState = nextState->Next) {
        bestActionListNode = nextState->State->BestPrevActions;
        if (bestActionListNode) {
          //RemoveActionList(bestActionListNode, oldBestAction);
          AppendActionList(bestActionListNode, bestAction);
        }
      }
    }
  
  oldMeanFirstPassage = state->meanFirstPassage;
  if (state->Terminal != 1) { /* update every state except goal */
    state->meanFirstPassage = state->BestAction->Cost;
    for (nextState = state->BestAction->NextState;
         nextState;
         nextState = nextState->Next) {
      state->meanFirstPassage += 
        nextState->Prob * nextState->State->meanFirstPassage;
        nextState->State->BestPrevAction = bestAction;
    }
  }
  
  numBackups++;
  return(bestfWeightCost - oldfWeightCost); /* or (bestfCost - oldfCost) */
}

double FBackup(struct StateNode *state)
/* Returns change of state value */
{
  double                    fCost, gCost, hCost, fprimeCost, hgCost, besthgCost,
                            bestfCost, bestgCost = 0.0, besthCost = 0.0, bestfprimeCost, oldfCost,
                            fWeightCost, bestfWeightCost, oldfWeightCost,
                            oldMeanFirstPassage;
  struct ActionListNode    *actionListNode, *bestActionListNode;
  struct ActionNode        *actionNode, *bestAction = NULL, *oldBestAction, *oldAction;
  struct StateDistribution *nextState;

  if (state == Goal)
    return 0.0;
  /* used for pathmax */
  oldfCost = state->f;
  oldBestAction = state->BestAction;

  /* Initialize to worst possible cost to guarantee improvement */
  bestfCost = 9999999999.9;
  besthgCost = 9999999999.9;

  /* Find action that minimizes expected cost */
  for (actionListNode = state->Action;
       actionListNode;
       actionListNode = actionListNode->Next) {
    actionNode = actionListNode->Node;
    gCost = actionNode->Cost;
    hCost = 0.0;
    fCost = actionNode->Cost;
    hgCost = actionNode->Cost;
    for (nextState = actionNode->NextState;
         nextState;
         nextState = nextState->Next) {
      gCost += gDiscount * nextState->Prob * nextState->State->g;
      hCost += gDiscount * nextState->Prob * nextState->State->h;
      hgCost += gDiscount * nextState->Prob * nextState->State->g;
      fCost += gDiscount * nextState->Prob * nextState->State->f;
    }
    //fCost = gCost + hCost;
    if (fCost < bestfCost) { /* or (fCost < bestfCost) */
      bestfCost = fCost;
      bestgCost = gCost;
      besthCost = hCost;      
      bestAction = actionNode;
      for (nextState = actionNode->NextState;
           nextState;
           nextState = nextState->Next) {
        nextState->State->BestPrevAction = bestAction;
      }
    }
    if (hgCost < besthgCost)
      besthgCost = hgCost;
  }
  
  state->f = bestfCost;
  state->g = bestgCost;
  state->h = besthCost;
  if (besthgCost < bestfCost)
    state->hg = besthgCost;
  else
    state->hg = bestfCost;
  state->BestAction = bestAction;
    
  /* now update the BestPrevActions of the successor states */
  if ((bestAction && oldBestAction) && (bestAction->ActionNo != oldBestAction->ActionNo)) {
    for (nextState = bestAction->NextState;
         nextState;
         nextState = nextState->Next) {
      bestActionListNode = nextState->State->BestPrevActions;
      if (nextState->State && bestActionListNode) {
        //RemoveActionList(bestActionListNode, oldBestAction);
        AppendActionList(bestActionListNode, bestAction);
      }
    }
  }
  
  /* updated hg value, needed by fdp algorithm */
  numBackups++;
  return(oldfCost - bestfCost); /* or (bestfCost - oldfCost) */
}

