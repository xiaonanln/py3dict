
#include <Python.h>
#include <stdlib.h>
#include "dictimpl.h"

// PyObject *py3dict_alloc(struct _typeobject *t, Py_ssize_t size) {
//     t->tp_new()
// }

struct dictimpl {
    int a;
};

PyObject *py3dict_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
    PyObject *obj;
    struct dictimpl *dictimpl;
    // printf("__new__\n");

    dictimpl = PyMem_New(struct dictimpl, 1);
    if (dictimpl == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    obj = PyType_GenericNew(subtype, args, kwds);
    if (obj == NULL) {
        PyMem_Del(dictimpl);
        return NULL;
    }

    DICTIMPL(obj) = dictimpl;
    return obj; 
}

int py3dict_init(PyObject *self, PyObject *args, PyObject *kwds) {
    printf("__init__\n");
    return 0;
}

Py_ssize_t py3dict_length(PyObject *self) {
    printf("__len__\n");
    return 0;
}

PyObject *py3dict_subscript(PyObject *self, PyObject *key) {
    printf("__getitem__\n");
    Py_RETURN_NONE;
}

int py3dict_ass_subscript(PyObject *self, PyObject *key, PyObject *val) {
    printf("__setitem__\n");
    return 0;
}

