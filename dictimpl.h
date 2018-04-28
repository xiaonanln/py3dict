#ifndef DICTIMPL_H
#define DICTIMPL_H

#include <Python.h>

struct dictimpl;
#define DICTIMPL(obj) (*(struct dictimpl **)((const char *)((obj) + 1)))

PyObject *py3dict_new(PyTypeObject *, PyObject *, PyObject *);
int py3dict_init(PyObject *, PyObject *, PyObject *);

Py_ssize_t py3dict_length(PyObject *self);
PyObject *py3dict_subscript(PyObject *self, PyObject *key);
int py3dict_ass_subscript(PyObject *self, PyObject *key, PyObject *val);
// PyObject *py3dict_alloc(struct _typeobject *, Py_ssize_t);

#endif//DICTIMPL_H