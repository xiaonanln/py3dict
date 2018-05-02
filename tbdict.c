#include <Python.h>
#include "dicttype.h"

static PyMethodDef ModuleMethods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
inittbdict(void)
{
    PyObject *m;

    dictType.tp_name = "tbdict";
    if (PyType_Ready(&dictType) < 0)
        return;
    
    m = Py_InitModule("tbdict", ModuleMethods);
    if (m == NULL)
        return;

    Py_INCREF(&dictType);
    PyModule_AddObject(m, "tbdict", (PyObject *)&dictType);
}
