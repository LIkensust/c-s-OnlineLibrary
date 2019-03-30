#include <iostream>
#include "../src/include/common/common.h"
#include "../src/include/thread_pool/thread_pool.h"
using namespace std;
struct Data{
  int a_;
  int b_;
};

void fun(void * data) {
  Data *p = (Data*)data;
  cout<<p->a_<<" "<<p->b_<<endl;
}

int main()
{
  auto thread_pool = threadpool::ThreadPool::make(3);
  
  return 0;
}

