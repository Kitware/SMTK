//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMapReader - reader for map files
// .SECTION Description
// Reads in vertexes, arcs, and polygons described in map files
// reader based on the filename's extension.

#ifndef __smtk_vtk_vtkCMBMapReader_h
#define __smtk_vtk_vtkCMBMapReader_h

#include "smtk/extension/vtk/reader/vtkSMTKReaderExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkIntArray;

class VTKSMTKREADEREXT_EXPORT vtkCMBMapReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBMapReader* New();
  vtkTypeMacro(vtkCMBMapReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  vtkGetMacro(NumArcs, int);
  vtkIntArray* GetArcIds() { return ArcIds; }

protected:
  vtkCMBMapReader();
  ~vtkCMBMapReader();

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  char* FileName;

  int NumArcs;
  vtkIntArray* ArcIds;

private:
  vtkCMBMapReader(const vtkCMBMapReader&); // Not implemented.
  void operator=(const vtkCMBMapReader&);  // Not implemented.
};

#endif
