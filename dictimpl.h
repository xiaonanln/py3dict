#ifndef DICTIMPL_H
#define DICTIMPL_H

#include <Python.h>
#include "common.h"

struct dictimpl;

struct dictimpl *dictimpl_new();
int dictimpl_init(struct dictimpl* d, PyObject *args, PyObject *kwds);

Py_ssize_t dictimpl_len(struct dictimpl *d);
int dictimpl_ass_subscript(struct dictimpl *d, PyObject *key, PyObject *val);
PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key);

// 

// PyObject *py3dict_new(PyTypeObject *, PyObject *, PyObject *);
// int py3dict_init(PyObject *, PyObject *, PyObject *);

// Py_ssize_t py3dict_length(PyObject *self);
// PyObject *py3dict_subscript(PyObject *self, PyObject *key);
// int py3dict_ass_subscript(PyObject *self, PyObject *key, PyObject *val);
// // PyObject *py3dict_alloc(struct _typeobject *, Py_ssize_t);

#endif//DICTIMPL_H