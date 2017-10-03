/** @file execution_plan.h
 *  @brief Contains the class for the execution plan.
 *
 *  An execution plan contains all the stages for a batch to be processed
 *  in a given pipeline DAG. Each stage would be used as placeholder for
 *  the temporary result from the previous stage. It would also be pushed
 *  to the worker to perform the actual computation.
 */
#pragma once

#include <vector>
#include <map>

#include "intermediate.h"
#include "job.h"

class Stage {
 private:
  std::map<int, Intermediate *> inputs;

 public:
  Operator *op;
  int batchId;
  size_t inputNum;
  Stage(Operator *op, int batchId) {
    this->op = op;
    this->batchId = batchId;
    inputNum = op->in.size();
  }

  bool isReady() { return inputs.size() == inputNum; }

  void add_input(int opId, Intermediate *in) { inputs[op->inOrder[opId]] = in; }

  void get_out_stages(std::vector<int> &list) {
    auto it = op->out.begin();
    for (; it != op->out.end(); ++it) {
      Operator *o = *it;
      list.push_back(o->opId);
    }
  }

  double *execute() {
    double *result;
    if (inputNum == 0) {
      result = op->execute(NULL);
    } else {
      double *intermediate[inputNum];
      for (size_t i = 0; i < inputNum; i++) {
        intermediate[i] = inputs[i]->get_result();
      }
      result = op->execute(intermediate);
      // Free temp results
      for (size_t i = 0; i < inputNum; i++) {
        if (inputs[i]->decrement_ref() == 0) delete inputs[i];
      }
    }
    return result;
  }
};

class ExecutionPlan {
 public:
  Job *job;
  int batchId;
  size_t outCount;  // Num of outputs to finish
  std::map<int, Stage *> stages;

  ExecutionPlan(Job *job, int batchId) {
    this->job = job;
    this->batchId = batchId;

    // Generate execution plan according to job DAG
    auto it = job->ops.begin();
    for (; it != job->ops.end(); ++it) {
      int opId = it->first;
      Operator *op = it->second;
      Stage *stage = new Stage(op, batchId);
      stages[opId] = stage;
    }

    outCount = job->outputs.size();
  }

  void get_input_stages(std::vector<Stage *> &list) {
    auto it = job->inputs.begin();
    for (; it != job->inputs.end(); ++it) {
      list.push_back(stages[(*it)->opId]);
    }
  }

  void output() { outCount--; }

  bool isFinish() { return outCount == 0; }
};
