
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

static struct listnode **
newhasharray(Py_ssize_t size) {
    struct listnode **array;
    array = PyMem_NEW(struct listnode *, size);
    if (array != NULL) {
        Py_ssize_t i;
        for (i = 0; i < size; i++) {
            array[i] = NULL;
        }
    }
    return array;
}


struct dictimpl {
    Py_ssize_t len;
    Py_ssize_t arraylen;
    struct listnode **array;
};

struct dictimpl *dictimpl_new() {
    struct listnode **array;
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
    Py_ssize_t size = sizeof(struct dictimpl) + d->arraylen * sizeof(d->array[0]) + d->len * sizeof(struct listnode);
    return size;
}


static int dictimpl_resize(struct dictimpl *d, Py_ssize_t newarraylen) {
    // printf("dictimpl_resize: from %ld to %ld ...\n", d->arraylen, newarraylen);
    struct listnode **newarray = NULL;
    int i;
    struct listnode *node;

    newarray = newhasharray(newarraylen);
    if (newarray == NULL) {
        goto fail;
    }

    for (i = 0; i < d->arraylen; i++) {
        node = d->array[i];
        d->array[i] = NULL;

        while (node != NULL) {
            long newhash;
            struct listnode *next;
            next = node->next; 

            newhash = PyObject_Hash(node->key);
            assert(newhash != -1);

            // move the node to new array
            newhash %= newarraylen;
            node->next = newarray[newhash];
            newarray[newhash] = node;

            node = next; 
        }
    }

    d->array = newarray; 
    d->arraylen = newarraylen;
    return 0;

fail:
    if (newarray != NULL) {
        PyMem_FREE(newarray);
        newarray = NULL;
    }
    return -1;
}


int dictimpl_ass_subscript(struct dictimpl *d, PyObject *key, PyObject *val) {
    long hash;
    int cmp;
    struct listnode **pnode;
    struct listnode *node;

    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return -1;
    }

    hash = hash % d->arraylen;
    pnode = &d->array[hash];
    node = d->array[hash];
    
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
                if (d->arraylen > MIN_ARRAY_LEN && d->len < d->arraylen * MIN_FRACTION) {
                    dictimpl_resize(d, d->arraylen >> 1);
                }
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
    if (d->len > d->arraylen * MAX_FRACTION) {
        dictimpl_resize(d, d->arraylen << 1);
    }
    // printf("ass_subscript returns\n");
    return 0;
}

PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key) {
    long hash;
    struct listnode *node;
    int cmp;
    assert(d);
    assert(key);

    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    hash %= d->arraylen;
    node = d->array[hash];

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
    long hash;
    struct listnode *node;
    int cmp;

    assert(key);
    assert(failobj);
    
    hash = PyObject_Hash(key);
    if (hash == -1) {
        return NULL;
    }

    hash %= d->arraylen;
    node = d->array[hash];
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
    struct listnode *node;
    assert(d);
    for (i = 0; i< d->arraylen; i++) {
        node = d->array[i];
        while (node != NULL) {
            Py_VISIT(node->key);
            Py_VISIT(node->val);
            node = node->next;
        }
    }
    return 0;
}
