//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_vtk_io_vtkMedWriter_h
#define smtk_vtk_io_vtkMedWriter_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
#include "vtkMultiBlockDataSetAlgorithm.h"

#include <string>

// Before vtk 9.2, there was no VTK_FILEPATH hint:
#ifndef VTK_FILEPATH
#define VTK_FILEPATH
#endif

class vtkMultiBlockDataSet;

// vtkMedWriter writes N meshes to a med file when provided with a valid
// vtkMultiBlockDataSet of vtkUnstructuredGrids
// per cell type as well as all the groups separately
// These groups come in a vtkMultiBlockDataSet as vtkUnstructuredGrids.
class SMTKIOVTK_EXPORT vtkMedWriter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkMedWriter* New();
  vtkTypeMacro(vtkMedWriter, vtkMultiBlockDataSetAlgorithm);

  vtkMedWriter(const vtkMedWriter&) = delete;
  vtkMedWriter& operator=(const vtkMedWriter&) = delete;

public:
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
    * The file to write to.
    */
  void SetFileName(VTK_FILEPATH const std::string& filename);
  VTK_FILEPATH const std::string& GetFileName() const;
  //@}

protected:
  vtkMedWriter()
  {
    SetNumberOfOutputPorts(0);
    SetNumberOfInputPorts(1);
  }

  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVec,
    vtkInformationVector* outputVec) override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  std::string FileName;
};

#endif
