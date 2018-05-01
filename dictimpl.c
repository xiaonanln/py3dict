
#include <Python.h>
#include <stdlib.h>
#include "dictimpl.h"

// PyObject *py3dict_alloc(struct _typeobject *t, Py_ssize_t size) {
//     t->tp_new()
// }

struct listnode {
    PyObject *key;
    PyObject *val;
    struct listnode *next;
};

static struct listnode *
newlistnode(PyObject *key, PyObject *val) {
    struct listnode *node = PyMem_New(struct listnode, 1);
    if (node != NULL) {
        Py_INCREF(key);
        Py_INCREF(val);
        node->key = key;
        node->val = val;
        node->next = NULL;
    }
    return node;
}

static void
freelistnode(struct listnode *node) {
    assert(node && node->key && node->val);
    Py_DECREF(node->key);
    Py_DECREF(node->val);
    PyMem_FREE(node);
}

static void 
freelist(struct listnode *head) {
    struct listnode *next;
    while (head != NULL) {
        next = head->next;
        freelistnode(head);
        head = next; 
    }
}

static void setlistnodeval(struct listnode *node, PyObject *val) {
    assert(val);
    assert(node->val);
    Py_INCREF(val);
    Py_DECREF(node->val);
    node->val = val;
}

struct dictimpl {
    Py_ssize_t len;
    int arraylen;
    struct listnode *array[8];
};

struct dictimpl *dictimpl_new() {
    struct dictimpl *d = PyMem_New(struct dictimpl, 1);
    int i;
    if (d == NULL) {
        return NULL;
    }
    d->len = 0;
    d->arraylen = ARRAY_LEN(d->array);
    for (i = 0; i < d->arraylen; i++) {
        // printf("init array[%d] from %p to %p\n", i, d->array[i], NULL);
        d->array[i] = NULL;
    }
    
    return d ;
}

void dictimpl_free(struct dictimpl *d) {
    assert(d);
    int i;
    for (i = 0; i < d->arraylen; i++) {
        freelist(d->array[i]);
        d->array[i] = NULL;
    }
    PyMem_FREE(d);
}

int dictimpl_init(struct dictimpl* d, PyObject *args, PyObject *kwds) {
    return 0;
}

Py_ssize_t dictimpl_len(struct dictimpl *d) {
    return d->len;
}

Py_ssize_t dictimpl_sizeof(struct dictimpl *d) {
    Py_ssize_t size = sizeof(struct dictimpl) + d->arraylen * sizeof(d->array[0]) + d->len * sizeof(struct listnode);
    return size;
}

int dictimpl_ass_subscript(struct dictimpl *d, PyObject *key, PyObject *val) {
    int hash = PyObject_Hash(key) % d->arraylen;
    int cmp;

    struct listnode **pnode = &d->array[hash];
    struct listnode *node = d->array[hash];
    
    assert(key);
    
    while (node != NULL) {
	    assert(node->key);
        cmp = PyObject_RichCompareBool(node->key, key, Py_EQ);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            // key exists
            if (val != NULL) {
                // update node with new value
                setlistnodeval(node, val);
            } else {
                // remove node
                *pnode = node->next; 
                freelistnode(node);
                d->len -= 1;
            }
            return 0;
        } else {
            pnode = &node->next;
            node = node->next;
        }
    }

    // key not exists, del raise KeyError
    if (val == NULL) {
        PyErr_SetObject(PyExc_KeyError, key);
        return -1;
    }
	
    node = newlistnode(key, val);
    // printf("newlistnode %p, keyref=%d\n", node, Py_REFCNT(key));
    if (node == NULL) {
    	// printf("nomemory\n");
        PyErr_NoMemory();
        return -1;
    }

    	// printf("put to array: node=%p, hash=%d, d=%p, array=%p\n", node, hash, d, d->array[hash]);
    node->next = d->array[hash];
    d->array[hash] = node;
    d->len += 1;
    // printf("ass_subscript returns\n");
    return 0;
}

PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key) {
    int hash = PyObject_Hash(key) % d->arraylen;
    struct listnode *node = d->array[hash];
    int cmp;

    while (node != NULL) {
        cmp = PyObject_RichCompareBool(node->key, key, Py_EQ);
        if (cmp < 0) {
            return NULL;
        } else if (cmp > 0) {
            Py_INCREF(node->val);
            return node->val;
        } else {
            node = node->next;
        }
    }

    PyErr_SetObject(PyExc_KeyError, key);
    return NULL;
}

PyObject *dictimpl_get(struct dictimpl *d, PyObject *key, PyObject *failobj) {
    int hash = PyObject_Hash(key) % d->arraylen;
    struct listnode *node = d->array[hash];
    int cmp;

    assert(key);
    assert(failobj);
    while (node != NULL) {
        cmp = PyObject_RichCompareBool(node->key, key, Py_EQ);
        if (cmp < 0) {
            return NULL;
        } else if (cmp > 0) {
            Py_INCREF(node->val);
            return node->val;
        } else {
            node = node->next;
        }
    }

    Py_INCREF(failobj);
    return failobj;
}


