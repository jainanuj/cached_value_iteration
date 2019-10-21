/* solve.c 

The command line arguments are as follows:

    -p <filename>          # input file (gInputFileName)
    -discount %f           # discount factor (gDiscount)
    -method %s             # solution method (gMethod)
    -epsilon %f            # convergence test (gEpsilon)
    -weight %f             # weighted heuristic (gWeight)
    -v                     # Turns on verbose mode (gVerbose)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "solve.h"
#include "graph.h"
#include "lao.h"
#include "rlao.h"
#include "blao.h"
#include "vi.h"
#include "top.h"
#include "hdp.h"
#include "rtdp.h"
#include "lrtdp.h"
#include "frtdp.h"
#include "brtdp.h"
#include "vpirtdp.h"
#include "nlao.h"
#include "pvi.h"
#include "qlearning.h"
#include "ps.h"
#include "tql.h"
#include "fdp.h"
#include "ips.h"
#include "components.h"
#include "setgoal.h"

#include "track.h"
#include "pi.h"
#include "blao_o.h"
#include "atvi.h"
#include "ftvi.h"
#include "rmax.h"
#include "trmax.h"
#include "unistd.h"


/* Globals with default values */
char        gInputFileName[80] = "big.dat";
MethodType  gMethod =            lao;
double      gDiscountOverride =  -1.0;
//double      gEpsilon =           0.000000;
double      gEpsilon =           0.000001;
double      gWeight =            1.0;
double      gRTProb =            0.9;
double      gHDiscount =         1.0;
int         gThread =            2;
int         gPre =               0;
int         gInitialState =      4498;     // 4498; //1161278; ;  //2;      //1161278 - 4 mill statescar. 483679
int         gQuitFlag =          0;
int         gFTVIVersion =       1;     //Changed by Anuj from 0
int         gGoalNum =           0;
int         gGoalLayer =         0;
int         gCutoff =            300;
int         gElimination =       0;
double      gLearningRate =      0.1;
int         gLTrials =           10; //10//100;       //Changed by Anuj from 100
int         gUTrials =           100;//100//1000;       //Changed by Anuj from 10000
MyBoolean   gVerbose =           false;
LaoBackupMethod gBackupMethod =  combine;
//LaoBackupMethod gBackupMethod =  separate;

/* Other globals */
int         gNumStates;
int         gNumActions;
double      gDiscount;
double      gError;
clock_t     gStartTime;
time_t      gStartT;
struct timeval gInitialT;
static void GetParamString(char *string, int argc, char **argv, char *flag)
/* Returns the string in argv that follows the flag string.
   (i.e., it processes the command line arguments and returns
   the string that corresponds to the argument after a flag. */
{
   int i;
   string[0] = '\0';
   
   for(i = 0; i < argc; i++) {
      if (strcmp(flag, argv[i]) == 0) {
         strcpy(string, argv[i+1]);
         return;
      } /* if */
   }
}

static void ProcessCommandLine( int argc, char **argv ) {
   char str[80];
   int i;

   /* Get name of input file */
   GetParamString(str, argc, argv, "-p");
   if( str[0] != '\0' )
     strcpy(gInputFileName, str);

   /* Set discount factor */
   /* Will this be overwritten when input file is read? */
   GetParamString( str, argc, argv, "-discount" );
   if( str[0] != '\0' )
      gDiscountOverride = strtod( str, NULL );

   /* Set which algorithm to use */
   GetParamString(str, argc, argv, "-method");
   if( str[0] != '\0' ) 
      for( i = 0; i < NUM_METHODS; i++)
         if( strcmp( str, method_str[ (MethodType) i ]) == 0 )
            gMethod = (MethodType) i;

   /* Set number of threads */
   GetParamString(str, argc, argv, "-thread");
   if( str[0] != '\0' ) 
      gThread = atoi(str);

   GetParamString(str, argc, argv, "-pre");
   if( str[0] != '\0' ) 
      gPre = atoi(str);

   /* Set initial state number */
   GetParamString(str, argc, argv, "-initial");
   if( str[0] != '\0' ) 
      gInitialState = atoi(str);

   /* Set layer number */
   GetParamString(str, argc, argv, "-layer");
   if( str[0] != '\0' ) 
      gGoalLayer = atoi(str);

   /* Set whether to create heuristics */
   GetParamString(str, argc, argv, "-create");
   if( str[0] != '\0' ) 
      gGoalNum = atoi(str);

   /* Set version number */
   GetParamString(str, argc, argv, "-version");
   if( str[0] != '\0' ) 
      gFTVIVersion = atoi(str);

   /* Set lower trial number for FTVI */
   GetParamString(str, argc, argv, "-lt");
   if( str[0] != '\0' ) 
      gLTrials = atoi(str);

   /* Set upper trial number for FTVI */
   GetParamString(str, argc, argv, "-ut");
   if( str[0] != '\0' ) 
      gUTrials = atoi(str);

   /* Set backup method for LAO* */
   GetParamString(str, argc, argv, "-backup");
   if( str[0] != '\0' ) 
      for( i = 0; i < NUM_BACKUP_METHODS; i++)
         if( strcmp( str, backup_str[ (LaoBackupMethod) i ]) == 0 )
            gBackupMethod = (LaoBackupMethod) i;

   /* Set desired epsilon-optimality of solution */
   GetParamString( str, argc, argv, "-epsilon" );
   if( str[0] != '\0' )
     gEpsilon = strtod( str, NULL );

   /* Set transition probability of racetrack */
   GetParamString( str, argc, argv, "-prob" );
   if( str[0] != '\0' )
     gRTProb = strtod( str, NULL );

   /* Set weighted heuristic */
   GetParamString( str, argc, argv, "-weight" );
   if( str[0] != '\0' )
     gWeight = strtod( str, NULL );

   /* Set heuristic discount */
   GetParamString( str, argc, argv, "-hdiscount" );
   if( str[0] != '\0' )
     gHDiscount = strtod( str, NULL );

   /* Set verbose mode if specified */
   for(i = 0; i < argc; i++) 
      if (strcmp("-v", argv[i]) == 0) 
         gVerbose = true;
}

