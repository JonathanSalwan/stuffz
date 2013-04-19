static unsigned long *get_syscalls_table(void)
{
  unsigned long *start;

  for (start = (unsigned long *)0xc0000000; start < (unsigned long *)0xffffffff; start++)
    if (start[__NR_close] == (unsigned long)sys_close){
      return start;
    }
  return NULL;
}

