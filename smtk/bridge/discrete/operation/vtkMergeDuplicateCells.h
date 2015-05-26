//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeDuplicateCells - Filter to merge duplicate cells.
// .SECTION Description
// Filter to merge duplicated cells of a vtkPolyData.  The cells are
// considered duplicated if they are constructed from the same points.
// The deleted cell is the one with the higher RegionArray cell data
// value to hopefully maintain consistent normals.  The outputted
// vtkPolyData has vtkCellData that associates each mesh facet
// with a model face based on model topology.

#ifndef __smtkdiscrete_vtkMergeDuplicateCells_h
#define __smtkdiscrete_vtkMergeDuplicateCells_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkCell;
class vtkIdTypeArray;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkMergeDuplicateCells : public vtkPolyDataAlgorithm
{
public:
  static vtkMergeDuplicateCells* New();
  vtkTypeMacro(vtkMergeDuplicateCells, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(ModelRegionArrayName);
  vtkGetStringMacro(ModelRegionArrayName);

  vtkSetStringMacro(ModelFaceArrayName);
  vtkGetStringMacro(ModelFaceArrayName);

  // Description:
  // Given a ModelFaceId, set the corresponding ModelRegion0Id and
  // ModelRegion1Id values.  Returns false if ModelFaceId was not found.
  // This is currently being done as a reverse lookup in a map which
  // is obviously slow for a large amount of items in the map.
  bool GetModelFaceRegions(vtkIdType ModelFaceId, vtkIdType & ModelRegion0Id,
                           vtkIdType & ModelRegion1Id);

  // Description:
  // Whether or not to pass through the vtkPointData vtkIdTypeArray named
  // "vtkOriginalPointIds" from the input to the output.
  vtkSetMacro(PassThroughPointIds,int);
  vtkGetMacro(PassThroughPointIds,int);
  vtkBooleanMacro(PassThroughPointIds,int);

  // Description:
  // Check whether the input polyData has duplicated cells.
  static bool HasDuplicateCells(vtkPolyData* polyData);

protected:
  vtkMergeDuplicateCells();
  ~vtkMergeDuplicateCells();

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  /// Implementation of the algorithm.
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  // Description:
  // Insert a vtkCell into the outputted vtkPolyData based on the region
  // ids for both sides of the cell (-1 indicates no region).  Also
  // fill in the ModelFaceIdArray.
  void InsertCell(vtkPolyData *input, vtkIdType cellId, vtkPolyData* output,
    vtkIdType Region0, vtkIdType Region1, vtkIdTypeArray* ModelFaceIdArray);

  // Description:
  // Fill the Cell info based on the region
  // ids for both sides of the cell (-1 indicates no region).  Also
  // fill in the ModelFaceIdArray.
  void FillCellFaceInfo(vtkIdType Region0,
                  vtkIdType Region1, vtkIdTypeArray* ModelFaceIdArray);

  // Description:
  // Set the region info field data in Poly.
  void SetModelFaceRegionInfo(vtkPolyData* output);

  // Description:
  // Given two model region Ids, return a consistent model face id.
  vtkIdType GetModelFaceId(int ModelRegionId0, int ModelRegionId1);

//BTX

private:
  vtkMergeDuplicateCells(const vtkMergeDuplicateCells&); // Not implemented.
  void operator=(const vtkMergeDuplicateCells&); // Not implemented.

  // Description:
  // The name of the vtkIntArray that specifies what region each mesh facet/cell
  // is associated with in the input to the filter.
  char *ModelRegionArrayName;

  // Description:
  // The name of the vtkCellData vtkIdTypeArray used to store the
  // model face that each mesh facet/cell is assigned to.
  char* ModelFaceArrayName;

  // Description:
  // If PassThroughPointIds is nonzero, the filter will pass through
  // the vtkPointData vtkIdTypeArray named "vtkOriginalPointIds"
  // from the input to the output.
  int PassThroughPointIds;

  class vtkInternal;
  vtkInternal* Internal;
//ETX

};

#endif


