
#include <Python.h>
#include <stdlib.h>
#include "dictimpl.h"

// PyObject *py3dict_alloc(struct _typeobject *t, Py_ssize_t size) {
//     t->tp_new()
// }

#define MIN_FRACTION        (0.2)
#define MAX_FRACTION        (0.666)
#define INITIAL_ARRAY_LEN   (8)
#define MIN_ARRAY_LEN       (8)

struct dictentry {
    long hash;
    PyObject *key;
    PyObject *val;
};

#define DICTENTRY_IS_CLEAR(de)      ((de)->hash == -1)
#define DICTENTRY_IS_DUMMY(de)      ((de)->key == NULL && (de)->hash != -1)
#define DICTENTRY_IS_EMPTY(de)      ((de)->key == NULL) // EMPTY == CLEAR || DUMMY
#define DICTENTRY_CLEAR_DUMMY(de)   do {    \
    assert(DICTENTRY_IS_DUMMY(de));         \
    (de)->hash = -1;                        \
} while(0)

static inline void 
dictentry_init(struct dictentry *oldentry) {
    assert(oldentry);
    oldentry->hash = -1;
    oldentry->key = oldentry->val = NULL;
}

static inline void 
dictentry_set(struct dictentry *oldentry, PyObject *key, PyObject *val, long hash) {
    assert(oldentry);
    assert(DICTENTRY_IS_EMPTY(oldentry));
    assert(key); 
    assert(val);
    assert(hash != -1);
    Py_INCREF(key);
    Py_INCREF(val);
    oldentry->key = key;
    oldentry->val = val;
    oldentry->hash = hash;
}

static inline void 
dictentry_update(struct dictentry *oldentry, PyObject *val) {
    assert(oldentry);
    assert(!DICTENTRY_IS_EMPTY(oldentry));
    assert(oldentry->val);
    assert(val);
    Py_DECREF(oldentry->val);
    Py_INCREF(val);
    oldentry->val = val;
}


static inline void 
dictentry_del(struct dictentry *oldentry) {
    assert(oldentry);
    assert(!DICTENTRY_IS_CLEAR(oldentry) && !DICTENTRY_IS_DUMMY(oldentry));
    assert(oldentry->key);
    assert(oldentry->val);

    Py_DECREF(oldentry->key);
    Py_DECREF(oldentry->val);
    oldentry->key = NULL;
    oldentry->val = NULL;
    oldentry->hash = 0; // hash != -1 -> DUMMY
}

static inline void 
dictentry_assign(struct dictentry *newentry, struct dictentry *oldentry) {
    assert(oldentry);
    assert(newentry);
    assert(DICTENTRY_IS_CLEAR(newentry));
    assert(!DICTENTRY_IS_EMPTY(oldentry));

    newentry->hash = oldentry->hash;
    newentry->key = oldentry->key;
    newentry->val = oldentry->val;
    oldentry->key = oldentry->val = NULL;
    return ;
}

static inline int 
fast_eq(struct dictentry *oldentry, PyObject *key, long hash) {
    assert(oldentry);
    assert(oldentry->key);
    assert(key);

    if (oldentry->key == key) {
        return 1;
    }
    
    if (oldentry->hash != hash) {
        return 0;
    }

    // printf("PyObject_RichCompareBool %d  %p == %p ?!!!\n", PyObject_RichCompareBool(oldentry->key, key, Py_EQ), oldentry->key, key);
    return PyObject_RichCompareBool(oldentry->key, key, Py_EQ);
}

static struct dictentry *
newhasharray(Py_ssize_t size) {
    struct dictentry *array;
    array = PyMem_NEW(struct dictentry, size);
    if (array != NULL) {
        Py_ssize_t i;
        for (i = 0; i < size; i++) {
            dictentry_init(&array[i]);
        }
    }
    return array;
}

static void 
freehasharray(struct dictentry *array) {
    assert(array);
    PyMem_FREE(array);
}

struct dictimpl {
    Py_ssize_t len;
    Py_ssize_t arraylen;
    struct dictentry *array;
};

