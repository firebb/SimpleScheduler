/** @file scheduler.cpp
 *  @brief Contains the main logic of the scheduler implementation
 *
 *  A push based scheduler for simple batched arithmatic computation. Each
 *  operator is some computation on the a batch of floating point numbers.
 *  The pipeline DAG is expressed as a Job with muliple operators and
 *  dependencies. 
 *
 *  There is one thread acting as a centralized scheduler, each thread
 *  would act as a worker. Each worker maintains a queue to take task from
 *  the scheduler. A shared result queue is used to gather the intermediate
 *  results from the workers.
 *
 *  For each batch, we create an execution plan based on the Job, so that
 *  the scheduler could keep track of the dependency for the batch. A todo
 *  list is maintained to keep all of the ready stages in the execution
 *  plan. Whenever a worker finishes its current task, the shceduler would
 *  assign the next ready stage to the worker.
 */
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <pthread.h>

#include "concurrent_queue.h"
#include "execution_plan.h"
#include "intermediate.h"
#include "scheduler.h"

#define MAX_THREADS 4

using namespace std;

typedef struct _Worker {
  int wid;
  ConcurrentQueue<Stage *> taskQueue;
  ConcurrentQueue<Intermediate *> *resultQueue;
} Worker;

int main() {
  int total_batch = 100;
  Job job(total_batch);
  // Init ops
  job.add_input(0, Operator::Type::RANDOM);
  job.add_input(1, Operator::Type::RANDOM);
  job.add_stage(2, Operator::Type::DOUBLE);
  job.add_stage(3, Operator::Type::DOUBLE);
  job.add_stage(4, Operator::Type::DEVIDE);
  job.add_output(5, Operator::Type::OUTPUT);
  // Add dependency
  job.add_dependency(0, 2);
  job.add_dependency(1, 3);
  job.add_dependency(2, 4);
  job.add_dependency(3, 4);
  job.add_dependency(4, 5);

  ConcurrentQueue<Intermediate *> resultQueue;
  Worker workers[MAX_THREADS];
  pthread_t threads[MAX_THREADS];
  pthread_attr_t attr[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_attr_init(&attr[i]);
    workers[i].wid = i;
    workers[i].resultQueue = &resultQueue;
  }

  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_create(&threads[i], &attr[i], worker_start, &workers[i]);
  }

  int cur_batch = 0;
  int finished = 0;
  std::map<int, ExecutionPlan *> plans;
  std::map<int, Stage *> worker_status;
  std::list<Stage *> todo;

  // Init by assigning each worker a task
  for (int i = 0; i < MAX_THREADS; i++) {
    if (todo.size() == 0) {
      ExecutionPlan *plan = new ExecutionPlan(&job, cur_batch++);
      plans[plan->batchId] = plan;
      std::vector<Stage *> starting;
      plan->get_input_stages(starting);
      for (auto it = starting.begin(); it != starting.end(); ++it) {
        todo.push_back(*it);
      }
    }
    Stage *stage = todo.front();
    todo.pop_front();
    worker_status[i] = stage;
    workers[i].taskQueue.push(stage);
  }

  while (finished != total_batch) {
    Intermediate *result = resultQueue.poll();
    int worker_id = result->worker_id;
    Stage *stage = worker_status[worker_id];
    int batchId = stage->batchId;
    ExecutionPlan *plan = plans[batchId];

    std::vector<int> out_stages;
    stage->get_out_stages(out_stages);

    if (out_stages.size() == 0) {
      plan->output();
      plans.erase(batchId);
      if (plan->isFinish()) {
        finished++;
        delete plan;
      }
    } else {
      for (auto it = out_stages.begin(); it != out_stages.end(); it++) {
        Stage *next = plan->stages[*it];
        next->add_input(stage->op->opId, result);
        if (next->isReady()) {
          todo.push_back(next);
        }
      }
    }

    // Assign task
    if (todo.size() == 0 && cur_batch < total_batch) {
      plan = new ExecutionPlan(&job, cur_batch++);
      plans[plan->batchId] = plan;
      std::vector<Stage *> starting;
      plan->get_input_stages(starting);
      for (auto it = starting.begin(); it != starting.end(); ++it) {
        todo.push_back(*it);
      }
    }

    if (todo.size() != 0) {
      stage = todo.front();
      todo.pop_front();
      worker_status[worker_id] = stage;
      workers[worker_id].taskQueue.push(stage);
    }
  }

  cout << "Done!" << endl;
  return 0;
}

void *worker_start(void *params) {
  Worker *worker = (Worker *)params;
  int wid = worker->wid;
  while (true) {
    Stage *stage = worker->taskQueue.poll();
    printf("op %d, batch %d\n", stage->op->opId, stage->batchId);
    double *result = stage->execute();
    Intermediate *out = new Intermediate(result, stage->op->out.size(), wid);
    worker->resultQueue->push(out);
  }
  return NULL;
}

void set_core(pthread_attr_t *attr, int cpu_no) {
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu_no, &mask);
  pthread_attr_setaffinity_np(attr, sizeof(cpu_set_t), &mask);
}
