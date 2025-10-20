#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>
#include "task.h"
void kernel_init(void);
void kernel_add_task(tcb_t* new_task);
void kernel_launch(void);
#endif
