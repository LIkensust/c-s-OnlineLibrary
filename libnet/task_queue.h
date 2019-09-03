#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <queue>
#include <vector>
#include "runnable.h"
#include "log.h"

enum TaskPriority {
    NO_1 = 0,
    NO_2 = 1,
    NO_3 = 2
};

struct Task {
    Runnable* mRunnable;
    TaskPriority mPriority;
    Task() = default;
    Task(Runnable* runnable, const TaskPriority priority)
        :mRunnable(runnable),
         mPriority(priority)
    {
    }
    Task(const Task&) = default;
    Task(Task&&) = default;
};

struct TaskCmp {
    bool operator()(const Task& a, const Task& b) const {
        return a.mPriority < b.mPriority;
    }
};

class TaskQueue {
public:
    ~TaskQueue() {
        Destroy();
    }

    void Destroy() {
        while(!mQueue.empty()) {
            const Task& t = mQueue.top();
            t.mRunnable->Free();
            mQueue.pop();
        }
    }


    void Push(Runnable* runnable, const TaskPriority priority = NO_3) {
        mQueue.emplace(runnable, priority);
    }

    Runnable* Front() {
        if(mQueue.empty()) {
            PLOG(WARNING, "Get Task from taskqueue but the queue is empty!");
            return NULL;
        }
        const Task& t = mQueue.top();
        return t.mRunnable;
    }

    void Pop() {
        if(mQueue.empty()) {
            PLOG(ERROR, "Pop, but the task queue is already empty");
            return;
        }
        mQueue.pop();
    }
    
    void Clear() {
        Destroy();
    }
private:
    typedef std::priority_queue<Task, std::vector<Task>, TaskCmp> tQueue;
private:
    tQueue mQueue;
};

#endif
