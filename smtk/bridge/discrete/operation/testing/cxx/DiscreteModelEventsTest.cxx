//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelReadOperator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkEdgeSplitOperator.h"
#include "vtkMergeEventData.h"
#include "vtkMergeOperator.h"
#include "vtkModelItemIterator.h"
#include "vtkNew.h"
#include "vtkSplitEventData.h"
#include "vtkSplitOperator.h"
#include <map>
#include <vtkCallbackCommand.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>


// This tests the events for model geometric entities.


// Callback function to handle events
struct ClientData {
  int EntityCounter;
  std::map<void*, int> EntitiesMap;
  // number of model geometric entities which have had their boundary
  // modified
  int NumberOfModifiedEvents;
  int NumberOfMergeEvents;
  int NumberOfSplitEvents;
  int NumberOfErrors;
  ClientData() : EntityCounter(0), NumberOfModifiedEvents(0),
                 NumberOfMergeEvents(0), NumberOfSplitEvents(0),
                 NumberOfErrors(0)
  {}
};

void ModelGeometricEntityChanged(
  vtkObject * /*caller*/, unsigned long event, void *cData, void *callData)
{
  ClientData* clientData = static_cast<ClientData*>(cData);
  if(event == ModelGeometricEntityCreated)
    {
    if(clientData->EntitiesMap.find(callData) != clientData->EntitiesMap.end())
      {
      vtkGenericWarningMacro("Multiple events for creating the same entity.");
      clientData->NumberOfErrors++;
      return;
      }
    clientData->EntitiesMap[callData] = clientData->EntityCounter++;
    }
  else if(event == ModelGeometricEntityAboutToDestroy)
    {
    if(clientData->EntitiesMap.find(callData) == clientData->EntitiesMap.end())
      {
      vtkGenericWarningMacro("Delete event is missing its create event.");
      clientData->NumberOfErrors++;
      return;
      }
    clientData->EntitiesMap.erase(callData);
    }
  else if(event == ModelGeometricEntityBoundaryModified)
    {

    if(clientData->EntitiesMap.find(callData) == clientData->EntitiesMap.end())
      {
      vtkGenericWarningMacro("Modify event is missing its create event.");
      clientData->NumberOfErrors++;
      return;
      }
    clientData->NumberOfModifiedEvents++;
    }
  else if(event == ModelGeometricEntitiesAboutToMerge)
    {
    vtkMergeEventData* mergeEventData = static_cast<vtkMergeEventData*>(callData);
    vtkModelEntity* sourceEntity = mergeEventData->GetSourceEntity()->GetThisModelEntity();
    if(sourceEntity->IsA("vtkDiscreteModelFace"))
      {  // 3D merge
      if(sourceEntity->GetUniquePersistentId() != 0)
        {
        vtkGenericWarningMacro("Wrong source entity for 3D merge.");
        clientData->NumberOfErrors++;
        }
      vtkModelEntity* targetFace = mergeEventData->GetTargetEntity()->GetThisModelEntity();
      if(targetFace->GetUniquePersistentId() != 89)
        {
        vtkGenericWarningMacro("Wrong target entity for 3D merge.");
        clientData->NumberOfErrors++;
        }
      if(mergeEventData->GetLowerDimensionalIds()->GetNumberOfTuples() != 0)
        {
        vtkGenericWarningMacro("There should be no LowerDimensionalIds for 3D merge.");
        clientData->NumberOfErrors++;
        }
      }
    else
      {  // 2D merge
      if(sourceEntity->GetUniquePersistentId() != 17)
        {
        vtkGenericWarningMacro("Wrong source entity for 2D merge.");
        clientData->NumberOfErrors++;
        }
      vtkModelEntity* targetFace = mergeEventData->GetTargetEntity()->GetThisModelEntity();
      if(targetFace->GetUniquePersistentId() != 42)
        {
        vtkGenericWarningMacro("Wrong target entity for 2D merge.");
        clientData->NumberOfErrors++;
        }
      if(mergeEventData->GetLowerDimensionalIds()->GetNumberOfTuples() != 1)
        {
        vtkGenericWarningMacro("There should be one LowerDimensionalIds for 2D merge.");
        clientData->NumberOfErrors++;
        }
      else if(mergeEventData->GetLowerDimensionalIds()->GetValue(0) != 41)
        {
        vtkGenericWarningMacro("The Id should be 41 for the 2D merge's LowerDimensionalId.");
        clientData->NumberOfErrors++;
        }
      }
    }
  else if(event == ModelGeometricEntitySplit)
    {
    vtkSplitEventData* splitEventData = static_cast<vtkSplitEventData*>(callData);
    vtkModelEntity* sourceEntity =
      splitEventData->GetSourceEntity()->GetThisModelEntity();
    if(sourceEntity->IsA("vtkDiscreteModelEdge"))
      {  // 2D split
      if(sourceEntity->GetUniquePersistentId() != 17)
        {
        vtkGenericWarningMacro("The 2D split source entity is incorrect.");
        clientData->NumberOfErrors++;
        }
      vtkIdList* createdEntityIds = splitEventData->GetCreatedModelEntityIds();
      if(createdEntityIds->GetNumberOfIds() != 2)
        {
        vtkGenericWarningMacro("The 2D split has the wrong amount of created entities.");
        clientData->NumberOfErrors++;
        }
      else
        {
        if(createdEntityIds->GetId(0) != 41 && createdEntityIds->GetId(1) != 41)
          {
          vtkGenericWarningMacro("The 2D split has the wrong created entity.");
          clientData->NumberOfErrors++;
          }
        if(createdEntityIds->GetId(0) != 42 && createdEntityIds->GetId(1) != 42)
          {
          vtkGenericWarningMacro("The 2D split has the wrong created entity.");
          clientData->NumberOfErrors++;
          }
        }
      }
    else
      {  // 3D split
      if(sourceEntity->GetUniquePersistentId() != 0)
        {
        vtkGenericWarningMacro("The 3D split source entity is incorrect.");
        clientData->NumberOfErrors++;
        }
      vtkIdList* createdEntityIds = splitEventData->GetCreatedModelEntityIds();
      if(createdEntityIds->GetNumberOfIds() != 9)
        {
        vtkGenericWarningMacro("The 3D split has the wrong amount of created entities.");
        clientData->NumberOfErrors++;
        }
      else
        {
        for(vtkIdType i=0;i<createdEntityIds->GetNumberOfIds();i++)
          {
          vtkIdType id = createdEntityIds->GetId(i);
          if(id < 89 || id > 97)
            {
            vtkGenericWarningMacro("The 3D split has the wrong created entity.");
            clientData->NumberOfErrors++;
            }
          }
        }
      }
    }
  else
    {
    vtkGenericWarningMacro("Unknown event.");
    clientData->NumberOfErrors++;
    }
}

