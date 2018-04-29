#include <Python.h>
#include <structmember.h>
#include "dictimpl.h"
#include <stdio.h>

extern PyMethodDef ModuleMethods[];

struct listnode {
    PyObject *key;
    PyObject *val;
    struct listnode *next;
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    struct dictimpl *impl;
} py3dictObject;

#define DICTIMPL(obj) (((py3dictObject *)(obj))->impl)

static PyObject *py3dict_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
    PyObject *obj;
    struct dictimpl *dictimpl;

    dictimpl = dictimpl_new();
    printf("__new__: dictimpl=%p\n", dictimpl);
    
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

static void py3dict_dealloc(py3dictObject* self) {
    struct dictimpl *d = DICTIMPL(self);
    printf("__dealloc__: dictimpl=%p\n", d);
    PyMem_Free(d);
    Py_TYPE(self)->tp_free(self);
}

static int py3dict_init(py3dictObject *self, PyObject *args, PyObject *kwds) {
    printf("__init__: dictimpl=%p\n", DICTIMPL(self));
    return dictimpl_init(DICTIMPL(self), args, kwds);
}

static Py_ssize_t py3dict_len(py3dictObject *self) {
    Py_ssize_t len = dictimpl_len(DICTIMPL(self));
    printf("__len__: %d, dictimpl=%p\n", len, DICTIMPL(self));
    return len;
}

static PyObject *py3dict_subscript(py3dictObject *self, PyObject *key) {
    printf("__getitem__: dictimpl=%p\n", DICTIMPL(self));
    return dictimpl_subscript(DICTIMPL(self), key);
}

static int py3dict_ass_subscript(py3dictObject *self, PyObject *key, PyObject *val) {
    printf("__setitem__: dictimpl=%p\n", DICTIMPL(self));
    // PyObject_Print(key, stdout, 0);
    // PyObject_Print(val, stdout, 0);
    return dictimpl_ass_subscript(DICTIMPL(self), key, val);
}

static PyMethodDef py3dict_methods[] = {
    // {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},

    {NULL}  /* Sentinel */
};

static PyMemberDef py3dict_members[] = {
    // {"number", T_INT, offsetof(py3dictObject, number), 0, "test number"},
    {NULL}  /* Sentinel */
};

static PyMappingMethods py3dict_mapping_methods = {
    (lenfunc) py3dict_len, 
    (binaryfunc) py3dict_subscript,
    (objobjargproc) py3dict_ass_subscript,
};

static PyTypeObject py3dict_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "py3dict",             /* tp_name */
    sizeof(py3dictObject),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)py3dict_dealloc,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    &py3dict_mapping_methods,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "py3dict objects",          /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py3dict_methods,             /* tp_methods */
    py3dict_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc) py3dict_init,   /* tp_init */
    0,                         /* tp_alloc */
    (newfunc) py3dict_new,     /* tp_new */
    0,                          /* tp_free */ /* Low-level free-memory routine */
    0,                          /* tp_is_gc */
    0,                          /* tp_bases */
    0,                          /* tp_mro */
    0,                          /* tp_cache */
    0,                          /* tp_subclasses */
    0,                          /* tp_weaklist */
    0,                          /* tp_del */
};

PyMODINIT_FUNC
initpy3dict(void)
{
    PyObject *m;

    // py3dict_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py3dict_type) < 0)
        return;

    m = Py_InitModule("py3dict", ModuleMethods);
    if (m == NULL)
        return;

    Py_INCREF(&py3dict_type);
    PyModule_AddObject(m, "py3dict", (PyObject *)&py3dict_type);
}

static PyMethodDef ModuleMethods[] = {
    // {"system",  spam_system, METH_VARARGS, "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};
