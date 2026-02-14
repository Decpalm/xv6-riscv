#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h"
#include <stddef.h>

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getprocs(void)
{
  // Getting the passed arguments
  uint64 pinfo_addr;
  int max_procs;

  argaddr(0, &pinfo_addr);
  argint(1, &max_procs);

  // Ensuring the address is not 0, and that the max number of processes to return is not negative or more than the max number of processes
  if(pinfo_addr == 0 || max_procs < 0 || max_procs > NPROC) {
     return -1;
  }

  // Overflow check
  if(pinfo_addr + max_procs*sizeof(struct procinfo) < pinfo_addr){
    return -1;
  }

  // Ensure the address is in user space
  if(pinfo_addr + max_procs*sizeof(struct procinfo) > MAXVA){
    return -1;
  }

  // Neccesary external variables, wait_lock and proccess list
  extern struct spinlock wait_lock;
  extern struct proc proc[NPROC];

  struct proc *myp = myproc();
  int count = 0;
  struct proc *p;

  // Acquiring lock as stating in proc.c
  acquire(&wait_lock);

  // Loop once for each proccess
  for(int i = 0; i < NPROC; i++) {
    // Getting proccess from list and acquiring its lock
    p = &proc[i];
    acquire(&p->lock);

    // Only return proccess to user space if it is not unused, and do not return more that requested
    if(p->state != UNUSED && count < max_procs) {
      // Offsetting pinfo pointer
      uint64 addr = pinfo_addr + count*sizeof(struct procinfo);

      // Getting parent pid
      int ppid = 0;
      if(p->parent){
        acquire(&p->parent->lock);
        ppid = p->parent->pid;
        release(&p->parent->lock);
      }

      // Copy data from proccess table to user space, and more error handling to ensure that the address passed by the user is valid
      if(copyout(myp->pagetable, addr+offsetof(struct procinfo, pid), (char *)&p->pid, sizeof(int)) < 0 || 
         copyout(myp->pagetable, addr+offsetof(struct procinfo, ppid), (char *)&ppid, sizeof(int)) < 0 || 
         copyout(myp->pagetable, addr+offsetof(struct procinfo, state), (char *)&p->state, sizeof(int)) < 0 || 
         copyout(myp->pagetable, addr+offsetof(struct procinfo, sz), (char *)&p->sz, sizeof(uint64)) < 0 || 
         copyout(myp->pagetable, addr+offsetof(struct procinfo, name), &p->name[0], sizeof(p->name)) < 0) {
        release(&p->lock);
        release(&wait_lock);
        return -1;
      }
      count++;
    }

    release(&p->lock);

    // Exit loop early if the max_procs proccesses have been added to pinfo
    if(count >= max_procs) {
      break;
    }
  }

  release(&wait_lock);

  return count;
}