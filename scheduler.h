/** @file scheduler.h
 *  @brief Header file for scheduler.cpp.
 */
void set_core(pthread_attr_t* attr, int cpu_no);
void* worker_start(void* params);
