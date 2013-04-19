/* 3.2.0 */

#include <linux/timer.h>

#define TIME_SLEEP 30000 /* in msec */

static struct timer_list timer_s;

static void timer_handler(unsigned long data)
{
  printk("Timer handler !\n");
}

static void foo(void)
{
  setup_timer(&timer_s, timer_handler, 0);
  if (mod_timer(&timer_s, jiffies + msecs_to_jiffies(TIME_SLEEP))){
    printk(KERN_INFO "Failed to set timer\n");
    return -ECANCELED;
  }
}
