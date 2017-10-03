#pragma once

#include "intermediate.h"
#include "job.h"

class Task {
private:
    Operator *op;
    int batch_id;

public:
    Task(Operator *op, int batch_id) {
        this->op = op;
        this->batch_id = batch_id;
    }
};
