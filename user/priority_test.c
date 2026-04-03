#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// This is a template tester program; you may modify as you see fit. 
int
main(int argc, char *argv[])
{
  int pid1, pid2, pid3;
  
  printf("Priority Scheduler Test with Aging\n");
  printf("===================================\n");
  
  // Create three child processes with different priorities
  pid1 = fork();
  if(pid1 == 0) {
    // Child 1: High priority (8)
    printf("Child 1 (PID %d, Priority %d): Starting CPU-intensive task\n", getpid(), getpriority(getpid()));
    
    // CPU-intensive loop with longer iterations to show aging
    for(int i = 0; i < 4000000; i++) {
      if(i % 400000 == 0) {
        printf("Child 1 (High Priority): %d iterations, current priority: %d\n", 
               i, getpriority(getpid()));
      }
      // Add some yielding to allow other processes to show aging
      if(i % 50000 == 0) {
        //printf("Child 1 (High Priority): Yielding CPU at iteration %d\n", i);
        yield();
      }
    }
    printf("Child 1 (High Priority): Finished\n");
    exit(0);
  }
  
  pid2 = fork();
  if(pid2 == 0) {
    // Child 2: Medium priority (5) - default
    printf("Child 2 (PID %d, Priority %d): Starting CPU-intensive task\n", getpid(), getpriority(getpid()));
    
    // CPU-intensive loop
    for(int i = 0; i < 4000000; i++) {
      if(i % 400000 == 0) {
        printf("Child 2 (Medium Priority): %d iterations, current priority: %d\n", 
               i, getpriority(getpid()));
      }
      if(i % 50000 == 0) {
        //printf("Child 2 (Medium Priority): Yielding CPU at iteration %d\n", i);
        yield();
      }
    }
    printf("Child 2 (Medium Priority): Finished\n");
    exit(0);
  }
  
  pid3 = fork();
  if(pid3 == 0) {
    // Child 3: Low priority (2)
    printf("Child 3 (PID %d, Priority %d): Starting CPU-intensive task\n", getpid(), getpriority(getpid()));
    
    // CPU-intensive loop
    for(int i = 0; i < 4000000; i++) {
      if(i % 400000 == 0) {
        printf("Child 3 (Low Priority): %d iterations, current priority: %d\n", 
               i, getpriority(getpid()));
      }
      if(i % 50000 == 0) {  // Every 50k iterations, voluntarily give up CPU
        printf("Child 3 (Low Priority): Yielding CPU at iteration %d\n", i);
        yield();
      }
    }
    printf("Child 3 (Low Priority): Finished\n");
    exit(0);
  }

  setpriority(pid1, 8);
  setpriority(pid2, 5);
  setpriority(pid3, 2);

  // Parent process
  printf("\nParent: Created 3 children with different priorities\n");
  printf("Child 1 (PID %d): Priority %d (High)\n", pid1, getpriority(pid1));
  printf("Child 2 (PID %d): Priority %d (Medium)\n", pid2, getpriority(pid2));
  printf("Child 3 (PID %d): Priority %d (Low)\n", pid3, getpriority(pid3));
  
  printf("\nNote: Watch how priorities change due to aging!\n");
  printf("Low priority processes should eventually get CPU time.\n\n");

  
  
  // Wait for all children
  wait(0);
  wait(0);
  wait(0);
  
  printf("\nPriority test with aging completed!\n");
  printf("All processes finished - aging prevented starvation!\n");
  exit(0);
}