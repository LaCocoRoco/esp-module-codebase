#include "task.h"

#include <TaskScheduler.h>

Scheduler scheduler;

Task handler(TASK_TIME, TASK_FOREVER, &task, &scheduler);

void setupTask() {
  scheduler.enableAll();
}

void taskLoop() {
  scheduler.execute();
}
