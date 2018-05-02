#include <Python.h>
#include <structmember.h>
#include "dictimpl.h"
#include <stdio.h>
#include "docstr.h"

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    struct dictimpl *impl;
} dictObject;

#define DICTIMPL(obj) (((dictObject *)(obj))->impl)

static PyObject *dict_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
    PyObject *obj;
    struct dictimpl *dictimpl;

    dictimpl = dictimpl_new();
    // printf("__new__: dictimpl=%p\n", dictimpl);
    
    if (dictimpl == NULL) {
        return PyErr_NoMemory();
    }

    obj = PyType_GenericNew(subtype, args, kwds);
    if (obj == NULL) {
        PyMem_Del(dictimpl);
        return NULL;
    }

    DICTIMPL(obj) = dictimpl;
    return obj; 
}

static int 
dict_tp_clear(dictObject *self) {
    // printf("tp_clear: dictimpl=%p\n", DICTIMPL(self));
    return dictimpl_clear(DICTIMPL(self));
}

static int 
dict_tp_traverse(dictObject *self, visitproc visit, void *args) {
    // printf("tp_traverse: dictimpl=%p\n", DICTIMPL(self));
    return dictimpl_traverse(DICTIMPL(self), visit, args);
}

static void dict_dealloc(dictObject* self) {
    // printf("__dealloc__: dictimpl=%p\n", DICTIMPL(self));
    dictimpl_free(DICTIMPL(self));
    DICTIMPL(self) = NULL;
    Py_TYPE(self)->tp_free(self);
}

static int 
dict_init(dictObject *self, PyObject *args, PyObject *kwds) {
    // printf("__init__: dictimpl=%p\n", DICTIMPL(self));
    return dictimpl_init(DICTIMPL(self), args, kwds);
}

static Py_ssize_t 
dict_len(dictObject *self) {
    Py_ssize_t len = dictimpl_len(DICTIMPL(self));
    // printf("__len__: %ld, dictimpl=%p\n", len, DICTIMPL(self));
    return len;
}

static PyObject *
dict_sizeof(dictObject *self)
{
    Py_ssize_t size = _PyObject_SIZE(Py_TYPE(self)) + dictimpl_sizeof(DICTIMPL(self));
    // printf("__sizeof__: %ld", size);
    return PyLong_FromSsize_t(size);
}

static PyObject *
dict_subscript(dictObject *self, PyObject *key) {
    // printf("__getitem__: dictimpl=%p\n", DICTIMPL(self));
    PyObject *val = dictimpl_subscript(DICTIMPL(self), key);
    // if (val != NULL) {
    //     // printf("__getitem__ val refcount = %ld\n", Py_REFCNT(val));
    // }
    return val; 
}

static PyObject *
dict_get(dictObject *self, PyObject *args) {
    PyObject *key;
    PyObject *failobj = Py_None;

    if (!PyArg_UnpackTuple(args, "get", 1, 2, &key, &failobj))
        return NULL;
    
    return dictimpl_get(DICTIMPL(self), key, failobj);
}

static int 
dict_ass_subscript(dictObject *self, PyObject *key, PyObject *val) {
    // printf("__setitem__: dictimpl=%p\n", DICTIMPL(self));
    if (val != NULL) {
        return dictimpl_setitem(DICTIMPL(self), key, val);
    } else {
        return dictimpl_delitem(DICTIMPL(self), key);
    }
}

static PyMethodDef dict_methods[] = {
    {"get",         (PyCFunction) dict_get,      METH_VARARGS, get__doc__},
    {"__sizeof__",  (PyCFunction) dict_sizeof,   METH_NOARGS, sizeof__doc__},
    {NULL}  /* Sentinel */
};

static PyMemberDef dict_members[] = {
    // {"number", T_INT, offsetof(dictObject, number), 0, "test number"},
    {NULL}  /* Sentinel */
};

static PyMappingMethods dict_mapping_methods = {
    (lenfunc) dict_len, 
    (binaryfunc) dict_subscript,
    (objobjargproc) dict_ass_subscript,
};

static PyTypeObject dictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "dict",                      /* tp_name */
    sizeof(dictObject
),          /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)dict_dealloc,    /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    &dict_mapping_methods,       /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    // Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,   /* tp_flags */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "dict objects",          /* tp_doc */
    (traverseproc) dict_tp_traverse, /* tp_traverse */
    (inquiry) dict_tp_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    dict_methods,             /* tp_methods */
    dict_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc) dict_init,   /* tp_init */
    0,                         /* tp_alloc */
    (newfunc) dict_new,     /* tp_new */
    0,                          /* tp_free */ /* Low-level free-memory routine */
    0,                          /* tp_is_gc */
    0,                          /* tp_bases */
    0,                          /* tp_mro */
    0,                          /* tp_cache */
    0,                          /* tp_subclasses */
    0,                          /* tp_weaklist */
    0,                          /* tp_del */
};