int Check3DModel(const char* fileName)
  {
  vtkDiscreteModelWrapper* modelWrapper = vtkDiscreteModelWrapper::New();
  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(ModelGeometricEntityChanged);
  ClientData clientData;
  callbackCommand->SetClientData(static_cast<void*>( &clientData ));
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
  model->AddObserver(ModelGeometricEntityCreated, callbackCommand);
  model->AddObserver(ModelGeometricEntityAboutToDestroy, callbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, callbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, callbackCommand);

  vtkSmartPointer<vtkCMBModelReadOperator> reader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  reader->SetFileName(fileName);
  reader->Operate(modelWrapper);
  if(reader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << fileName);
    return 1;
    }

  if(clientData.EntityCounter != 22 || clientData.NumberOfModifiedEvents != 0)
    {
    vtkGenericWarningMacro("3D read operator missed some events.");
    clientData.NumberOfErrors++;
    }

  // test 3D split
  vtkSmartPointer<vtkSplitOperator> splitOperator =
    vtkSmartPointer<vtkSplitOperator>::New();
  vtkModelItemIterator* faces = model->NewIterator(vtkModelFaceType);
  faces->Begin();
  vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
  faces->Delete();
  splitOperator->SetId(face->GetUniquePersistentId());
  splitOperator->SetFeatureAngle(30);
  splitOperator->Operate(modelWrapper);
  if(splitOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model face split operator failed.");
    clientData.NumberOfErrors++;
    }
  if(clientData.EntityCounter != 31 || clientData.NumberOfModifiedEvents != 9)
    {
    vtkGenericWarningMacro("Model face split operator missed some events.");
    clientData.NumberOfErrors++;
    }
  vtkIdTypeArray* createdModelFaceIds = splitOperator->GetCreatedModelFaceIDs();
  if(createdModelFaceIds->GetNumberOfTuples() == 0)
    {
    vtkGenericWarningMacro("Split operator failed to split any model faces.");
    return clientData.NumberOfErrors+1;
    }

  // test 3D merge
  vtkSmartPointer<vtkMergeOperator> mergeOperator =
    vtkSmartPointer<vtkMergeOperator>::New();
  mergeOperator->SetSourceId(face->GetUniquePersistentId());
  mergeOperator->SetTargetId(createdModelFaceIds->GetValue(0));
  mergeOperator->Operate(modelWrapper);
  if(mergeOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model face merge operator failed.");
    clientData.NumberOfErrors++;
    }
  if(clientData.EntityCounter != 31 || clientData.NumberOfModifiedEvents != 10)
    {
    vtkGenericWarningMacro("Model face merge operator missed some events.");
    clientData.NumberOfErrors++;
    }

  model->Reset();
  if(clientData.EntitiesMap.size() != 0)
    {
    vtkGenericWarningMacro("Not all entities had a delete event triggered (3d).");
    clientData.NumberOfErrors++;
    }

  if(clientData.EntityCounter != 31 || clientData.NumberOfModifiedEvents != 10)
    {
    vtkGenericWarningMacro("3D model reset has bad events.");
    clientData.NumberOfErrors++;
    }

  modelWrapper->Delete();

  return clientData.NumberOfErrors;
}

