//
// User-space helpers for kernel threads.
//
// thread_create() and thread_join() are thin wrappers over the clone()
// and join() system calls.  They mirror the spirit of pthread_create()
// and pthread_join():

#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"
#include "user/uthread.h"

int
thread_create(void (*fn)(void *), void *arg, int n_pages)
{
  void *stack_bottom = malloc(n_pages * PGSIZE);
  if (stack_bottom == 0) return -1;

  void *stack_top = (void*)((uint64)stack_bottom + n_pages * PGSIZE);

  int pid = clone(fn, arg, stack_top, n_pages, CLONE_VM);

  if (pid < 0){
    free(stack_bottom);
    return -1;
  }

  return pid;
}

int
thread_join(void)
{
  void *stack = 0;

  int pid = join(&stack);
  if (pid < 0) return -1;

  if (pid >= 0 && stack != 0){
    free(stack);
  }
  return pid;
}
