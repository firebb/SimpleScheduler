#pragma once

#include <random>
#include <vector>
#include <map>

#include "intermediate.h"

class Operator {
public:
    enum Type {
        RANDOM,
        ADD,
        MINUS,
        MULTIPLE,
        DEVIDE,
        DOUBLE,
        OUTPUT
    };

    Type type;
    int opId;
    std::vector<Operator *> in;
    std::map<int, int> inOrder;
    std::vector<Operator *> out;

    Operator(int opId, Type type) {
        this->opId = opId;
        this->type = type;
    }

    double *execute(double **input) {
        double *result = NULL;
        switch (type) {
        case Type::RANDOM:
            result = new double[INTERMEDIATE_SIZE];
            srand(time(NULL));
            for (int i = 0; i < INTERMEDIATE_SIZE; i++) {
                result[i] = rand() / (RAND_MAX / 1000);
            }
            break;
        case Type::DOUBLE:
            result = new double[INTERMEDIATE_SIZE];
            for (int i = 0; i < INTERMEDIATE_SIZE; i++) {
                result[i] = *(input[0] + i) * 2;
            }
            break;
        case Type::DEVIDE:
            result = new double[INTERMEDIATE_SIZE];
            for (int i = 0; i < INTERMEDIATE_SIZE; i++) {
                double a = *(input[0] + i);
                double b = *(input[1] + i);
                result[i] = a/b;
            }
            break;
        default:
            break;
        }
        return result;
    }
};

class Job {
public:
    std::vector<Operator *> inputs;
    std::vector<Operator *> outputs;
    std::map<int, Operator *> ops;
    int numBatch;

    Job(int numBatch) {
        this->numBatch = numBatch;
    }

    void add_input(int opId, Operator::Type type) {
        Operator *op = new Operator(opId, type);
        inputs.push_back(op);
        ops[opId] = op;
    }

    void add_stage(int opId, Operator::Type type) {
        Operator *op = new Operator(opId, type);
        ops[opId] = op;
    }

    void add_output(int opId, Operator::Type type) {
        Operator *op = new Operator(opId, type);
        outputs.push_back(op);
        ops[opId] = op;
    }

    void add_dependency(int op1, int op2) {
        ops[op1]->out.push_back(ops[op2]);
        ops[op2]->inOrder[op1] = ops[op2]->in.size();
        ops[op2]->in.push_back(ops[op1]);
    }
};
