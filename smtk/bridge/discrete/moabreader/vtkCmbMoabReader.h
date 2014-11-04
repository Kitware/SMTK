//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtkdiscrete_vtkCmbMoabReader_h
#define __smtkdiscrete_vtkCmbMoabReader_h

#include "vtkPolyDataAlgorithm.h"
#include "smtk/bridge/discrete/moabreader/discreteMoabExports.h"

class vtkInformation;
class vtkInformationVector;

namespace smoab{ class Tag; }

class VTKDISCRETEMOABREADER_EXPORT vtkCmbMoabReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCmbMoabReader *New();
  vtkTypeMacro(vtkCmbMoabReader,vtkPolyDataAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCmbMoabReader();
  ~vtkCmbMoabReader();

  int RequestData(vtkInformation *vtkNotUsed(request),
                  vtkInformationVector **vtkNotUsed(inputVector),
                  vtkInformationVector *outputVector);
private:
  char *FileName;

  vtkCmbMoabReader(const vtkCmbMoabReader&);  // Not implemented.
  void operator=(const vtkCmbMoabReader&);  // Not implemented.

};

#endif // __vtkCmbMoabReader_h
