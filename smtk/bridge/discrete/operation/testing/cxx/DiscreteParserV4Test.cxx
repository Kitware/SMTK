//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkCMBModelReader.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkCMBModelWriterBase.h"
#include "vtkCMBParserV4.h"
#include "vtkModelItemIterator.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include <map>

// This tests the version 4 parser for a 2D model.

struct ModelInfo
{
  ModelInfo() : NumberOfVertices(0)
    {}
  // A map from a model entity's unique persistent Id to the
  // number of cells in that entity (0 if it has no cells)
  std::map<vtkIdType, vtkIdType> EdgeInfo;

  vtkIdType NumberOfVertices;
};

// checks the model to see if we get the expected results
// returns the number of errors we encountered
int CheckModel(vtkDiscreteModel* Model, ModelInfo & modelInfo)

{
  int Errors = 0;
  if(Model->GetNumberOfAssociations(vtkModelEdgeType) != static_cast<vtkIdType>(modelInfo.EdgeInfo.size()) ||
     Model->GetNumberOfAssociations(vtkModelVertexType) != modelInfo.NumberOfVertices ||
     Model->GetNumberOfAssociations(vtkModelFaceType) != 1)
    {
    vtkGenericWarningMacro("Wrong number of model entities.");
    return 1;
    }

  vtkModelItemIterator* Edges = Model->NewIterator(vtkModelEdgeType);
  for(Edges->Begin();!Edges->IsAtEnd();Edges->Next())
    {
    vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(
      Edges->GetCurrentItem());
    if(modelInfo.EdgeInfo.find(Edge->GetUniquePersistentId()) == modelInfo.EdgeInfo.end())
      {
      vtkGenericWarningMacro("Edge has wrong unique persistent id.");
      Errors++;
      }
    else
      {
      vtkPolyData* Geometry = vtkPolyData::SafeDownCast(Edge->GetGeometry());
      vtkIdType NumberOfCells = modelInfo.EdgeInfo[Edge->GetUniquePersistentId()];
      if(Geometry == 0 || Geometry->GetNumberOfCells() != NumberOfCells ||
       Edge->GetNumberOfCells() != NumberOfCells)
        {
        vtkGenericWarningMacro("Edge has wrong polydata geometry.");
        Errors++;
        }
      }
    }
  Edges->Delete();

  vtkModelItemIterator* Faces = Model->NewIterator(vtkModelFaceType);
  for(Faces->Begin();!Faces->IsAtEnd();Faces->Next())
    {
    vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(
      Faces->GetCurrentItem());
    if(Face->GetMaterial() == 0)
      {
      vtkGenericWarningMacro("Model face has no material.");
      Errors++;
      }
    }
  Faces->Delete();

  cout << "Model check with " << Errors << " errors.\n";

  return Errors;
}

// check to see if the model modification operators work properly
// return the number of errors we get
int CheckModifyModel(vtkDiscreteModel* Model, int SplitPointId)
{
  int Errors = 0;

  vtkModelItemIterator* Edges = Model->NewIterator(vtkModelEdgeType);
  Edges->Begin();
  if(Edges->IsAtEnd())
    {
    vtkGenericWarningMacro("No model edge to modify.");
    Edges->Delete();
    return 1;
    }
  vtkDiscreteModelEdge* Edge = vtkDiscreteModelEdge::SafeDownCast(Edges->GetCurrentItem());
  Edges->Delete();

  vtkIdType createdEdgeId = -100, createdVertexId = -100;
  if(!Edge->Split(SplitPointId, createdVertexId, createdEdgeId))
    {
    vtkGenericWarningMacro("Edge split operation failed for looped edge.");
    Errors++;
    }

  if(createdEdgeId >= 0 && Model->GetNumberOfAssociations(vtkModelEdgeType) == 1)
    {
    vtkGenericWarningMacro("Should not have created a new model edge.");
    Errors++;
    }

  cout << "Edge split with " << Errors << " errors.\n";

  return Errors;
}

int main(int argc, char ** argv)
{
  if(argc < 2)
    {
    vtkGenericWarningMacro("Not enough arguments -- need to specify the V4 CMB file.");
    return 1;
    }
  int Errors = 0;

  vtkDiscreteModelWrapper* ModelWrapper = vtkDiscreteModelWrapper::New();
  vtkDiscreteModel* Model = ModelWrapper->GetModel();

  vtkSmartPointer<vtkCMBModelReader> Reader =
    vtkSmartPointer<vtkCMBModelReader>::New();

  Reader->SetFileName(argv[1]);
  Reader->Update();

  vtkPolyData* MasterPoly = Reader->GetOutput();

  vtkSmartPointer<vtkCMBParserV4> Parser =
    vtkSmartPointer<vtkCMBParserV4>::New();
  if(!Parser)
    {
    vtkGenericWarningMacro("File version not supported.");
    return 1;
    }

  if(Parser->Parse(MasterPoly, Model) == 0)
    {
    vtkGenericWarningMacro("Could not parse file");
    Errors++;
    }

  ModelInfo modelInfo;
  modelInfo.EdgeInfo[18] = 6;
  Errors += CheckModel(Model, modelInfo);

  // hopefully we have a valid model now so we'll try doing
  // some modifications to it -- first is splitting a looped edge
  Errors += CheckModifyModel(Model, 0);

  modelInfo.NumberOfVertices = 1;
  Errors += CheckModel(Model, modelInfo);

  // second modification is splitting of an edge into two edges
  modelInfo.NumberOfVertices = 2;
  modelInfo.EdgeInfo[22] = 3;
  modelInfo.EdgeInfo[18] = 3;
  Errors += CheckModifyModel(Model, 3);

  Errors += CheckModel(Model, modelInfo);

  // try writing it out
  vtkCMBModelWriterBase* Writer = vtkCMBModelWriterBase::New();
  Writer->SetFileName("junkv4.cmb");
  Writer->SetVersion(4);
  Writer->Operate(ModelWrapper);
  Writer->Delete();

  Model->Reset();

  //Reader->SetFileName(argv[1]);
  Reader->SetFileName("junkv4.cmb");
  Reader->Update();

  if(Parser->Parse(Reader->GetOutput(), Model) == 0)
    {
    vtkGenericWarningMacro("Could not parse file");
    Errors++;
    }

  std::cout << "Finished with " << Errors << " errors.\n";
  ModelWrapper->Delete();

  return Errors;
}
