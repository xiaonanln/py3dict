#include <Python.h>
#include <structmember.h>
#include "dictimpl.h"

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

static PyMethodDef py3dict_methods[] = {
    // {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},

    {NULL}  /* Sentinel */
};

static PyMemberDef py3dict_members[] = {
    // {"number", T_INT, offsetof(py3dictObject, number), 0, "test number"},
    {NULL}  /* Sentinel */
};

static PyMappingMethods py3dict_mapping_methods = {
    (lenfunc) py3dict_length, 
    (binaryfunc) py3dict_subscript,
    (objobjargproc) py3dict_ass_subscript,
};

static PyTypeObject py3dict_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "py3dict",             /* tp_name */
    sizeof(py3dictObject),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
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
