//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_vtk_io_vtkSideSetsToScalars_h
#define smtk_vtk_io_vtkSideSetsToScalars_h

#include "smtk/extension/vtk/io/IOVTKExports.h"
#include "vtkMultiBlockDataSetAlgorithm.h"

#include <set>
#include <string>
#include <unordered_map>

// Given two multi blocks, which have the following structure:
// - The first block is a set of vtkUnstructuredGrid, the entire geometry separated by cell type
//  They should contain global ids.
// - They second block is a set of vtkUnstructuredGrid, a vtkUnstructuredGrid per "group", which
//  represents a grouping from the "master" geometry (first block). They should contain global ids.
//  per cell or point and vtkCompositeName for the block.
// - They all share the same points
// This filter then produces, as output, a shallow copy of the first block, with each vtkUnstructuredGrid
// recieving a scalar array that represents the flatten families from the groups.
class SMTKIOVTK_EXPORT vtkSideSetsToScalars : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSideSetsToScalars* New();
  vtkTypeMacro(vtkSideSetsToScalars, vtkMultiBlockDataSetAlgorithm);

  vtkSideSetsToScalars(const vtkSideSetsToScalars&) = delete;
  vtkSideSetsToScalars& operator=(const vtkSideSetsToScalars&) = delete;

  vtkSideSetsToScalars()
  {
    SetNumberOfInputPorts(2);
    SetNumberOfOutputPorts(1);
  }

public:
  //@{
  /**
      * The name of the array in Scalar array to create
      */
  void SetScalarArrayName(const std::string& name);
  const std::string& GetScalarArrayName() const;
  //@}

  /**
      * Given a family id, returns all the group ids of the family
      */
  std::set<vtkIdType> getCellGroupSet(const int familyId) const
  {
    if (familyIdToCellGroupSet.count(familyId) != 0)
    {
      return familyIdToCellGroupSet.at(familyId);
    }
    return std::set<vtkIdType>();
  }

  /**
      * Given a family id, returns all the group ids of the family
      */
  std::set<vtkIdType> getVertexGroupSet(const int familyId) const
  {
    if (familyIdToVertexGroupSet.count(familyId) != 0)
    {
      return familyIdToVertexGroupSet.at(familyId);
    }
    return std::set<vtkIdType>();
  }

  /**
      * Returns the familyId map for cells
      */
  const std::unordered_map<int, std::set<vtkIdType>>& getFamilyIdToCellGroupSetMap() const
  {
    return familyIdToCellGroupSet;
  }

  /**
      * Returns the familyId map for vertices
      */
  const std::unordered_map<int, std::set<vtkIdType>>& getFamilyIdToVertexGroupSetMap() const
  {
    return familyIdToVertexGroupSet;
  }

protected:
  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVec,
    vtkInformationVector* outputVec) override;

private:
  std::string ScalarArrayName = "FAM";
  std::unordered_map<int, std::set<vtkIdType>> familyIdToCellGroupSet;
  std::unordered_map<int, std::set<vtkIdType>> familyIdToVertexGroupSet;
};

#endif
