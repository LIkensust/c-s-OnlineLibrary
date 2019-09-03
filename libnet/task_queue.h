#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <queue>
#include "net_mutex.h"
#include "net_cond.h"

enum TaskPriority {
    NO_1 = 0,
    NO_2 = 1,
    NO_3 = 2
};

struct Task {
    std::function<void(void*)> Task;

};


class TaskQueue {


    
};

#endif
