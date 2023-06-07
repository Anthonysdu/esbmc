#include <pthread.h>
#include <assert.h>

int join1,join2;

void* t1(void* arg) {
  join1 = 1;
}

void* t2(void* arg) {
  join2 = 1;
}

int main(void) {
  pthread_t id1, id2;
  pthread_create(&id1, NULL, t1, NULL);
  pthread_create(&id2, NULL, t2, NULL);
  
  if(join1 && join2)
    assert(0);

  return 0;
}
