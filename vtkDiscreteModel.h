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
// .NAME vtkDiscreteModel - CMB's implementation of a vtkModel.
// .SECTION Description
// The implementation of vtkModel for discrete models.
// The server and the client
// will both have a vtkDiscreteModel with the one on the server having
// vtkPolyDatas for GEOMETRY, otherwise they are identical.  Use
// vtkCMBModelWrapper to access the model on the server from the
// client.

#ifndef __vtkDiscreteModel_h
#define __vtkDiscreteModel_h

#include "Model/vtkModel.h"

class vtkCharArray;
class vtkCmbGridRepresentation;
class vtkCMBMaterial;
class vtkCMBModelEdge;
class vtkCMBModelEntity;
class vtkCMBModelEntityGroup;
class vtkCMBModelFace;
class vtkCMBModelGeometricEntity;
class vtkCMBNodalGroup;
class vtkCMBUniqueNodalGroup;
class vtkIdTypeArray;
class vtkInformationDataObjectKey;
class vtkIntArray;
class vtkModelVertex;
class vtkModelVertexUse;

//BTX
#include <string>
enum vtkCMBModelEntityTypes
{
  vtkCMBModelEntityGroupType = 100,
  vtkCMBMaterialType,
  vtkCMBNodalGroupType
};

// Description:
// All the currently defined CMB Model events are listed here.
enum CMBModelEventIds {
  ModelEntityGroupCreated = 10000,
  ModelEntityGroupAboutToDestroy,
  ModelEntityGroupDestroyed,
  NodalGroupCreated,
  NodalGroupAboutToDestroy,
  NodalGroupDestroyed,
  DomainSetCreated,
  DomainSetAboutToDestroy,
  DomainSetDestroyed
};
//ETX