static void DisplayHeader( void )
{
  printf("\n***************************************************************");
  printf("\nSolve %s with %d states, %d actions and discount of %f", 
	 gInputFileName, gNumStates, gNumActions, gDiscount);
  printf("\nUsing %s with epsilon %f",
	 method_str[gMethod], gEpsilon);
  printf("\nUsing transition prob %f",
	 gRTProb);
  printf("\nUsing heuristic discount %f",
	 gHDiscount);
  if (gMethod == lao) {
    if(gBackupMethod == combine)
      printf(", combined backup and expand");
    else 
      printf(", separate backup and expand");
  }
  if ( gWeight != 1.0 )
    printf(", and heuristic with weight %f", gWeight);
  printf("\n***************************************************************");
}

int main(int argc, char **argv)
{

const rlim_t kStackSize = 64L * 1024L * 1024L;   // min stack size = 64 Mb
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
            else
                fprintf(stderr, "setrlimit set the stack size to desired - 8MB");
        }
        else
              fprintf(stderr, "Stack size already >= desired 8 MB");
    }
    else
        fprintf(stderr, "Unable to fetch stack size");

  char* suffix;
  //sleep(10);

  ProcessCommandLine( argc, argv);

  // used to generate goals at the search depth gGoalLayer only
/*
  if (gGoalNum == 0) {
    readRanMDP();
    setgoal(100, gGoalLayer);
    return;
  }
*/

  suffix = strchr(gInputFileName, '.');
  if (strcmp(suffix, ".dat") == 0) {
    readRacetrackMDP();
  }
  else if (strcmp(suffix, ".mdp") == 0) {
    readMDP();
  }
  else if (strcmp(suffix, ".wet") == 0) {
    readWetfloorMDP();
  }
  else {
      readMDP();
//    readRanMDP();
//      readRanMDPGoals();//(gGoalNum);
  }
    
  if (gDiscountOverride > 0.0) 
    gDiscount = gDiscountOverride;


  double largest_diff = CreateHeuristic();

/*
  if (gHDiscount >= 0.0) {
    if (gHDiscount == 0.0)
      dump_optimal_values();
    else
      largest_diff = CreateDiscountedOptimalHeuristics();
  }
*/

//  initMeanFirstPassage();
  DisplayHeader();
  if (gQuitFlag) {
    printf("\nProblem unsolvable\n");
    return 1;
  }
//  printf("\nShortest distance to goal %f", StateIndex[gInitialState]->f);
  printf("\nShortest distance to goal %d", (int)StateIndex[gInitialState]->f);
  numBackups = 0;
  gStartTime = clock();
  time(&gStartT);
  gettimeofday(&gInitialT, NULL);
  switch( gMethod ) {

  case pi:
    //PolicyIteration (CreateAllStateListWithoutGoal());
    PolicyIteration (CreateAllStateList());
    printf("\nConverged! Value of start state %f\n", Start->f);
    break;

  case vi:
    ValueIteration (CreateAllStateListWithoutGoal(), 200000);
    break;

  case vie:
    gElimination = 1;
    ValueIteration (CreateAllStateListWithoutGoal(), 200000);
    break;

  case rtdp:
    RTDP(25000); /* the parameter to RTDP is the number of trials */
    break;

  case lrtdp:
    LRTDP(0, gEpsilon); /* the parameter to LRTDP is the initial state and epsilon */
    break;

  case frtdp:
    FRTDP(10, gEpsilon); /* the parameter to LRTDP is the initial depth and epsilon */
    break;

  case brtdp:
    BRTDP(1000000);
    break;
    
  case vpirtdp:
    VPIRTDP(1000000, largest_diff);
    break;
    
  case lao:
    LAOstar();
    break;

  case rlao:
    rLAOstar();
    break;

  case blao:
    //bLAOstar();
    break;

  case nlao:
    nLAOstar(gThread);
    break;

  case pvi:
    PVI();
    break;

  case npvi:
    NPVI();
    break;

  case nbpvi:
    NPVI();
    break;

  case bpvi:
    BPVI();
    break;

  case mbpvi:
    MBPVI();
    break;

  case lpvi:
    //LPVI();
    PVI();
    break;

  case lbpvi:
    LBPVI();
    break;

  case blao_o:
    bLAOOstar();
    break;
    
  case tvi:
    TVI();
    break;

  case atvi:
    ATVI();
    break;
    
  case ftvi:
    FTVI(gFTVIVersion);
    break;
    
  case hdp:
    HDP(78, gEpsilon);
    break;
  
  case qlearning:
    QLearning(500000);
    break;
  
  case pris:
    PS(0);
    break;
    
  case ppris:
    PS(1);
    break;
    
  case fdp:
    FDP(0);
    break;
    
  case ffdp:
    FDP(1);
    break;
    
  case ips:
    IPS();
    break;
    
  case tql:
    TQL(500000);
    break;
    
  case rmax:
    RMax();
    break;
    
  case trmax:
    TRMax();
    break;
    
  case trmax_o:
    TRMax_O();
    break;
    
  default:
    fprintf( stderr, "Unrecognized solving method. Aborting.\n");
    exit( -1 );
  }
  printf("Total number of backups: %ld", numBackups);
/*
  generate_components();

  if ((gHDiscount == 1.0) && (gMethod == ftvi))
    printf("\nGlobal optimality of initial heuristics: %f", optimality());
*/

  printf("\n");
}
