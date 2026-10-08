//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <Python.h>
#include <QCoreApplication>

namespace
{
PyObject* processEvents(PyObject*, PyObject*)
{
  // Python polls job state on this thread; dispatch the Qt callbacks that update it.
  QCoreApplication::processEvents();
  Py_RETURN_NONE;
}

PyMethodDef methods[] = {
  { "process_events", processEvents, METH_NOARGS, "Process pending Qt events." },
  { nullptr, nullptr, 0, nullptr }
};
PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "_queueJobTest",
  nullptr, // m_doc
  -1,      // m_size: the module has no per-interpreter state.
  methods,
  nullptr, // m_slots
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr  // m_free
};

PyObject* initializeModule()
{
  return PyModule_Create(&module);
}
} // namespace

int main(int argc, char* argv[])
{
  // Keep Qt alive for the entire Python session, including object destruction.
  // Give Qt separate arguments so it does not consume Python's command line.
  int qtArgc = 1;
  char* qtArgv[] = { argv[0], nullptr };
  QCoreApplication app(qtArgc, qtArgv);
  // Test-only support avoids requiring a separate Python Qt binding.
  if (PyImport_AppendInittab("_queueJobTest", initializeModule) == -1)
  {
    return 1;
  }
  // Preserve Python's command-line handling and unittest's failure exit code.
  return Py_BytesMain(argc, argv);
}
