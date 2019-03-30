#include "../src/include/thread_pool/thread_pool.h"
#include "../src/include/common/common.h"
#include <iostream>
using namespace std;
struct Data {
  int a_;
  int b_;
};

void fun(void *data) {
  if (data == NULL) {
    cout << "NULL" << endl;
    return;
  }
  Data *p = (Data *)data;
  cout << p->a_ << " " << p->b_ << endl;
}

int main() {
  auto thread_pool = threadpool::ThreadPool::make(3);
  thread_pool.start();
  Data *d = new Data;
  for (int i = 0; i < 100; i++) {
    d->a_ = i;
    d->b_ = i + 1;
    thread_pool.add_task(bind(fun, d));
  }
  sleep(3);
  thread_pool.stop();
  return 0;
}
