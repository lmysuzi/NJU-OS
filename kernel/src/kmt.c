#include <kmt.h>


    
static void spin_init(spinlock_t *lk, const char *name){
  lk->flag=0;lk->name=name;
}

static void spin_lock(spinlock_t *lk){
  bool prev_status=ienabled();
  if(prev_status)return;
}
static void spin_unlock(spinlock_t *lk){

}

MODULE_DEF(kmt)={
  .spin_init=spin_init,
  .spin_lock=spin_lock,
  .spin_unlock=spin_unlock,
};