//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBIncorporateMeshOperator -Incorporate solid meshes into a
//       CMB model.
// .SECTION Description

#ifndef __vtkCMBIncorporateMeshOperator_h
#define __vtkCMBIncorporateMeshOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"
#include <set>
#include <map>

class DiscreteMesh;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkAlgorithm;
class vtkDiscreteModel;
class vtkDiscreteModelRegion;
class vtkIdList;
class vtkDiscreteModelFace;

class SMTKDISCRETESESSION_EXPORT vtkCMBIncorporateMeshOperator : public vtkObject
{
public:
  static vtkCMBIncorporateMeshOperator * New();
  vtkTypeMacro(vtkCMBIncorporateMeshOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reads in the file assuming we're on the server.
  // Sets OperateSucceeded.
  void Operate(vtkDiscreteModelWrapper* modelWrapper);

  //Description:
  //Add solid meshes
  void AddSolidMesh(vtkIdType meshRegionId,
    vtkPolyData* meshRegionSurface);
  void AddSolidMesh(vtkIdType meshRegionId,
    vtkAlgorithm* algOut);

  //Description:
  //Remove all solid meshes
  void ClearSolidMeshes();

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBIncorporateMeshOperator();
  virtual ~vtkCMBIncorporateMeshOperator();

  bool IncorporateSolidMesh(
    vtkDiscreteModel* model,
    const vtkIdType& solidFile,
    vtkPolyData* solidRegionSurface);
  bool SplitMeshRegion(
    vtkDiscreteModelRegion* meshRegion,
    vtkDiscreteModel* solidModel,
    const std::string& solidFile);
  bool CreateNewMeshRegions( vtkDiscreteModel* solidModel,
    vtkDiscreteModelRegion* meshRegion,
    std::map<vtkDiscreteModelRegion*, std::set<vtkDiscreteModelFace*> >
     &SolidRegionMeshFacesMap,
    const std::string& solidFile,
    std::map<vtkIdType, vtkIdType> &SolidToMeshPointMap,
    std::set<vtkDiscreteModelFace*>& UsedSolidFaces);

  vtkIdType FindPointsCell(const DiscreteMesh *targetMesh, vtkIdType ptId,
    vtkPolyData* soucePoly, vtkIdList *points,
    vtkIdList* visitedCellMask,
    std::map<vtkIdType, vtkIdType> &SolidToMeshPointMap);
  bool IsSameCellPoints(
    const DiscreteMesh* targetMesh, vtkIdList* tpts,
    vtkPolyData* soucePoly, vtkIdList* spts,
    std::map<vtkIdType, vtkIdType> &SolidToMeshPointMap);
  void AddSolidFaceCells(
    vtkDiscreteModel* meshModel, vtkDiscreteModelFace* faceEntity,
    vtkIdList* newCellIds,
    std::map<vtkIdType, vtkIdType> &SolidToMeshPointMap);

private:
  // Description:
  // map for the solid meshes
  std::map<vtkIdType, vtkPolyData*> SolidMeshes;

  vtkCMBIncorporateMeshOperator(const vtkCMBIncorporateMeshOperator&);  // Not implemented.
  void operator=(const vtkCMBIncorporateMeshOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
