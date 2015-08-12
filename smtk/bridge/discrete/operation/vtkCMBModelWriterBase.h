//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelWriterBase - Writes out a CMB file on the server.
// .SECTION Description
// Writes out a CMB file on the server.  It currently gets written
// out as a vtkPolyData using vtkXMLPolyDataWriter with the necessary
// information included in the field data.  It writes out the current
// version unless specified otherwise.

#ifndef __smtkdiscrete_vtkCMBModelWriterBase_h
#define __smtkdiscrete_vtkCMBModelWriterBase_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

namespace smtk {
  namespace bridge {
    namespace discrete {
      class Session;
    }
  }
}

class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;

class SMTKDISCRETESESSION_EXPORT vtkCMBModelWriterBase : public vtkObject
{
public:
  static vtkCMBModelWriterBase * New();
  vtkTypeMacro(vtkCMBModelWriterBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.  This is the operator version of writing.
  // The \a session will be used to retrieve UUIDs data array for entities and
  // add it to field data of output.
  // NOTE: vtkCMBModelWriterV2 has implementation using the discrete \a session
  void Operate(vtkDiscreteModelWrapper* ModelWrapper,
               smtk::bridge::discrete::Session* session);

//BTX
  // Description:
  // Write the CMB file out.
  void Write(vtkDiscreteModel* model,
             smtk::bridge::discrete::Session* session);
//ETX
  // Description:
  // Get/Set the name of the output file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Set/get the version of the file to be written.
  vtkSetMacro(Version, int);
  vtkGetMacro(Version, int);

  // Description:
  // Get the current/most up-to-date version of the writer.
  int GetCurrentVersion();

protected:
  vtkCMBModelWriterBase();
  virtual ~vtkCMBModelWriterBase();

private:
  vtkCMBModelWriterBase(const vtkCMBModelWriterBase&);  // Not implemented.
  void operator=(const vtkCMBModelWriterBase&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  // Description:
  // The version of the file.
  int Version;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
