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
// .NAME vtkModel - An abstract model interface for geometry.
// .SECTION Description
// The vtkPolyData/vtkSMProxy object that is stored on the
// server/client, respectively in the GEOMETRY() information key.
// Use a SafeDownCast() to determine which one.
// The model also includes events for keeping track of changes to
// the model.  The events are only triggered for operations on
// geometric entities.  Currently the events are created, about to
// be deleted, and boundary modified.  The client data for the event
// call back is a vtkModelGeometricEntity.  For the created and about
// to be deleted events, the model may not be valid but for those
// events the vtkModelGeometricEntity should have all valid
// downward adjacencies set (e.g. a model face will know about its
// adjacent model edges which will know about their model vertices).
// Upwared adjacency queries (e.g. the regions that are adjacent to
// a model face) should not be performed since they are not guaranteed
// to be valid.  For the boundary modified event, the model will be
// in a valid state.  Lower dimensional entities will have their
// boundary modified event triggered before higher dimensional entities.

#ifndef __vtkModel_h
#define __vtkModel_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelItem.h"
#include "cmbSystemConfig.h"
#include <vector>

class vtkModelGeometricEntity;
class vtkModelVertex;
class vtkModelEdge;
class vtkModelEntity;
class vtkModelFace;
class vtkModelRegion;
class vtkIntArray;

enum ModelEntityTypes
{
  vtkModelType = 0,
  vtkModelVertexType,
  vtkModelVertexUseType,
  vtkModelEdgeType,
  vtkModelEdgeUseType,
  vtkModelLoopUseType,
  vtkModelFaceType,
  vtkModelFaceUseType,
  vtkModelShellUseType,
  vtkModelRegionType,
};

// Description:
// All the currently defined Model events are listed here.
enum ModelEventIds {
  ModelGeometricEntityCreated = 11000,
  ModelGeometricEntityBoundaryModified,
  ModelGeometricEntityAboutToDestroy,
  ModelGeometricEntitiesAboutToMerge,
  ModelGeometricEntitySplit, // this event is called after the entity is split
  ModelEntityGeometrySet,
  ModelReset
};

class VTKSMTKDISCRETEMODEL_EXPORT vtkModel : public vtkModelItem
{
public:
  vtkTypeMacro(vtkModel,vtkModelItem);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Build a model entity. The model is responsible for the management
  // of the built model entity.  The build model entity is deleted
  // from the model with the corresponding DestroyModel function.
  // Since the New() functions won't have enough information about
  // the specific model entities being built to get them to a valid
  // state, the implementations of these methods will have to do
  // all of the operations necessary to get them into a valid state
  // (e.g. create a valid model shell use for a model region).
  virtual vtkModelEdge* BuildModelEdge(vtkModelVertex* vertex0,
                                       vtkModelVertex* vertex1) = 0;
  virtual vtkModelFace* BuildModelFace(int numEdges, vtkModelEdge** edges,
                                       int* edgeDirections) = 0;
  virtual vtkModelRegion* BuildModelRegion(int numFaces, vtkModelFace** faces,
                                           int* faceSides) = 0;
  virtual vtkModelRegion* BuildModelRegion() = 0;

  // Description:
  // Functions for accessing the number of objects of a certain type.
  // This only gives non-zero results for objects that a model
  // explicitly aggregates.
  int GetNumberOfModelEntities(int itemType);
  int GetNumberOfGeometricEntities();

  // Description:
  // Function for getting a model entity from its unique persistent id.
  vtkModelEntity* GetModelEntity(vtkIdType uniquePersistentId);

  // Description:
  // Function for getting a model entity from its unique persistent id.  It
  // only looks for this entity in the given entity type to make the search
  // more efficient.
  vtkModelEntity* GetModelEntity(int itemType, vtkIdType uniquePersistentId);

  // Description:
  // Destroy a model entity if possible.  A situation where a
  // model entity cannot be destroyed is when it is "used" by
  // a higher order entity that has not yet been deleted.
  // Returns true if the model entity was able to be destroyed.
  virtual bool DestroyModelGeometricEntity(vtkModelGeometricEntity* entity);

  // Description:
  // Function to clear out the current model data.  This must be called
  // in order for a model to be deleted since this removes the
  // associations to the model of objects aggregated by the model.
  virtual void Reset();

  virtual void GetBounds(double bounds[6]) = 0;

  virtual int GetType();

  virtual int GetModelDimension();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Set LargestUsedUniqueId.
  vtkGetMacro(LargestUsedUniqueId, vtkIdType);

  // Description:
  // Flag to whether invoke event
  vtkSetMacro(BlockModelGeometricEntityEvent, bool);
  vtkGetMacro(BlockModelGeometricEntityEvent, bool);
  vtkBooleanMacro(BlockModelGeometricEntityEvent, bool);

protected:
  vtkModel();
  virtual ~vtkModel();

  // Description:
  // Return LargestUsedUniqueId and then incrememt it.
  vtkIdType GetNextUniquePersistentId();

  // Description:
  // Build up the associations of a model geometric entity.  This avoids
  // the problems of dealing with friend classes for models derived
  // from vtkModel.
  void BuildModelEdgeAssociations(vtkModelEdge* edge, vtkModelVertex* vertex0,
                                  vtkModelVertex* vertex1);
  void BuildModelRegionAssociations(vtkModelRegion* region, int numFaces,
                                    vtkModelFace** faces, int* faceSides);

  // Description:
  // Set/get LargestUsedUniqueId.
  vtkSetMacro(LargestUsedUniqueId, vtkIdType);
  friend class vtkCMBParserBase;
  //friend class vtkCMB3dmReader;
  //friend class vtkCMBParserV2;

  // Description:
  // Flag to whether invoke event
  bool BlockModelGeometricEntityEvent;
  void InvokeModelGeometricEntityEvent(unsigned long event, void *callData);
  friend class vtkDiscreteModelEdge;
  friend class vtkDiscreteModelFace;
  friend class vtkDiscreteModelGeometricEntity;
  friend class vtkXMLModelReader;
  friend class vtkModelEdge;
  friend class vtkModelFace;
  friend class vtkModelRegion;
  friend class vtkModelVertex;
  friend class vtkModelGeometricEntity;
  friend class vtkEdgeSplitOperatorClient;

private:
  vtkModel(const vtkModel&);  // Not implemented.
  void operator=(const vtkModel&);  // Not implemented.

  // Description:
  // Used to make sure that all model entities have a unique persistent Id.  This
  // value should be at least as high as the maximum used unique persistend Id
  // in the model.
  vtkIdType LargestUsedUniqueId;
};

#endif

