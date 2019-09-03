#ifndef __TASK_H__
#define __TASK_H__

class Task
{
public:
    Task() {}
    virtual ~Task() {}
    virtual void Run() = 0;
private:

};

#endif
