/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCMBIncorporateMeshOperator -Incorporate solid meshes into a
//       CMB model.
// .SECTION Description

#ifndef __vtkCMBIncorporateMeshOperator_h
#define __vtkCMBIncorporateMeshOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <set>
#include <map>
#include "cmbSystemConfig.h"

class DiscreteMesh;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkAlgorithm;
class vtkDiscreteModel;
class vtkDiscreteModelRegion;
class vtkIdList;
class vtkDiscreteModelFace;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBIncorporateMeshOperator : public vtkObject
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
