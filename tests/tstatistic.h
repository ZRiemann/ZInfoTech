#include <zit/statistic/statistic.h>

#define ZUSE_STATISTIC 1

#if ZUSE_STATISTIC
zstat_hots_t **g_stat_hots;

#define SRI_GLOBAL 0 // statistic resource index global function
#define SRI_QUE1 1 // statistic resource index que1 functions
#define SRI_SIZE 2

#define SFI_GLOBAL_SIZE 0

#define SFI_QUE1_PUSH 0
#define SFI_QUE1_POP 1
#define SFI_QUE1_SIZE 2

#endif
