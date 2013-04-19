#include <linux/workqueue.h>

static struct workqueue_struct *wq;

static void func_handler(struct work_struct *w)
{

}
static DECLARE_DELAYED_WORK(func, func_handler);

static int __init hook_detection_init(void)
{
  unsigned long onesec;
  
  wq = create_singlethread_workqueue("wq");
  
  onesec = msecs_to_jiffies(1000);
  queue_delayed_work(wq, &func, onesec);
}

static void __exit hook_detection_exit(void)
{
  if (wq)
    destroy_workqueue(wq);
}
