//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME ModelParserHelper -
// .SECTION Description
// convenient methods to get strings/names for various purpose.

#ifndef __smtkdiscrete_ModelParserHelper_h
#define __smtkdiscrete_ModelParserHelper_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro

class VTKSMTKDISCRETEMODEL_EXPORT ModelParserHelper
{
public:

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
};

#endif
