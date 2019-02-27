/* track.h */
#ifndef __TRACK__
#define __TRACK__


extern int  gNumStates;
extern double  gInitValueSum;

extern void readRacetrackMDP (void);
extern void readRanMDP (void);
extern void readMDP (void);

extern void readWetfloorMDP (void);
extern void readRanMDPGoals(void);

extern void initMeanFirstPassage (void);
extern void initHeuristic (void);
double CreateHeuristic(void);

#endif
