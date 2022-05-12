//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_vtk_io_vtkMedReader_h
#define smtk_vtk_io_vtkMedReader_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
#include "vtkMultiBlockDataSetAlgorithm.h"

class HdfNode;

// vtkMedReader reads N meshes from a med file providing all the geometry
// per cell type as well as all the groups separately
// The output has 3 tiers of vtkMultiBlockDataSets
// The output vtkMultiBlockDataSet
// One block with a vtkMultiBlockDataSet for each mesh in the med file.
// Each mesh block then contains 2 vtkMultiBlockDataSet blocks where:
// Block 0 contains a vtkMultiBlockDataSet of vtkUnstructuredGrids for the entire geometry per cell type
// Block 1 contains a vtkMultiBlockDataSet of vtkUnstructuredGrids for each group
class SMTKIOVTK_EXPORT vtkMedReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkMedReader* New();
  vtkTypeMacro(vtkMedReader, vtkMultiBlockDataSetAlgorithm);

  vtkMedReader(const vtkMedReader&) = delete;
  vtkMedReader& operator=(const vtkMedReader&) = delete;

public:
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
    * The file to open and read.
    */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

protected:
  vtkMedReader() { this->FileName = nullptr; }
  ~vtkMedReader() override { delete[] this->FileName; }

  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVec,
    vtkInformationVector* outputVec) override;

  static void Cleanup(int64_t fileId, HdfNode* node);

  int FillInputPortInformation(int port, vtkInformation* info) override;

  char* FileName;
};

#endif
