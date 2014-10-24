//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelWriterClient - Wrapper on the client to write out a CMB file on the server.
// .SECTION Description
// Wrapper operator on the client to write out a CMB file on the server.
// It currently gets written
// out as a vtkPolyData using vtkXMLPolyDataWriter with the necessary
// information included in the field data.

#ifndef __vtkCMBModelWriterClient_h
#define __vtkCMBModelWriterClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelWriterClient : public vtkObject
{
public:
  static vtkCMBModelWriterClient * New();
  vtkTypeMacro(vtkCMBModelWriterClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/get the version of the file to be written.  Currently
  // this is ignored as there is only a single version.
  vtkSetMacro(Version, int);
  vtkGetMacro(Version, int);

protected:
  vtkCMBModelWriterClient();
  virtual ~vtkCMBModelWriterClient();

private:
  vtkCMBModelWriterClient(const vtkCMBModelWriterClient&);  // Not implemented.
  void operator=(const vtkCMBModelWriterClient&);  // Not implemented.

  // Description:
  // The name of the file to be written.
  char* FileName;

  // Description:
  // The version of the file.
  int Version;
};

#endif
