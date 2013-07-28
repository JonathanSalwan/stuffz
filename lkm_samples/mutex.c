#include <linux/mutex.h>

struct mutex my_mutex;

void foo(void)
{
  mutex_lock(&my_mutex);
  /* ... */
  mutex_unlock(&my_mutex);
}