struct dictimpl *dictimpl_new() {
    struct dictentry *array;
    struct dictimpl *d = PyMem_NEW(struct dictimpl, 1);

    if (d == NULL) {
        return NULL;
    }

    array = newhasharray(INITIAL_ARRAY_LEN);
    if (array == NULL) {
        PyMem_FREE(d);
        return NULL;
    }

    d->array = array;
    d->len = 0;
    d->arraylen = INITIAL_ARRAY_LEN;
    
    return d ;
}

void dictimpl_free(struct dictimpl *d) {
    assert(d);
    assert(d->array);
    dictimpl_clear(d);
    PyMem_FREE(d->array);
    PyMem_FREE(d);
}

int dictimpl_init(struct dictimpl* d, PyObject *args, PyObject *kwds) {
    return 0;
}

Py_ssize_t dictimpl_len(struct dictimpl *d) {
    return d->len;
}

Py_ssize_t dictimpl_sizeof(struct dictimpl *d) {
    Py_ssize_t size = sizeof(struct dictimpl) + d->arraylen * sizeof(d->array[0]);
    return size;
}


static int dictimpl_resize(struct dictimpl *d, Py_ssize_t newarraylen) {
    // printf("dictimpl_resize: from %ld to %ld ...\n", d->arraylen, newarraylen);
    struct dictentry *newarray, *oldarray;
    Py_ssize_t i, oldarraylen;

    assert(d->len <= newarraylen);
    assert(d->len <= d->arraylen);

    newarray = newhasharray(newarraylen);
    if (newarray == NULL) {
        return -1;
    }

    oldarray = d->array;
    oldarraylen = d->arraylen;

    for (i = 0; i < oldarraylen; i++) {
        struct dictentry *oldentry, *newentry;
        long hash;
        Py_ssize_t idx;
        oldentry = &oldarray[i];
        if (DICTENTRY_IS_EMPTY(oldentry)) {
            continue ;
        }

        hash = oldentry->hash;
        idx = (size_t)hash % newarraylen;
        newentry = &newarray[idx];
        while (!DICTENTRY_IS_CLEAR(newentry)) { // there is no DUMMY in new array 
            idx = (idx + 1) % newarraylen;
            newentry = &newarray[idx];
        }

        dictentry_assign(newentry, oldentry);
    }
    
    freehasharray(oldarray);
    d->array = newarray; 
    d->arraylen = newarraylen;
    // d->len not change
    return 0;
}

static int lookup(struct dictimpl *d, PyObject *key, long hash, int *hashpos, PyObject **pval) {
    struct dictentry *array, *oldentry;
    int arrayidx, arraylen, firstDummyIdx, beginarrayidx;

    assert(d);
    assert(key);

    if (hashpos != NULL) {
        *hashpos = -1;
    }

    if (pval != NULL) {
        *pval = NULL;
    }

    array = d->array;
    arraylen = d->arraylen;
    beginarrayidx = arrayidx = (size_t)hash % arraylen;
    // assert(arraylen > d->len); // make sure there are free entries

    firstDummyIdx = -1;
    oldentry = &array[arrayidx];
    while (!DICTENTRY_IS_CLEAR(oldentry)) {
        int cmp;

        if DICTENTRY_IS_DUMMY(oldentry) {
            if (firstDummyIdx == -1) {
                firstDummyIdx = arrayidx;
            }
        } else {
            cmp = fast_eq(oldentry, key, hash);
            if (cmp < 0) {
                return -1;
            }  else if (cmp > 0) {
                // found key 
                if (hashpos != NULL) {
                    *hashpos = arrayidx;
                }
                if (pval != NULL) {
                    *pval = oldentry->val;
                }
                return 0;
            } 
        }

        arrayidx = (arrayidx + 1) % arraylen;
        if (arrayidx == beginarrayidx) {
            break; 
        }
        oldentry = &array[arrayidx];
    }

    // foud a clear oldentry, or the full array is traversed
    if (hashpos != NULL) {
        if (firstDummyIdx != -1) {
            *hashpos = firstDummyIdx;
        } else if (DICTENTRY_IS_CLEAR(oldentry)) {
            *hashpos = arrayidx;
        }
    }
    return 0;
}

