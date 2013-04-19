/* 3.2.0 */

#include <linux/kthread.h>

static int handler_thread(void *arg)
{
  
}

static void foo(void)
{
  kernel_thread(handler_thread, NULL, CLONE_KERNEL); /* thread 1 */
}
