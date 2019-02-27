/* backup.h */

double Backup(struct StateNode *state);
double BackupTwo(struct StateNode *state);
double BackupQ(struct ActionNode *action);
double QValue(struct ActionNode *action);
struct ActionNode* BestAction(struct StateNode *state);
double BackupusingQ(struct StateNode *state);
double FBackup(struct StateNode *state);
double BackupTwo_conv(struct StateNode *state);