static int
dictimpl_setitem_with_hash(struct dictimpl *d, PyObject *key, PyObject *val, long hash) {
    int err; 
    int hashpos;
    PyObject *oldval;
    struct dictentry *oldentry;

    err = lookup(d, key, hash, &hashpos, &oldval);
    if (err < 0) {
        return err;
    }

    assert(hashpos != -1);
    oldentry = &d->array[hashpos];
    if (oldval == NULL) {
        dictentry_set(oldentry, key, val, hash);
        d->len += 1;
    } else {
        dictentry_update(oldentry, val);
    }
    return 0;
}

int dictimpl_setitem(struct dictimpl *d, PyObject *key, PyObject *val) {
    long hash;
    int err;

    assert(d);
    assert(key);
    assert(val);

    if ((d->len + 1) > MAX_FRACTION * d->arraylen) {
        err = dictimpl_resize(d, d->arraylen << 1);
        if (err < 0) {
            return err; 
        }
    }

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }

    return dictimpl_setitem_with_hash(d, key, val, hash);
}

int dictimpl_delitem(struct dictimpl *d, PyObject *key) {
    long hash;
    struct dictentry *oldentry;
    int err;
    int hashpos;
    PyObject *oldval;

    assert(d);
    assert(key);


    if (d->arraylen > MIN_ARRAY_LEN && d->len-1 < MIN_FRACTION * d->arraylen) {
        err = dictimpl_resize(d, d->arraylen >> 1);
        if (err < 0) {
            return err; 
        }
    }

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }
    
    err = lookup(d, key, hash, &hashpos, &oldval);
    if (err < 0) {
        return err;
    }

    if (oldval != NULL) {
        assert(hashpos != -1);
        oldentry = &d->array[hashpos];
        dictentry_del(oldentry);
        d->len -= 1;
        return 0;
    } else {
        // key not exists, raise KeyError
        PyErr_SetObject(PyExc_KeyError, key);
        return -1;
    }
}

PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key) {
    long hash;
    int err;
    PyObject *oldval;

    assert(d);
    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    err = lookup(d, key, hash, NULL, &oldval);
    if (err < 0) {
        return NULL;
    }

    if (oldval != NULL) {
        Py_INCREF(oldval);
        return oldval;
    } else {
        PyErr_SetObject(PyExc_KeyError, key);
        return NULL;
    }
}

PyObject *dictimpl_get(struct dictimpl *d, PyObject *key, PyObject *failobj) {
    long hash;
    int hashpos;
    PyObject *val;
    int err;

    assert(d);
    assert(key);
    assert(failobj);
    
    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    err = lookup(d, key, hash, &hashpos, &val);
    if (err < 0) {
        return NULL;
    }

    if (val == NULL) {
        val = failobj;
    }

    Py_INCREF(val);
    return val;
}

int dictimpl_clear(struct dictimpl *d) {
    Py_ssize_t i;
    struct dictentry *array;

	assert(d);
    array = d->array;
    for (i = 0; i < d->arraylen; i++) {
        struct dictentry *entry;
        entry = &array[i];
        if (!DICTENTRY_IS_EMPTY(entry)) {
            dictentry_del(entry);
        } else if (DICTENTRY_IS_DUMMY(entry)) {
            DICTENTRY_CLEAR_DUMMY(entry);
        }
    }
    d->len = 0;

    dictimpl_resize(d, MIN_ARRAY_LEN);

    return 0;
}

int dictimpl_traverse(struct dictimpl *d, visitproc visit, void *arg) {
    Py_ssize_t i;
    assert(d);

    struct dictentry *array = d->array;

    for (i = 0; i< d->arraylen; i++) {
        struct dictentry *entry;
        entry  = &array[i];
        if (!DICTENTRY_IS_EMPTY(entry)) {
            Py_VISIT(entry->key);
            Py_VISIT(entry->val);
        }
    }
    return 0;
}
