
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

#define DICTENTRY_IS_CLEAR(de) ((de)->hash == -1)
#define DICTENTRY_IS_DUMMY(de) ((de)->key == NULL && (de)->hash != -1)

static inline void 
dictentry_init(struct dictentry *entry) {
    assert(entry);
    entry->hash = -1;
    entry->key = entry->val = NULL;
}

static inline void 
dictentry_set(struct dictentry *entry, PyObject *key, PyObject *val, long hash) {
    assert(entry);
    assert(DICTENTRY_IS_CLEAR(entry));
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
    assert(!DICTENTRY_IS_CLEAR(entry) && !DICTENTRY_IS_DUMMY(entry));
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

// static struct dictentry *
// newdictentry(long hash, PyObject *key, PyObject *val) {
//     struct dictentry *entry = PyMem_New(struct dictentry, 1);
//     if (entry != NULL) {
//         Py_INCREF(key);
//         Py_INCREF(val);
//         entry->hash = hash;
//         entry->key = key;
//         entry->val = val;
//         entry->next = NULL;
//     }
//     return entry;
// }

// static void
// freedictentry(struct dictentry *entry) {
//     assert(entry && entry->key && entry->val);
//     Py_DECREF(entry->key);
//     Py_DECREF(entry->val);
//     PyMem_FREE(entry);
// }

// static void 
// freelist(struct dictentry *head) {
//     struct dictentry *next;
//     while (head != NULL) {
//         next = head->next;
//         freedictentry(head);
//         head = next; 
//     }
// }

static void setdictentryval(struct dictentry *entry, PyObject *val) {
    assert(val);
    assert(entry->val);
    Py_INCREF(val);
    Py_DECREF(entry->val);
    entry->val = val;
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

int dictimpl_setitem(struct dictimpl *d, PyObject *key, PyObject *val) {
    long hash;
    struct dictentry *array, *entry;
    int arrayidx, arraylen, firstDummyIdx;

    assert(key);
    assert(val);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }
    
    array = d->array;
    arraylen = d->arraylen;
    arrayidx = (unsigned long)hash % arraylen;
    assert(arraylen > d->len); // make sure there are free entries

    firstDummyIdx = -1;
    while (!DICTENTRY_IS_CLEAR(&array[arrayidx])) {
        struct dictentry *entry;
        int cmp;

        entry = &array[arrayidx];
        if DICTENTRY_IS_DUMMY(entry) {
            if (firstDummyIdx == -1) {
                firstDummyIdx = arrayidx;
            }
            continue ;
        }

        cmp = fast_eq(entry, key, hash);
        if (cmp < 0) {
            return -1;
        }  else if (cmp > 0) {
            dictentry_update(entry, val);
            return 0;
        } else {
            arrayidx = (arrayidx + 1) % arraylen;
        }
    }

    // eventually will find a empty entry
    if (firstDummyIdx != -1) {

    }
    
    entry = &array[arrayidx];
    dictentry_set(entry, key, val, hash);
    return 0;
}

int dictimpl_delitem(struct dictimpl *d, PyObject *key) {
    long hash;
    int cmp;
    struct dictentry *array, *entry;
    int arrayidx, arraylen;

    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }

    array = d->array;
    arraylen = d->arraylen;
    arrayidx = (unsigned long)hash % arraylen;
    entry = &array[arrayidx];
    
    while (!DICTENTRY_IS_CLEAR(entry)) {
	    assert(entry->key);
        cmp = fast_eq(entry, key, hash);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            // key exists, remove entry
            *pnode = entry->next; 
            freedictentry(entry);
            d->len -= 1;
            if (d->arraylen > MIN_ARRAY_LEN && d->len < d->arraylen * MIN_FRACTION) {
                dictimpl_resize(d, d->arraylen >> 1);
            }
            return 0;
        } else {
            pnode = &entry->next;
            entry = entry->next;
        }
    }

    // key not exists, del raise KeyError
    PyErr_SetObject(PyExc_KeyError, key);
    return -1;
}

PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key) {
    long hash;
    struct dictentry *entry;
    int cmp;
    int arrayidx;
    assert(d);
    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    arrayidx = (unsigned long)hash % d->arraylen;
    entry = d->array[arrayidx];

    while (entry != NULL) {
        cmp = fast_eq(entry, key, hash);
        if (cmp < 0) {
            return NULL;
        } else if (cmp > 0) {
            Py_INCREF(entry->val);
            return entry->val;
        } else {
            entry = entry->next;
        }
    }

    PyErr_SetObject(PyExc_KeyError, key);
    return NULL;
}

PyObject *dictimpl_get(struct dictimpl *d, PyObject *key, PyObject *failobj) {
    long hash, arrayidx;
    struct dictentry *entry;
    int cmp;

    assert(key);
    assert(failobj);
    
    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    arrayidx = (unsigned long)hash % d->arraylen;
    entry = d->array[arrayidx];
    while (entry != NULL) {
        cmp = fast_eq(entry, key, hash);
        if (cmp < 0) {
            return NULL;
        } else if (cmp > 0) {
            Py_INCREF(entry->val);
            return entry->val;
        } else {
            entry = entry->next;
        }
    }

    Py_INCREF(failobj);
    return failobj;
}

int dictimpl_clear(struct dictimpl *d) {
    int i;
    assert(d);

    for (i = 0; i < d->arraylen; i++) {
        freelist(d->array[i]);
        d->array[i] = NULL;
    }
    d->len = 0;
    return 0;
}

int dictimpl_traverse(struct dictimpl *d, visitproc visit, void *arg) {
    int i;
    struct dictentry *entry;
    assert(d);
    for (i = 0; i< d->arraylen; i++) {
        entry = d->array[i];
        while (entry != NULL) {
            Py_VISIT(entry->key);
            Py_VISIT(entry->val);
            entry = entry->next;
        }
    }
    return 0;
}
