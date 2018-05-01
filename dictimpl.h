#ifndef DICTIMPL_H
#define DICTIMPL_H

#include <Python.h>
#include "common.h"

struct dictimpl;

struct dictimpl *dictimpl_new(void);
void dictimpl_free(struct dictimpl *d);
int dictimpl_init(struct dictimpl* d, PyObject *args, PyObject *kwds);

Py_ssize_t dictimpl_len(struct dictimpl *d);
int dictimpl_ass_subscript(struct dictimpl *d, PyObject *key, PyObject *val);
PyObject *dictimpl_subscript(struct dictimpl *d, PyObject *key);
PyObject *dictimpl_get(struct dictimpl *d, PyObject *key, PyObject *failobj);
Py_ssize_t dictimpl_sizeof(struct dictimpl *d);

#endif//DICTIMPL_H