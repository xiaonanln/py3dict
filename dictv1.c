#include <Python.h>
#include <structmember.h>

extern PyMethodDef ModuleMethods[];

struct listnode {
    PyObject *key;
    PyObject *val;
    struct listnode *next;
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    int number; 
    struct listnode *array;
} dictv1Object;

static PyObject* say_hello(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);

    Py_RETURN_NONE;
}

static Py_ssize_t dictv1_length(PyObject *self) {
    printf("__len__\n");
    return 0;
}

static PyObject *dictv1_subscript(PyObject *self, PyObject *key) {
    printf("__getitem__\n");
    Py_RETURN_NONE;
}


static int dictv1_ass_subscript(PyObject *self, PyObject *key, PyObject *val) {
    printf("__setitem__\n");
    return 0;
}


static PyMethodDef dictv1_methods[] = {
    {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},
    // {"__setitem__", dictv1Object_SetItem, METH_VARARGS, "set item"},

    {NULL}  /* Sentinel */
};

static PyMemberDef dictv1_members[] = {
    {"number", T_INT, offsetof(dictv1Object, number), 0, "test number"},
    {NULL}  /* Sentinel */
};

static PyMappingMethods dictv1_mapping_methods = {
    (lenfunc) dictv1_length, 
    (binaryfunc) dictv1_subscript,
    (objobjargproc) dictv1_ass_subscript,
};

static PyTypeObject dictv1Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "dictv1",             /* tp_name */
    sizeof(dictv1Object),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    &dictv1_mapping_methods,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "dictv1 objects",          /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    dictv1_methods,             /* tp_methods */
    dictv1_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};

PyMODINIT_FUNC
initdictv1(void)
{
    PyObject *m;

    dictv1Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&dictv1Type) < 0)
        return;

    m = Py_InitModule("dictv1", ModuleMethods);
    if (m == NULL)
        return;

    Py_INCREF(&dictv1Type);
    PyModule_AddObject(m, "dictv1", (PyObject *)&dictv1Type);
}

static PyMethodDef ModuleMethods[] = {
    // {"system",  spam_system, METH_VARARGS, "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};
