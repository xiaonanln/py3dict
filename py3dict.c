#include <Python.h>
#include "dicttype.h"

static PyMethodDef ModuleMethods[] = {
    // {"system",  spam_system, METH_VARARGS, "Execute a shell command."},
    // {"get", py3dict_get, METH_VARARGS, "get"}, 
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initpy3dict(void)
{
    PyObject *m;

    dictType.tp_name = "py3dict";
    if (PyType_Ready(&dictType) < 0)
        return;

    m = Py_InitModule("py3dict", ModuleMethods);
    if (m == NULL)
        return;

    Py_INCREF(&dictType);
    PyModule_AddObject(m, "py3dict", (PyObject *)&dictType);
}
