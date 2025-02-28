//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/Logger.h"

#include "smtk/extension/vtk/io/RedirectOutput.h"

#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkOutputWindow.h>

class vtkMyObject : public vtkObject
{
public:
  static vtkMyObject* New()
  {
    vtkMyObject* obj = new vtkMyObject;
    return obj;
  }

  vtkMyObject() = default;

  ~vtkMyObject() override = default;

  void NormalMessage() { vtkOutputWindowDisplayText("VTK-style normal message!"); }

  void ErrorMessage() { vtkErrorMacro(<< "VTK-style error message!"); }

  void WarningMessage() { vtkWarningMacro(<< "VTK-style warning message!"); }

  static void GenericWarningMessage()
  {
    vtkGenericWarningMacro(<< "VTK-style generic warning message!");
  }

  void DebugMessage() { vtkDebugMacro(<< "VTK-style debug message!"); }
};

int UnitTestRedirectOutput(int /*unused*/, char** const /*unused*/)
{
  vtkNew<vtkMyObject> myVTKObject;
  myVTKObject->DebugOn();

  // First use the default VTK I/O path
  myVTKObject->NormalMessage();
  myVTKObject->ErrorMessage();
  myVTKObject->WarningMessage();
  vtkMyObject::GenericWarningMessage();
  myVTKObject->DebugMessage();

  // Then create a logger and redirect VTK's output to it
  {
    smtk::io::Logger logger;
    smtk::extension::vtk::io::RedirectVTKOutputTo(logger);

    myVTKObject->NormalMessage();
    myVTKObject->ErrorMessage();
    myVTKObject->WarningMessage();
    vtkMyObject::GenericWarningMessage();
    myVTKObject->DebugMessage();

    // Check that the logger received the output
    std::size_t i, n = logger.numberOfRecords();
    std::size_t expected = 4;

#ifndef NDEBUG
    expected++;
#endif

    if (n != expected)
    {
      std::cerr << "Wrong number of records!  Got " << n << " Should be " << expected << "!\n";
      return -1;
    }

    // Print the received output to screen
    smtk::io::Logger::Record r;
    for (i = 0; i < n; i++)
    {
      r = logger.record(static_cast<int>(i));
      std::cerr << " Record " << i
                << ": \n\tSeverity = " << smtk::io::Logger::severityAsString(r.severity)
                << "\n\tMessage = " << r.message << "\tFile = " << r.fileName
                << "\n\tLine = " << r.lineNumber << std::endl;
    }
  }

  // Finally, check that VTK's I/O falls back to the default path when the
  // logger goes out of scope
  myVTKObject->NormalMessage();
  myVTKObject->ErrorMessage();
  myVTKObject->WarningMessage();
  vtkMyObject::GenericWarningMessage();
  myVTKObject->DebugMessage();

  return 0;
}
