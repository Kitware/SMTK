/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
// .NAME vtkCMBParserBase -
// .SECTION Description
// Abstract base class for parsing a vtkPolyData to create a vtkDiscreteModel.

#ifndef __smtkcmb_vtkCMBParserBase_h
#define __smtkcmb_vtkCMBParserBase_h

#include "vtkSMTKCMBModelModule.h" // For export macro
#include "vtkObject.h"
#include "vtkSmartPointer.h" //needed for classification
#include <map> //needed for classification


class vtkCharArray;
class vtkDiscreteModel;
class vtkDiscreteModelGeometricEntity;
class vtkDataArray;
class vtkIdList;
class vtkIdTypeArray;
class vtkModel;
class vtkModelEntity;
class vtkPolyData;

class VTKSMTKCMBMODEL_EXPORT vtkCMBParserBase : public vtkObject
{
public:
  vtkTypeMacro(vtkCMBParserBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model) = 0;

  // Description:
  // Functions get string names used to store cell/field data.
  static const char* GetModelFaceTagName() {return "modelfaceids";};
  static const char* GetCellClassificationName() {return "CellClassification";};
  static const char* GetShellTagName() {return "Region";};
  static const char* GetMaterialTagName() {return "cell materials";};
  static const char* GetReverseClassificationTagName() {return "reverse classification";};
  static const char* GetSplitFacesTagName() {return "splitfaces";};
  static const char* GetModelFaceDataName() {return "modelfacedata";};
  static const char* GetModelPredefinedBoundarySets() {return "boundarysets";};
  static const char* GetModelPredefinedDomainSets() {return "domainsets";};
  static const char* GetModelFaceConvenientArrayName()
  {
    // convenient model face data array stored at block 0, includes four components
    // shell, materials, model face id, NOT USED (previously it is block index, should be removed)
    // only needed for parser V1
    return "modelfaceconvenientarray";
  };
  static const char* GetVertexPointIdString() {return "vertex point id";};
  static const char* GetVertexLocationString() {return "vertex location";};
  static const char* GetFloatingEdgesString() {return "ModelFloatingEdgeRegions";};
  static const char* GetEdgeLineResolutionString() {return "ModelEdgeLineResolution";};
  static const char* GetMaterialUserNamesString() {return "material names";};
  static const char* GetShellUserNamesString() {return "shell names";};
  static const char* GetShellTranslationPointString() {return "shell translation points";};
  static const char* GetModelFaceUserNamesString() {return "model face names";};
  static const char* GetBCSUserNamesString() {return "BCS names";};
  static const char* GetShellColorsString() {return "shell colors";};
  static const char* GetMaterialColorsString() {return "material colors";};
  static const char* GetBCSColorsString() {return "BCS colors";};
  static const char* GetModelFaceColorsString() {return "model face colors";};
  static const char* GetFileVersionString() {return "version";};
  static const char* GetShellMaterialIdString() {return "shell material ids";};
  static const char* GetFaceMaterialIdString() {return "face material ids";};
  static const char* GetMaterialUserIdsString() {return "material user ids";};
  static const char* GetModelFaceUse1String() {return "ModelFaceUse1";};
  static const char* GetModelFaceUserIdsString() {return "model face user ids";};
  static const char* GetShellUserIdsString() {return "shell user ids";};
  static const char* GetBCSUserIdsString() {return "BCS ids";};
  static const char* GetModelFaceRegionsString() {return "ModelFaceRegions";};
  static const char* GetModelFaceEdgesString() {return "ModelFaceEdges";};
  static const char* GetModelEdgeDirectionsString() {return "edgedirections";};
  static const char* GetModelFaceRegionsMapString() {return "ModelFaceRegionsMap";};

