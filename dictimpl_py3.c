
#include <Python.h>
#include <stdlib.h>
#include "dictimpl.h"

// PyObject *py3dict_alloc(struct _typeobject *t, Py_ssize_t size) {
//     t->tp_new()
// }

#define MIN_FRACTION        (0.2)
#define MAX_FRACTION        (0.666)
#define INITIAL_ARRAY_LEN   (2048)
#define MIN_ARRAY_LEN       (8)

struct dictentry {
    long hash;
    PyObject *key;
    PyObject *val;
};

#define DICTENTRY_IS_CLEAR(de) ((de)->hash == -1)
#define DICTENTRY_IS_DUMMY(de) ((de)->key == NULL && (de)->hash != -1)
#define DICTENTRY_IS_EMPTY(de) ((de)->key == NULL) // EMPTY == CLEAR || DUMMY

static inline void 
dictentry_init(struct dictentry *entry) {
    assert(entry);
    entry->hash = -1;
    entry->key = entry->val = NULL;
}

static inline void 
dictentry_set(struct dictentry *entry, PyObject *key, PyObject *val, long hash) {
    assert(entry);
    assert(DICTENTRY_IS_EMPTY(entry));
    assert(key); 
    assert(val);
    assert(hash != -1);
    Py_INCREF(key);
    Py_INCREF(val);
    entry->key = key;
    entry->val = val;
    entry->hash = hash;
}

static inline void 
dictentry_update(struct dictentry *entry, PyObject *val) {
    assert(entry);
    assert(!DICTENTRY_IS_EMPTY(entry));
    assert(entry->val);
    assert(val);
    Py_DECREF(entry->val);
    Py_INCREF(val);
    entry->val = val;
}


static inline void 
dictentry_del(struct dictentry *entry) {
    assert(entry);
    assert(!DICTENTRY_IS_CLEAR(entry) && !DICTENTRY_IS_DUMMY(entry));
    assert(entry->key);
    assert(entry->val);

    Py_DECREF(entry->key);
    Py_DECREF(entry->val);
    entry->key = NULL;
    entry->val = NULL;
    entry->hash = 0; // hash != -1 -> DUMMY
}

static inline int 
fast_eq(struct dictentry *entry, PyObject *key, long hash) {
    assert(entry);
    assert(entry->key);
    assert(key);

    if (entry->key == key) {
        return 1;
    }
    
    if (entry->hash != hash) {
        return 0;
    }

    // printf("PyObject_RichCompareBool %d  %p == %p ?!!!\n", PyObject_RichCompareBool(entry->key, key, Py_EQ), entry->key, key);
    return PyObject_RichCompareBool(entry->key, key, Py_EQ);
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
freehasharray(struct dictentry **array) {
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
    
    return 0;
//     struct dictentry **newarray = NULL;
//     int i;
//     struct dictentry *entry;

//     newarray = newhasharray(newarraylen);
//     if (newarray == NULL) {
//         goto fail;
//     }

//     for (i = 0; i < d->arraylen; i++) {
//         entry = d->array[i];
//         d->array[i] = NULL;

//         while (entry != NULL) {
//             struct dictentry *next;
//             int newidx;
//             next = entry->next; 

//             // move the entry to new array
//             newidx = (unsigned long)entry->hash % newarraylen;
//             entry->next = newarray[newidx];
//             newarray[newidx] = entry;

//             entry = next; 
//         }
//     }

//     freehasharray(d->array);

//     d->array = newarray; 
//     d->arraylen = newarraylen;
//     return 0;

// fail:
//     if (newarray != NULL) {
//         PyMem_FREE(newarray);
//         newarray = NULL;
//     }
//     return -1;
}

static int lookup(struct dictimpl *d, PyObject *key, long hash, int *hashpos, PyObject **pval) {
    struct dictentry *array, *entry;
    int arrayidx, arraylen, firstDummyIdx, beginarrayidx;

    assert(d);
    assert(key);

    if (hashpos != NULL) {
        *hashpos = -1;
    }

    if (pval != NULL) {
        *pval = NULL;
    }

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }
    
    array = d->array;
    arraylen = d->arraylen;
    beginarrayidx = arrayidx = (unsigned long)hash % arraylen;
    // assert(arraylen > d->len); // make sure there are free entries

    firstDummyIdx = -1;
    entry = &array[arrayidx];
    while (!DICTENTRY_IS_CLEAR(entry)) {
        int cmp;

        if DICTENTRY_IS_DUMMY(entry) {
            if (firstDummyIdx == -1) {
                firstDummyIdx = arrayidx;
            }
        } else {
            cmp = fast_eq(entry, key, hash);
            if (cmp < 0) {
                return -1;
            }  else if (cmp > 0) {
                // found key 
                if (hashpos != NULL) {
                    *hashpos = arrayidx;
                }
                if (pval != NULL) {
                    *pval = entry->val;
                }
                return 0;
            } 
        }

        arrayidx = (arrayidx + 1) % arraylen;
        if (arrayidx == beginarrayidx) {
            break; 
        }
        entry = &array[arrayidx];
    }

    // foud a clear entry, or the full array is traversed
    if (hashpos != NULL) {
        if (firstDummyIdx != -1) {
            *hashpos = firstDummyIdx;
        } else if (DICTENTRY_IS_CLEAR(entry)) {
            *hashpos = arrayidx;
        }
    }
    return 0;
}

