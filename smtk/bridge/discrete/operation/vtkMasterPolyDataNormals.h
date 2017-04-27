//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMasterPolyDataNormals - compute normals pointing out for all shells
// .SECTION Description
// This filter expects vtkPolyData, ordering the point ids making up each cell
// such that the normals are pointing out.  This is done by taking each shell
// individually and passing it through vtkPolyDataNormals (with no splitting or
// computation of point normals, as only interested in cell normals).
// Note: Only Polys are passed through this filter.  Any Verts or Lines on the
// input are removed, and we don't handle Strips.

#ifndef __smtkdiscrete_vtkMasterPolyDataNormals_h
#define __smtkdiscrete_vtkMasterPolyDataNormals_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkIdList;

class SMTKDISCRETESESSION_EXPORT vtkMasterPolyDataNormals : public vtkPolyDataAlgorithm
{
public:
  static vtkMasterPolyDataNormals* New();
  vtkTypeMacro(vtkMasterPolyDataNormals, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkMasterPolyDataNormals();
  ~vtkMasterPolyDataNormals();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

private:
  vtkMasterPolyDataNormals(const vtkMasterPolyDataNormals&); // Not implemented.
  void operator=(const vtkMasterPolyDataNormals&);           // Not implemented.
};

#endif