  static const char* GetModelUUIDsString() {return "ModelUUIDs";};
  static const char* GetMaterialUUIDsString() {return "MaterialUUIDs";};
  static const char* GetModelGroupUUIDsString() {return "ModelGroupUUIDs";};
  static const char* GetModelRegionUUIDsString() {return "ModelRegionUUIDs";};
  static const char* GetModelFaceUUIDsString() {return "ModelFaceUUIDs";};
  static const char* GetModelEdgeUUIDsString() {return "ModelEdgeUUIDs";};
  static const char* GetModelVertexUUIDsString() {return "ModelVertexUUIDs";};
  static const char* GetModelFaceUseUUIDsString() {return "ModelFaceUseUUIDs";};
  static const char* GetModelEdgeUseUUIDsString() {return "ModelEdgeUseUUIDs";};
  static const char* GetModelVertexUseUUIDsString() {return "ModelVertexUseUUIDs";};
  static const char* GetModelShellUUIDsString() {return "ModelShellUUIDs";};
  static const char* GetModelLoopUUIDsString() {return "ModelLoopUUIDs";};
  static const char* GetModelChainUUIDsString() {return "ModelChainUUIDs";};
  static const char* GetMaterialUniquePersistentIdsString() {return "MaterialIds";};
  static const char* GetModelRegionUniquePersistentIdsString() {return "ModelRegionIds";};
  static const char* GetModelFaceUniquePersistentIdsString() {return "ModelFaceIds";};
  static const char* GetModelEdgeUniquePersistentIdsString() {return "ModelEdgeIds";};
  static const char* GetModelVertexUniquePersistentIdsString() {return "ModelVertexIds";};
  static const char* GetModelEdgeVerticesString() {return "ModelEdgeEndPoints";};
  static const char* GetModelRegionsMaterialsString() {return "ModelRegionMaterials";};
  static const char* GetModelRegionPointInsideString() {return "ModelRegionPointInside";};
  static const char* GetModelRegionPointInsideValidity() {return "ModelRegionPointInsideValidity";};

  static const char* GetBCNodalGroupDataString() {return "BCNodalGroupData";};
  static const char* GetBCFloatingEdgeDataString() {return "BCFloatingEdgeData";};
  static const char* GetBCModelFaceDataString() {return "BCModelFaceData";};
  static const char* GetAnalysisGridFileName() {return "AnalysisGridName";};
  static const char* GetAnalysisGridFileType() {return "AnalysisGridType";};

  // Description:
  // Function to set the geometry of the model so that every parser
  // does not need to be specified as a friend class of vtkDiscreteModel.
  void SetGeometry(vtkDiscreteModel* Model, vtkObject* Geometry);

  // Description:
  // Function to set the cells of the model geometric entity so that
  // every parser does not need to be specified as a friend class of vtkDiscreteModel.
  bool AddCellsToGeometry(vtkDiscreteModelGeometricEntity* Entity,
                           vtkIdList* MasterCellIds);

  // Description:
  // Function to set the unique persistent Id of an entity so that every parser
  // does not need to be specified as a friend class of vtkModelEntity.
  void SetUniquePersistentId(vtkModelEntity* Entity, vtkIdType Id);

  // Description:
  // Function to set the MaxId of all entities in the model so that every parser
  // does not need to be specified as a friend class of vtkModel.
  // It checks that MaxId is larger than the current value.
  void SetLargestUsedUniqueId(vtkModel* Model, vtkIdType MaxId);

  // Description:
  // Function to output vtkIdTypeArray given a data array.  This is needed
  // since the array may be read in as an vtkIntArray (they are essentially the
  // same on 32 bit machines) but cannot be cast to a vtkIdTypeArray.
  // If Array is not a vtkIdTypeArray, it will allocate a new array
  // which the caller must delete.  Otherwise it will increase the reference
  // count to the original array such that the caller must delete the array as well.
  vtkIdTypeArray* NewIdTypeArray(vtkDataArray* Array);

  // Description:
  // Function to set the information for mapping from the model grid to the
  // analysis grid.  It assumes the boundary topology is the same and that we
  // have the full mapping information.  We are
  // unsure what type of array we are getting from the reader
  // so we will convert it internally to a vtkIdTypeArray.
  void SetAnalysisGridInfo(
    vtkDiscreteModel* model, vtkDataArray* pointMapArray, vtkDataArray* cellMapArray,
    vtkCharArray* canonicalSideArray);

protected:
  vtkCMBParserBase();
  virtual ~vtkCMBParserBase();

  //Description:
  // Convert a flat cell Id space where some cells represent edges
  // or faces, and return a mapping from model item id to cell or edge ids
  typedef std::map<vtkIdType, vtkSmartPointer<vtkIdList> > CellToModelType;
  typedef CellToModelType::iterator CellToModelIterator;
  void SeparateCellClassification(vtkDiscreteModel* model,
                                  vtkIdTypeArray* cellClassification,
                                   vtkCMBParserBase::CellToModelType& cellToModelMap) const;
private:
  vtkCMBParserBase(const vtkCMBParserBase&);  // Not implemented.
  void operator=(const vtkCMBParserBase&);  // Not implemented.
};

#endif