int dictimpl_setitem(struct dictimpl *d, PyObject *key, PyObject *val) {
    long hash;
    int hashpos;
    struct dictentry *entry;
    PyObject *oldval;
    int lkret;

    assert(d);
    assert(key);
    assert(val);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }
    
    lkret = lookup(d, key, hash, &hashpos, &oldval);
    if (lkret < 0) {
        return lkret;
    }

    assert(hashpos != -1);
    entry = &d->array[hashpos];
    if (oldval == NULL) {
        dictentry_set(entry, key, val, hash);
        d->len += 1;
    } else {
        dictentry_update(entry, val);
    }
    return 0;
}

int dictimpl_delitem(struct dictimpl *d, PyObject *key) {
    long hash;
    struct dictentry *entry;
    int lkret;
    int hashpos;
    PyObject *oldval;

    assert(d);
    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }
    
    lkret = lookup(d, key, hash, &hashpos, &oldval);
    if (lkret < 0) {
        return lkret;
    }

    if (oldval != NULL) {
        assert(hashpos != -1);
        entry = &d->array[hashpos];
        dictentry_del(entry);
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
    int lkret;
    PyObject *oldval;

    assert(d);
    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    lkret = lookup(d, key, hash, NULL, &oldval);
    if (lkret < 0) {
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
    struct dictentry *entry;
    int hashpos;
    PyObject *val;
    int lkret;

    assert(d);
    assert(key);
    assert(failobj);
    
    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    lkret = lookup(d, key, hash, &hashpos, &val);
    if (lkret < 0) {
        return NULL;
    }

    if (val == NULL) {
        val = failobj;
    }

    Py_INCREF(val);
    return val;
}

int dictimpl_clear(struct dictimpl *d) {
    // int i;
    // assert(d);

    // for (i = 0; i < d->arraylen; i++) {
    //     freelist(d->array[i]);
    //     d->array[i] = NULL;
    // }
    // d->len = 0;
    return 0;
}

int dictimpl_traverse(struct dictimpl *d, visitproc visit, void *arg) {
    // int i;
    // struct dictentry *entry;
    // assert(d);
    // for (i = 0; i< d->arraylen; i++) {
    //     entry = d->array[i];
    //     while (entry != NULL) {
    //         Py_VISIT(entry->key);
    //         Py_VISIT(entry->val);
    //         entry = entry->next;
    //     }
    // }
    return 0;
}
