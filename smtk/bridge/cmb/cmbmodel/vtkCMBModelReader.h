//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelReader -
// .SECTION Description
// Front end for the readers.  Reads in a vtkPolyData and then figures
// out how to parse that vtkPolyData.

#ifndef __smtkcmb_vtkCMBModelReader_h
#define __smtkcmb_vtkCMBModelReader_h

#include "smtk/bridge/cmb/cmbBridgeExports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"


class SMTKCMBBRIDGE_EXPORT vtkCMBModelReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBModelReader * New();
  vtkTypeMacro(vtkCMBModelReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCMBModelReader();
  virtual ~vtkCMBModelReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);


private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBModelReader(const vtkCMBModelReader&);  // Not implemented.
  void operator=(const vtkCMBModelReader&);  // Not implemented.
};

#endif
