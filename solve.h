/* solve.h */

/* A boolean type will be useful */
typedef enum { false, true } MyBoolean;
//enum MyBoolean { false, true };

/* Change this as more algorithms are added */
#define NUM_METHODS  34
static char *method_str[] = {
   "pi", "vi", "vie", "rtdp", "lao", "rlao", "blao", "nlao", "pvi", "npvi", "nbpvi", "lpvi", "bpvi", "lbpvi", "mbpvi", "blao_o", "tvi", "lrtdp", "hdp", "frtdp", "qlearning", "pris", "ppris", "tql", "rmax", "trmax", "trmax_o", "fdp", "ffdp", "ips", "atvi", "ftvi", "brtdp", "vpirtdp"
};
typedef enum {pi, vi, vie, rtdp, lao, rlao, blao, nlao, pvi, npvi, nbpvi, lpvi, bpvi, lbpvi, mbpvi, blao_o, tvi, lrtdp, hdp, frtdp, qlearning, pris, ppris, tql, rmax, trmax, trmax_o, fdp, ffdp, ips, atvi, ftvi, brtdp, vpirtdp} MethodType;

#define NUM_BACKUP_METHODS 2
static char *backup_str[] = {
  "combine", "separate"
};
typedef enum {combine, separate} LaoBackupMethod;

extern char        gInputFileName[];
extern MethodType  gMethod;
extern double      gDiscount;
extern double      gEpsilon;
extern double      gWeight;
extern double      gLearningRate;
extern MyBoolean   gVerbose;
extern int         gNumStates;
extern int         gNumActions;
extern int         gPre;
extern int         gInitialState;
extern int         gQuitFlag;
extern int         gFTVIVersion;
extern int         gGoalNum;
extern int         gGoalLayer;
extern int         gCutoff;
extern int         gElimination;
extern double      gRTProb;
extern double      gHDiscount;
extern clock_t     gStartTime;
extern time_t      gStartT;
extern int         gLTrials;
extern int         gUTrials;
extern struct timeval gInitialT;
extern LaoBackupMethod gBackupMethod;