int Check2DModel(const char* fileName)
  {
  vtkDiscreteModelWrapper* modelWrapper = vtkDiscreteModelWrapper::New();
  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkSmartPointer<vtkCallbackCommand> callbackCommand =
    vtkSmartPointer<vtkCallbackCommand>::New();
  callbackCommand->SetCallback(ModelGeometricEntityChanged);
  ClientData clientData;
  callbackCommand->SetClientData(static_cast<void*>(&clientData));
  model->AddObserver(ModelGeometricEntityBoundaryModified, callbackCommand);
  model->AddObserver(ModelGeometricEntityCreated, callbackCommand);
  model->AddObserver(ModelGeometricEntityAboutToDestroy, callbackCommand);
  model->AddObserver(ModelGeometricEntitiesAboutToMerge, callbackCommand);
  model->AddObserver(ModelGeometricEntitySplit, callbackCommand);

  vtkSmartPointer<vtkCMBModelReadOperator> reader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  reader->SetFileName(fileName);
  reader->Operate(modelWrapper);
  if(reader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << fileName);
    return 1;
    }

  if(clientData.EntityCounter != 21 || clientData.NumberOfModifiedEvents != 0)
    {
    vtkGenericWarningMacro("2D read operator missed some events.");
    clientData.NumberOfErrors++;
    }

  // test 2D split by splitting an edge adjacent to 2 faces
  vtkSmartPointer<vtkEdgeSplitOperator> splitOperator =
    vtkSmartPointer<vtkEdgeSplitOperator>::New();
  vtkModelEdge* edge = vtkModelEdge::SafeDownCast(
    model->GetModelEntity(vtkModelEdgeType, 17));
  splitOperator->SetEdgeId(edge->GetUniquePersistentId());
  splitOperator->SetPointId(6);
  splitOperator->Operate(modelWrapper);
  if(splitOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model edge split operator failed.");
    clientData.NumberOfErrors++;
    }
  if(clientData.EntityCounter != 23 || clientData.NumberOfModifiedEvents != 3)
    {
    vtkGenericWarningMacro("Model edge split operator missed some events.");
    clientData.NumberOfErrors++;
    }
  if(splitOperator->GetCreatedModelEdgeId() < 0)
    {
    vtkGenericWarningMacro("Split operator failed to split any model edges.");
    return clientData.NumberOfErrors+1;
    }

  // test 2D merge
  vtkSmartPointer<vtkMergeOperator> mergeOperator =
    vtkSmartPointer<vtkMergeOperator>::New();
  mergeOperator->SetSourceId(edge->GetUniquePersistentId());
  mergeOperator->SetTargetId(splitOperator->GetCreatedModelEdgeId());
  mergeOperator->AddLowerDimensionalId(41);
  mergeOperator->Operate(modelWrapper);
  if(mergeOperator->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Model edge merge operator failed.");
    clientData.NumberOfErrors++;
    }
  if(clientData.EntityCounter != 23 || clientData.NumberOfModifiedEvents != 5)
    {
    vtkGenericWarningMacro("Model edge merge operator missed some events.");
    clientData.NumberOfErrors++;
    }

  model->Reset();
  if(clientData.EntitiesMap.size() != 0)
    {
    vtkGenericWarningMacro("Not all entities had a delete event triggered (2D).");
    clientData.NumberOfErrors++;
    }

  modelWrapper->Delete();

  return clientData.NumberOfErrors;
}

int main(int argc, char ** argv)
{
  if(argc != 3)
    {
    vtkGenericWarningMacro("Not enough arguments -- need to specify a 2D and 3D CMB model file.");
    return 1;
    }
  int errors = Check2DModel(argv[1]);
  errors += Check3DModel(argv[2]);
  std::cout << "Finished with " << errors << " errors.\n";
  return errors;
}
