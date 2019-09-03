#ifndef __RUNNABLE_H__
#define __RUNNABLE_h__

class Runnable
{
public:
    Runnable() {}
    virtual ~Runnable() {}
public:
    virtual void Run() = 0;
    virtual void Free() {
        delete this;
    }
};

#endif
