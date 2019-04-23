#pragma once
#include "../common/include/common.h"
#define THREADPOOLSIZE 5
namespace threadpool {
enum TaskPriority { NO_1 = 0, NO_2 = 1, NO_3 = 2 };

typedef std::function<void(void *)> Task;
typedef std::pair<TaskPriority, Task> TaskPair;

class ThreadPool {
public:
  static ThreadPool make(int n = THREADPOOLSIZE) {
    return *(new ThreadPool(n));
  }
  ~ThreadPool() {
    if (pool_is_start_) {
      stop();
    }
  }

  void stop() {
    if (pool_is_start_ == false)
      return;
    pthread_mutex_lock(&mutex_);
    pool_is_start_ = false;
    pthread_cond_broadcast(&cond_);
    pthread_mutex_unlock(&mutex_);
    pthread_cond_broadcast(&cond_);
    for (auto it = threads_.begin(); it != threads_.end(); it++) {
      (*it)->join();
      delete *it;
    }
    threads_.clear();
#ifdef THREADPOOL_DEBUG
    std::cout << "[=THREADPOOL_DEBUG=][thread_pool is closed]" << std::endl;
#endif
  }

  Task take() {
    pthread_mutex_lock(&mutex_);
    while (tasks_.empty() && pool_is_start_) {
      pthread_cond_wait(&cond_, &mutex_);
    }
    Task task;
    Tasks::size_type tmpsize = tasks_.size();
    if (!tasks_.empty() && pool_is_start_) {
      task = tasks_.top().second;
      tasks_.pop();
      ASSERT_MSG(tmpsize - 1 == tasks_.size(),
                 "ERR when threadpool take task!");
#ifdef THREADPOOL_DEBUG
      std::cout << "[=THREADPOOL_DEBUG=][take a task]" << std::endl;
#endif
    }
    pthread_mutex_unlock(&mutex_);
    return task;
  }

  void thread_loop() {
#ifdef THREADPOOL_DEBUG
    std::cout << "[=THREADPOOL_DEBUG=][thread_loop : "
              << std::this_thread::get_id() << " start]" << std::endl;
#endif
    while (pool_is_start_) {
#ifdef THREADPOOL_DEBUG
      std::cout << "[=THREADPOOL_DEBUG=][thread_loop : "
                << std::this_thread::get_id() << " is in loop]" << std::endl;
#endif
      Task task = take();
      if (task != nullptr) {
#ifdef THREADPOOL_DEBUG
        std::cout << "[=THREADPOOL_DEBUG=][thread_loop : "
                  << std::this_thread::get_id() << " take a task]" << std::endl;
#endif
        task(NULL);
      }
    }
#ifdef THREADPOOL_DEBUG
    std::cout << "[=THREADPOOL_DEBUG=][thread_loop : "
              << std::this_thread::get_id() << " exit]" << std::endl;
#endif
  }

  void add_task(const Task &task, TaskPriority priority = NO_2) {
#ifdef THREADPOOL_DEBUG
    std::cout << "[=THREADPOOL_DEBUG=][add task to thread pool]" << std::endl;
#endif
    pthread_mutex_lock(&mutex_);
    TaskPair taskpair(priority, task);
    tasks_.push(taskpair);
    // pthread_cond_broadcast(&cond_);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
  }

  void start() {
    ASSERT_MSG(threads_.empty(), "Thread list is not empty when start!");
    ASSERT_MSG(pool_is_start_ == false, "Restart threadpool!");
#ifdef THREADPOOL_DEBUG
    std::cout << "[=THREADPOOL_DEBUG=][thread_pool size is "
              << init_thread_size_ << " ]" << std::endl;
#endif
    threads_.reserve(init_thread_size_);
    pool_is_start_ = true;
    for (size_t i = 1; i <= init_thread_size_; i++) {
      threads_.push_back(
          new std::thread(std::bind(&ThreadPool::thread_loop, this)));
    }
  }

private:
  ThreadPool(size_t n) : init_thread_size_(n), pool_is_start_(false) {
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
  };
  // ThreadPool(const ThreadPool &);
  const ThreadPool &operator=(const ThreadPool &);
  struct TaskPriorityCmp {
    bool operator()(const TaskPair p1, const TaskPair p2) {
      return p1.first > p2.first;
    }
  };
  typedef std::vector<std::thread *> Threads;
  typedef std::priority_queue<TaskPair, std::vector<TaskPair>, TaskPriorityCmp>
      Tasks;

  size_t init_thread_size_;
  Threads threads_;
  Tasks tasks_;
  pthread_mutex_t mutex_;
  pthread_cond_t cond_;
  bool pool_is_start_;
};
} // namespace threadpool
