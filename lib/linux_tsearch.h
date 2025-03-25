#ifndef TSEARCH_H__
#define TSEARCH_H__

typedef int (* 	__compar_fn_t )(const void *, const void *);
typedef void (*__action_fn_t) (const void *__nodep, VISIT __value,
                               int __level);
typedef void (*__free_fn_t) (void *__nodep);

void *
tsearch (const void *key, void **vrootp, __compar_fn_t compar);
void *
tfind (const void *key, void *const *vrootp, __compar_fn_t compar);
void
twalk (const void *vroot, __action_fn_t action);
void *
tdelete (const void *key, void **vrootp, __compar_fn_t compar);
void
tdestroy (void *vroot, __free_fn_t freefct);

#endif
