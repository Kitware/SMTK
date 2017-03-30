//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModel - CMB's implementation of a vtkModel.
// .SECTION Description
// The implementation of vtkModel for discrete models.
// The server and the client
// will both have a vtkDiscreteModel with the one on the server having
// vtkPolyDatas for GEOMETRY, otherwise they are identical.  Use
// vtkDiscreteModelWrapper to access the model on the server from the
// client.

#ifndef __smtkdiscrete_vtkDiscreteModel_h
#define __smtkdiscrete_vtkDiscreteModel_h

#include "Model/vtkModel.h"
#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro

class vtkCharArray;
class vtkModelGridRepresentation;
class vtkModelMaterial;
class vtkDiscreteModelEdge;
class vtkDiscreteModelEntity;
class vtkDiscreteModelEntityGroup;
class vtkDiscreteModelFace;
class vtkDiscreteModelGeometricEntity;
class vtkIdTypeArray;
class vtkInformationDataObjectKey;
class vtkIntArray;
class vtkModelVertex;
class vtkModelVertexUse;

#include "DiscreteMesh.h"       //needed for Discrete Mesh
#include "MeshClassification.h" //needed for Discrete Mesh Classification

#include <string>

// Description:
// Enumeration for the types of groupings we would like to do.
// This doesn't work well for mixed type domains where a 2D
// and a 3D model entity are both considered domains, resulting
// in boundary entities of dimension 1 and 2.
enum vtkDiscreteModelEntityTypes
{
  vtkDiscreteModelEntityGroupType = 100, // intended for boundaries
  vtkModelMaterialType                   // intended for domains
};

