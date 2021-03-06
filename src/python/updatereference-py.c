/* createrepo_c - Library of routines for manipulation with repodata
 * Copyright (C) 2013  Tomas Mlcoch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <Python.h>
#include <assert.h>
#include <stddef.h>

#include "updatereference-py.h"
#include "exception-py.h"
#include "typeconversion.h"
#include "contentstat-py.h"

typedef struct {
    PyObject_HEAD
    cr_UpdateReference *reference;
} _UpdateReferenceObject;

PyObject *
Object_FromUpdateReference(cr_UpdateReference *ref)
{
    PyObject *py_rec;

    if (!ref) {
        PyErr_SetString(PyExc_ValueError, "Expected a cr_UpdateReference pointer not NULL.");
        return NULL;
    }

    py_rec = PyObject_CallObject((PyObject *) &UpdateReference_Type, NULL);
    cr_updatereference_free(((_UpdateReferenceObject *)py_rec)->reference);
    ((_UpdateReferenceObject *)py_rec)->reference = ref;

    return py_rec;
}

cr_UpdateReference *
UpdateReference_FromPyObject(PyObject *o)
{
    if (!UpdateReferenceObject_Check(o)) {
        PyErr_SetString(PyExc_TypeError, "Expected a UpdateReference object.");
        return NULL;
    }
    return ((_UpdateReferenceObject *)o)->reference;
}

static int
check_UpdateReferenceStatus(const _UpdateReferenceObject *self)
{
    assert(self != NULL);
    assert(UpdateReferenceObject_Check(self));
    if (self->reference == NULL) {
        PyErr_SetString(CrErr_Exception, "Improper createrepo_c UpdateReference object.");
        return -1;
    }
    return 0;
}

/* Function on the type */

static PyObject *
updatereference_new(PyTypeObject *type,
                    G_GNUC_UNUSED PyObject *args,
                    G_GNUC_UNUSED PyObject *kwds)
{
    _UpdateReferenceObject *self = (_UpdateReferenceObject *)type->tp_alloc(type, 0);
    if (self) {
        self->reference = NULL;
    }
    return (PyObject *)self;
}

PyDoc_STRVAR(updatereference_init__doc__,
".. method:: __init__()\n\n");

static int
updatereference_init(_UpdateReferenceObject *self,
                     G_GNUC_UNUSED PyObject *args,
                     G_GNUC_UNUSED PyObject *kwds)
{
    /* Free all previous resources when reinitialization */
    if (self->reference)
        cr_updatereference_free(self->reference);

    /* Init */
    self->reference = cr_updatereference_new();
    if (self->reference == NULL) {
        PyErr_SetString(CrErr_Exception, "UpdateReference initialization failed");
        return -1;
    }

    return 0;
}

static void
updatereference_dealloc(_UpdateReferenceObject *self)
{
    if (self->reference)
        cr_updatereference_free(self->reference);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
updatereference_repr(G_GNUC_UNUSED _UpdateReferenceObject *self)
{
    if (self->reference->type)
        return PyString_FromFormat("<createrepo_c.UpdateReference %s object>",
                                   self->reference->type);
    else
        return PyString_FromFormat("<createrepo_c.UpdateReference object>");
}

/* UpdateReference methods */

PyDoc_STRVAR(copy__doc__,
"copy() -> UpdateReference\n\n"
"Return copy of the UpdateReference object");

static PyObject *
copy_updatereference(_UpdateReferenceObject *self, G_GNUC_UNUSED void *nothing)
{
    if (check_UpdateReferenceStatus(self))
        return NULL;
    return Object_FromUpdateReference(cr_updatereference_copy(self->reference));
}

static struct PyMethodDef updatereference_methods[] = {
    {"copy", (PyCFunction)copy_updatereference, METH_NOARGS,
        copy__doc__},
    {NULL} /* sentinel */
};

/* getsetters */

#define OFFSET(member) (void *) offsetof(cr_UpdateReference, member)

static PyObject *
get_str(_UpdateReferenceObject *self, void *member_offset)
{
    if (check_UpdateReferenceStatus(self))
        return NULL;
    cr_UpdateReference *ref = self->reference;
    char *str = *((char **) ((size_t) ref + (size_t) member_offset));
    if (str == NULL)
        Py_RETURN_NONE;
    return PyString_FromString(str);
}

static int
set_str(_UpdateReferenceObject *self, PyObject *value, void *member_offset)
{
    if (check_UpdateReferenceStatus(self))
        return -1;
    if (!PyString_Check(value) && value != Py_None) {
        PyErr_SetString(PyExc_TypeError, "String or None expected!");
        return -1;
    }
    cr_UpdateReference *ref = self->reference;
    char *str = cr_safe_string_chunk_insert(ref->chunk,
                                            PyObject_ToStrOrNull(value));
    *((char **) ((size_t) ref + (size_t) member_offset)) = str;
    return 0;
}

static PyGetSetDef updatereference_getsetters[] = {
    {"href",            (getter)get_str, (setter)set_str,
        "Reference URL",OFFSET(href)},
    {"id",              (getter)get_str, (setter)set_str,
        "ID",           OFFSET(id)},
    {"type",            (getter)get_str, (setter)set_str,
        "Type",         OFFSET(type)},
    {"title",           (getter)get_str, (setter)set_str,
        "Title",        OFFSET(title)},
    {NULL, NULL, NULL, NULL, NULL} /* sentinel */
};

/* Object */

PyTypeObject UpdateReference_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size */
    "createrepo_c.UpdateReference", /* tp_name */
    sizeof(_UpdateReferenceObject), /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) updatereference_dealloc, /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_compare */
    (reprfunc) updatereference_repr,/* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
    updatereference_init__doc__,    /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    PyObject_SelfIter,              /* tp_iter */
    0,                              /* tp_iternext */
    updatereference_methods,        /* tp_methods */
    0,                              /* tp_members */
    updatereference_getsetters,     /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc) updatereference_init,/* tp_init */
    0,                              /* tp_alloc */
    updatereference_new,            /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};
