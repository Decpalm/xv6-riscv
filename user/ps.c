#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"
#include "kernel/param.h"

// Prints n spaces
void
printspaces(int n)
{
    for(int i = 0; i < n; i++) {
        printf(" ");
    }
}

// Returns the length of a number
int
numlen(int n)
{
    if(n == 0) return 1;
    int len = 0l;
    if(n < 0) {
        len = 1;
        n = -n;
    }
    while(n > 0) {
        len++;
        n /= 10;
    }
    return len;
}

int
main(int argc, char *argv[])
{
    // Array used to decode the state enum
    static const char *statelookup[] = { "UNUSED", "USED", "SLEEPING", "RUNNABLE", "RUNNING", "ZOMBIE" };

    int max_procs;
    struct procinfo pinfo[NPROC];

    // sets max_procs to either the first command line input, or NPROC if there are none
    if(argc > 1) {
        max_procs = atoi(argv[1]);
    }
    else {
        max_procs = NPROC;
    }

    // Using the system cal
    int procs;
    procs = getprocs(pinfo, max_procs);

    if(procs < 0){
        printf("getprocs error\n");
    }
    else {
        printf("PID    PPID   STATE      SIZE       NAME\n");
        for (int i = 0; i < procs; i++) {
            // printspaces() is used to ensure fixed width columns
            printf("%d", pinfo[i].pid);
            printspaces(7 - numlen(pinfo[i].pid));

            printf("%d", pinfo[i].ppid);
            printspaces(7 - numlen(pinfo[i].ppid));

            printf("%s", statelookup[pinfo[i].state]);
            printspaces(11 - strlen(statelookup[pinfo[i].state]));

            printf("%lu", pinfo[i].sz);
            printspaces(11 - numlen(pinfo[i].sz));

            printf("%s\n", pinfo[i].name);

        }
    }
}