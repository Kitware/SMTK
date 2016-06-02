//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCUBITReader - Reader for CUBIT facet files
// .SECTION Description
// Reader for *.fac files.  The format is simple, conatining only points
// followed by facets (traingles and/or quads).

#ifndef __smtk_vtk_CUBITReader_h
#define __smtk_vtk_CUBITReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKSMTKREADEREXT_EXPORT vtkCUBITReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCUBITReader *New();
  vtkTypeMacro(vtkCUBITReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCUBITReader();
  ~vtkCUBITReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  char *FileName;

  // Description:
  // Get next line of data (and put in lineStream); skips over comments or blank lines
  int GetNextLineOfData(ifstream &fin, std::stringstream &lineStream);

private:
  vtkCUBITReader(const vtkCUBITReader&);  // Not implemented.
  void operator=(const vtkCUBITReader&);  // Not implemented.
};

#endif
