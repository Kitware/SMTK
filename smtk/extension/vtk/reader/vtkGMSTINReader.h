//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGMSTINReader - Reader for GMS TIN files
// .SECTION Description
// Reads GMS TIN files (ASCII only). It is assumed that the
// vertex indices start at 0.

#ifndef __smtk_vtk_vtkGMSTINReader_h
#define __smtk_vtk_vtkGMSTINReader_h

#include "smtk/extension/vtk/reader/Exports.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"

class vtkCellArray;
class vtkFloatArray;
class vtkMultiBlockDataSet;
class vtkUnsignedCharArray;
class vtkPolyData;

//BTX
struct vtkGMSTINReaderInternals;
//ETX

class VTKSMTKREADEREXT_EXPORT vtkGMSTINReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkGMSTINReader *New();
  vtkTypeMacro(vtkGMSTINReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkGMSTINReader();
  ~vtkGMSTINReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  int ReadTIN(unsigned int block, vtkMultiBlockDataSet* output);
  void ReadTriangles(vtkCellArray*);
  void ReadVerts(vtkPolyData*);

  char * FileName;

private:
  vtkGMSTINReader(const vtkGMSTINReader&);  // Not implemented.
  void operator=(const vtkGMSTINReader&);  // Not implemented.

  vtkGMSTINReaderInternals* Internals;
};

#endif