class VTK_EXPORT vtkDiscreteModel : public vtkModel
{
public:
  static vtkDiscreteModel* New();

//BTX
  vtkTypeRevisionMacro(vtkDiscreteModel,vtkModel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Functions to get the vtkCMBModelGeometricEntity object that CellId
  // is classified on and the cell index of the copy of CellId in that
  // vtkPolyData.  Note that these should only be called on the server.
  vtkCMBModelGeometricEntity* GetCellModelGeometricEntity(vtkIdType CellId);
  vtkIdType GetCellModelGeometricEntityIndex(vtkIdType CellId);

  // Description:
  // Get the geometric representation of the model. Returns null on the
  // client.
  vtkObject* GetGeometry();

  // Description:
  // Get the name of the array that stores the point mapping information.
  // We want to rename it so that we have a unique name for it.
  static const char* GetPointMapArrayName();

  // Description:
  // Get the name of the arrays that stores the cell mapping information.
  // We want to rename it so that we have a unique name for it.
  static const char* GetCellMapArrayName();
  static const char* GetCanonicalSideArrayName();


  // Description:
  // Function to clear out the current model data.  This must be called
  // in order for a model to be deleted since this removes the
  // associations to the model of objects aggregated by the model.
  // NOTE: This function should be called before call Delete, otherwise,
  // there might be leaks due to model associations.
  virtual void Reset();

  // Description:
  // Build a model entity. The model is responsible for the management
  // of the built model entity.  The build model entity is deleted
  // from the model with the corresponding DestroyModel function.
  virtual vtkModelVertex* BuildModelVertex(vtkIdType PointId);
  virtual vtkModelVertex* BuildModelVertex(vtkIdType PointId,
                                           vtkIdType VertexId);
  virtual vtkModelEdge* BuildModelEdge(vtkModelVertex* Vertex0,
                                       vtkModelVertex* Vertex1);

  virtual vtkModelEdge* BuildModelEdge(
    vtkModelVertex* Vertex0, vtkModelVertex* Vertex1, vtkIdType EdgeId);

  virtual vtkModelFace* BuildModelFace(int NumEdges, vtkModelEdge** Edges,
                                       int* EdgeDirections, vtkCMBMaterial* Material);
  virtual vtkModelFace* BuildModelFace(int NumEdges, vtkModelEdge** Edges,
                                       int* EdgeDirections);
  virtual vtkModelFace* BuildModelFace(int NumEdges, vtkModelEdge** Edges,
                                       int* EdgeDirections,
                                       vtkIdType ModelFaceId);
  virtual vtkModelRegion* BuildModelRegion();
  virtual vtkModelRegion* BuildModelRegion(vtkIdType ModelRegionId);
  virtual vtkModelRegion* BuildModelRegion(
    int NumFaces, vtkModelFace** Faces, int* FaceSides);
  virtual vtkModelRegion* BuildModelRegion(
    int NumFaces, vtkModelFace** Faces, int* FaceSides,
    vtkIdType ModelRegionId);
  virtual vtkModelRegion* BuildModelRegion(
    int NumFaces, vtkModelFace** Faces, int* FaceSides,
    vtkCMBMaterial* Material);
  virtual vtkModelRegion* BuildModelRegion(
    int NumFaces, vtkModelFace** Faces, int* FaceSides,
    vtkIdType ModelRegionId, vtkCMBMaterial* Material);

  virtual bool DestroyModelGeometricEntity(
    vtkCMBModelGeometricEntity* GeomEntity);

  virtual void GetBounds(double bounds[6]);

  virtual vtkCMBMaterial* BuildMaterial();
  virtual vtkCMBMaterial* BuildMaterial(vtkIdType Id);

  // Description:
  // Remove an existing material from the model.  Note that no model entities
  // should be associated with this material.
  virtual bool DestroyMaterial(vtkCMBMaterial* Material);

  // Description:
  // Build a vtkCMBModelEntityGroup and initialize it with some some objects.
  virtual vtkCMBModelEntityGroup* BuildModelEntityGroup(
    int itemType, int NumEntities, vtkCMBModelEntity** Entities);
  virtual vtkCMBModelEntityGroup* BuildModelEntityGroup(
    int itemType, int NumEntities, vtkCMBModelEntity** Entities, vtkIdType Id);

  // Description:
  // Destroy EntityGroup.  Returns true if successful.
  virtual bool DestroyModelEntityGroup(vtkCMBModelEntityGroup* EntityGroup);

  // Description:
  // Build a vtkCMBNodalGroup and populate it with the vtkPoint Ids
  // in PointIds. If Type is 0/BASE_NODAL_GROUP a vtkCMBNodalGroup will
  // be created and if Type is 1/UNIQUE_NODAL_GROUP a vtkCMBUniqueNodalGroup
  // will be created. Id is the unique persistent Id to be assigned
  // to the nodal group.
  virtual vtkCMBNodalGroup* BuildNodalGroup(int Type, vtkIdList* PointIds);
  virtual vtkCMBNodalGroup* BuildNodalGroup(int Type, vtkIdList* PointIds,
                                            vtkIdType Id);

  // Description:
  // Destroy EntityGroup.  Returns true if successful.
  virtual bool DestroyNodalGroup(vtkCMBNodalGroup* NodalGroup);

  // Description:
  // Build/Destroy a floating vtkCMBModelEdge.
  virtual vtkModelEdge* BuildFloatingRegionEdge(
    double point1[3], double point2[3],
    int resolution, vtkIdType RegionId)
    {return this->BuildFloatingRegionEdge(this->GetNextUniquePersistentId(),
      point1, point2, resolution, RegionId);}
  virtual vtkModelEdge* BuildFloatingRegionEdge(vtkIdType edgeId,
    double point1[3], double point2[3],
    int resolution, vtkIdType RegionId);
  virtual bool DestroyModelEdge(vtkCMBModelEdge* ModelEdge);

  vtkGetMacro(AnalysisGridInfo, vtkCmbGridRepresentation*);
  void SetAnalysisGridInfo(vtkCmbGridRepresentation*);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Static function for getting the information object
  // used to (optionally) store the mapping from
  // entities in this grid to another grid.
  static vtkInformationObjectBaseKey* POINTMAPARRAY();
  static vtkInformationObjectBaseKey* CELLMAPARRAY();

  // Description:
  // Flag to whether invoke event
  vtkSetMacro(BlockEvent, bool);
  vtkGetMacro(BlockEvent, bool);
  vtkBooleanMacro(BlockEvent, bool);

protected:
  vtkDiscreteModel();
  virtual ~vtkDiscreteModel();

  // Description:
  // Function to get the next default name for a model entity.  It
  // is used for giving a default value when creating a model entity
  // that the user can change.
  void GetModelEntityDefaultName(int EntityType, const char* BaseName,
                                 std::string & DefaultEntityName);

  // Description:
  // Function to set the vtkCMBModelGeometricEntity object and Geometric
  // Entity grid Id (GeomeEntityCellId) that MasterCellId is classified on.
  // This does not set the reverse classification information though.
  // Note that this should only be called on the server as it won't do
  // anything on the client.
  void SetCellClassification(vtkIdType MasterCellId,vtkIdType GeomEntityCellId,
                             vtkCMBModelGeometricEntity* GeomEntity);
//BTX
  friend class vtkCMBModelGeometricEntity;
  friend class vtkCMBParserBase;
  friend class vtkCMBModelFace;
  friend class vtkCMBModelWrapper;
  friend class vtkCMBModelBuilder;
  friend class vtkCMBModelEdge;
  friend class CmbSceneBuilderCore;
  friend class CmbGeologyBuilderCore;
  friend class vtkCmbMapToCmbModel;
  friend class vtkCMBUniqueNodalGroup;
  friend class vtkCmbIncorporateMeshOperator;
//ETX

  // Description:
  // Set/get the vtkCMBUniqueNodalGroups that a point is assigned to.
  // For setting, it removes the point from its current vtkCMBUniqueNodalGroups
  // if it belongs to one.
  void SetPointUniqueNodalGroup(vtkCMBUniqueNodalGroup* NodalGroup,
                                vtkIdType PointId);
  vtkCMBUniqueNodalGroup* GetPointUniqueNodalGroup(vtkIdType PointId);

  // Description:
  // Set the vtkObject that is used to represent the Geometry.  This should
  // only be called on the server. Note: Sometimes UpdateGeometry() needs to
  // be called if the master polydata is modified.
  void SetGeometry(vtkObject* Geometry);
  void UpdateGeometry();

  // Description:
  // The mappings from a cell on the master geometry to the geometric model
  // entity it is classified on (CellClassification) as well as the index
  // of the cell on the geometric model entity geometric representation.
  std::vector<vtkCMBModelGeometricEntity*> CellClassification;
  std::vector<vtkIdType> ClassifiedCellIndex;

  // Description:
  // The vector of vtkCMBUniqueNodalGroups that grid points are assigned to.
  // A point can be assigned to at most one vtkCMBUniqueNodalGroups but
  // is not required to be assigned to any.
  std::vector<vtkCMBUniqueNodalGroup*> UniquePointGroup;

  // Description:
  // The bounds of the model; set (on the server) when doing a SetGeometry,
  // and then passed to the client during serialization
  double ModelBounds[6];

  // Description:
  // Object to get analysis grid information.  Currently used for
  // model entity and nodal groups.
  vtkCmbGridRepresentation* AnalysisGridInfo;

  // Description:
  // Flag to whether invoke event
  bool BlockEvent;
  void InternalInvokeEvent(unsigned long event, void *callData);

private:
  vtkDiscreteModel(const vtkDiscreteModel&);  // Not implemented.
  void operator=(const vtkDiscreteModel&);  // Not implemented.
//ETX
};

#endif
