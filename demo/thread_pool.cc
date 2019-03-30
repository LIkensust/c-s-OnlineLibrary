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
  vector<Data *> v;
  for (int i = 0; i < 100; i++) {
    Data *d = new Data;
    d->a_ = i;
    d->b_ = i + 1;
    thread_pool.add_task(bind(fun, d));
    v.push_back(d);
  }
  sleep(2);
  for (int i = 0; i < 100; i++) {
    delete v[i];
  }
  return 0;
}
