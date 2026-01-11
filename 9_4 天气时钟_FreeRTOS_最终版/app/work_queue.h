#ifndef __WORK_QUEUE_H__
#define __WORK_QUEUE_H__


typedef void (*WorkType)(void*);


void initSystemWork(void);
void sendDataToWorkQueue(WorkType work, void* param);


#endif
