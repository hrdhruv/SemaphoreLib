#include<semaphore.h>
void *handleThread(void *);

void admin_mode(int);
void user_mode(int);


extern sem_t books_semaphore;
extern sem_t issues_semaphore;
extern sem_t users_semaphore;

