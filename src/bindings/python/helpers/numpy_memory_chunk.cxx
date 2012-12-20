/*ckwg +5
 * Copyright 2012 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "numpy_memory_chunk.h"

#include <vistk/python/util/python_gil.h>

#include <boost/cstdint.hpp>

#include <vil/vil_pixel_format.h>

static vil_pixel_format convert_format(int numpy_format);

numpy_memory_chunk
::numpy_memory_chunk(PyArrayObject* arr)
  : vil_memory_chunk()
  , m_arr(arr)
{
  vistk::python::python_gil const gil;

  (void)gil;

  Py_INCREF(m_arr);

  int const numpy_format = PyArray_TYPE(m_arr);
  vil_pixel_format vil_format = convert_format(numpy_format);

  pixel_format_ = vil_format;

  /// \fixme Why does PyArray_NBYTES fail?

  size_t const pxsz = PyArray_ITEMSIZE(m_arr);
  size_t const nd = PyArray_NDIM(m_arr);
  npy_intp* const dimensions = PyArray_DIMS(m_arr);

  size_ = pxsz;

  for (size_t i = 0; i < nd; ++i)
  {
    size_ *= dimensions[i];
  }
}

numpy_memory_chunk
::~numpy_memory_chunk()
{
  release();
}

void*
numpy_memory_chunk
::data()
{
  return const_data();
}

void*
numpy_memory_chunk
::const_data() const
{
  if (!m_arr)
  {
    return vil_memory_chunk::const_data();
  }

  vistk::python::python_gil const gil;

  (void)gil;

  return PyArray_DATA(m_arr);
}

void
numpy_memory_chunk
::set_size(unsigned long n, vil_pixel_format format)
{
  if (m_arr)
  {
    size_ = 0;

    release();
  }

  vil_memory_chunk::set_size(n, format);
}

void
numpy_memory_chunk
::release()
{
  if (m_arr)
  {
    vistk::python::python_gil const gil;

    (void)gil;

    Py_DECREF(m_arr);
    m_arr = NULL;
  }
}

vil_pixel_format
convert_format(int numpy_format)
{
  switch (numpy_format)
  {

#define CONVERT_FORMAT(npy_type, cpp_type) \
  case npy_type:                           \
    return vil_pixel_format_of(static_cast<cpp_type>(0))

    CONVERT_FORMAT(NPY_BOOL, bool);
    CONVERT_FORMAT(NPY_BYTE, signed char);
    CONVERT_FORMAT(NPY_UBYTE, unsigned char);
    CONVERT_FORMAT(NPY_SHORT, short);
    CONVERT_FORMAT(NPY_USHORT, unsigned short);
    CONVERT_FORMAT(NPY_INT, int);
    CONVERT_FORMAT(NPY_UINT, unsigned int);
    CONVERT_FORMAT(NPY_LONG, long);
    CONVERT_FORMAT(NPY_ULONG, unsigned long);
#if VXL_HAS_INT_64
    CONVERT_FORMAT(NPY_LONGLONG, long long);
    CONVERT_FORMAT(NPY_ULONGLONG, unsigned long long);
#endif
    CONVERT_FORMAT(NPY_FLOAT, float);
    CONVERT_FORMAT(NPY_DOUBLE, double);

#undef CONVERT_FORMAT

    default:
      return VIL_PIXEL_FORMAT_UNKNOWN;
  }
}