// Description:
// All the currently defined CMB Model events are listed here.
enum DiscreteModelEventIds
{
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

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModel : public vtkModel
{
public:
  typedef MeshClassification<vtkDiscreteModelGeometricEntity> ClassificationType;

  static vtkDiscreteModel* New();

  vtkTypeMacro(vtkDiscreteModel, vtkModel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the geometric representation of the model.
  const DiscreteMesh& GetMesh() const { return this->Mesh; }

  // Description:
  // Get the classification model to the mesh.
  ClassificationType& GetMeshClassification() { return this->MeshClassificationInstance; }

  // Description:
  // Returns true if the DiscreteModel has a non empty Mesh.
  // we define empty to be a NULL pointer OR a mesh with zero points and cells
  bool HasValidMesh() const;

  // Description:
  // Returns false if the DiscreteModel has a non empty Mesh.
  // we define empty to be a NULL pointer OR a mesh with zero points and cells
  bool HasInValidMesh() const;

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
  virtual vtkModelVertex* BuildModelVertex(vtkIdType pointId, bool bCreateGeometry = false);
  virtual vtkModelVertex* BuildModelVertex(
    vtkIdType pointId, vtkIdType vertexId, bool bCreateGeometry = false);
  virtual vtkModelEdge* BuildModelEdge(vtkModelVertex* vertex0, vtkModelVertex* vertex1);

  virtual vtkModelEdge* BuildModelEdge(
    vtkModelVertex* vertex0, vtkModelVertex* vertex1, vtkIdType edgeId);

  virtual vtkModelFace* BuildModelFace(
    int numEdges, vtkModelEdge** edges, int* edgeDirections, vtkModelMaterial* material);
  virtual vtkModelFace* BuildModelFace(int numEdges, vtkModelEdge** edges, int* edgeDirections);
  virtual vtkModelFace* BuildModelFace(
    int NumEdges, vtkModelEdge** Edges, int* edgeDirections, vtkIdType modelFaceId);
  virtual vtkModelRegion* BuildModelRegion();
  virtual vtkModelRegion* BuildModelRegion(vtkIdType modelRegionId);
  virtual vtkModelRegion* BuildModelRegion(int numFaces, vtkModelFace** faces, int* faceSides);
  virtual vtkModelRegion* BuildModelRegion(
    int numFaces, vtkModelFace** faces, int* faceSides, vtkIdType modelRegionId);
  virtual vtkModelRegion* BuildModelRegion(
    int numFaces, vtkModelFace** faces, int* faceSides, vtkModelMaterial* material);
  virtual vtkModelRegion* BuildModelRegion(int numFaces, vtkModelFace** faces, int* faceSides,
    vtkIdType modelRegionId, vtkModelMaterial* material);

  virtual bool DestroyModelGeometricEntity(vtkModelGeometricEntity* geomEntity);

  virtual void GetBounds(double bounds[6]);

  virtual vtkModelMaterial* BuildMaterial();
  virtual vtkModelMaterial* BuildMaterial(vtkIdType id);

  // Description:
  // Remove an existing material from the model.  Note that no model entities
  // should be associated with this material.
  virtual bool DestroyMaterial(vtkModelMaterial* material);

  // Description:
  // Build a vtkDiscreteModelEntityGroup and initialize it with some some objects.
  virtual vtkDiscreteModelEntityGroup* BuildModelEntityGroup(
    int itemType, int numEntities, vtkDiscreteModelEntity** entities);
  virtual vtkDiscreteModelEntityGroup* BuildModelEntityGroup(
    int itemType, int numEntities, vtkDiscreteModelEntity** entities, vtkIdType id);

  // Description:
  // Destroy EntityGroup.  Returns true if successful.
  virtual bool DestroyModelEntityGroup(vtkDiscreteModelEntityGroup* entityGroup);

  // Description:
  // Build/Destroy a floating vtkDiscreteModelEdge.
  virtual vtkModelEdge* BuildFloatingRegionEdge(
    double point1[3], double point2[3], int resolution, vtkIdType regionId)
  {
    return this->BuildFloatingRegionEdge(
      this->GetNextUniquePersistentId(), point1, point2, resolution, regionId);
  }
  virtual vtkModelEdge* BuildFloatingRegionEdge(
    vtkIdType edgeId, double point1[3], double point2[3], int resolution, vtkIdType regionId);
  virtual bool DestroyModelEdge(vtkDiscreteModelEdge* modelEdge);

  vtkGetMacro(AnalysisGridInfo, vtkModelGridRepresentation*);
  void SetAnalysisGridInfo(vtkModelGridRepresentation*);

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

  vtkSetMacro(FileName, vtkStdString);
  vtkGetMacro(FileName, vtkStdString);

protected:
  vtkDiscreteModel();
  virtual ~vtkDiscreteModel();

  // Description:
  // Function to get the next default name for a model entity.  It
  // is used for giving a default value when creating a model entity
  // that the user can change.
  void GetModelEntityDefaultName(
    int entityType, const char* baseName, std::string& defaultEntityName);

  friend class vtkDiscreteModelGeometricEntity;
  friend class vtkCMBParserBase;
  friend class vtkDiscreteModelFace;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCMBModelBuilder;
  friend class vtkDiscreteModelEdge;
  friend class vtkGenerateSimpleModelOperator;
  friend class pqCMBSceneBuilderMainWindowCore;
  friend class pqCMBGeologyBuilderMainWindowCore;
  friend class vtkCMBMapToCMBModel;
  friend class vtkCMBIncorporateMeshOperator;

  // Description:
  // Set the Discrete mesh. This should only be called on the server.
  void SetMesh(DiscreteMesh& mesh);

  // Description:
  // If an operator or model item modify the point set that is attached
  // to the DiscreteMesh it can cause a desync. If that happens you should
  // call UpdateMesh.
  void UpdateMesh();

  // Description:
  // The bounds of the model; set (on the server) when doing a SetGeometry,
  // and then passed to the client during serialization
  double ModelBounds[6];

  // Description:
  // Object to get analysis grid information.  Currently used for
  // model entity and nodal groups.
  vtkModelGridRepresentation* AnalysisGridInfo;

  // Description:
  // Flag to whether invoke event
  bool BlockEvent;
  void InternalInvokeEvent(unsigned long event, void* callData);

  vtkStdString FileName;

private:
  DiscreteMesh Mesh;
  ClassificationType MeshClassificationInstance;

  vtkDiscreteModel(const vtkDiscreteModel&); // Not implemented.
  void operator=(const vtkDiscreteModel&);   // Not implemented.
};

#endif
