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

#include <vtkCmbMesh.h>
#include <vtkCMBModel.h>
#include <vtkCmbModelEdgeMesh.h>
#include <vtkCmbModelEntityMesh.h>
#include <vtkCmbModelFaceMesh.h>
#include <vtkCMBModelReadOperator.h>
#include <vtkCMBModelWrapper.h>
#include <vtkEdgeSplitOperator.h>
#include <vtkMergeOperator.h>
#include <vtkModelEdge.h>
#include <vtkModelFace.h>
#include <vtkModelItemIterator.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

#include "vtkCmbMapReader.h"
#include "vtkCmbTriangleMesher.h"

// This tests the events for the SimBuilder Mesh.



double TimeSimMesher(const char* fileName, double MaxArea, double MinAngle)
{
  vtkCMBModelWrapper* modelWrapper = vtkCMBModelWrapper::New();
  vtkCMBModel* model = modelWrapper->GetModel();

  vtkSmartPointer<vtkCMBModelReadOperator> reader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  reader->SetFileName(fileName);
  reader->Operate(modelWrapper);
  if(reader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << fileName);
    return 1;
    }

  vtkSmartPointer<vtkCmbMesh> mesh =
    vtkSmartPointer<vtkCmbMesh>::New();
  mesh->Initialize(model);
  mesh->SetGlobalLength(100.);

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(model->NewIterator(vtkModelEdgeType));
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkModelEdge::SafeDownCast(edges->GetCurrentItem());
    vtkCmbModelEdgeMesh* edgeMesh = vtkCmbModelEdgeMesh::SafeDownCast(
      mesh->GetModelEntityMesh(edge));
    }

  //start time here
  vtkSmartPointer<vtkTimerLog> tl = vtkSmartPointer<vtkTimerLog>::New();
  tl->StartTimer();
  mesh->SetGlobalMaximumArea(MaxArea);
  mesh->SetGlobalMinimumAngle(MinAngle);
  vtkSmartPointer<vtkModelItemIterator> faces;
  faces.TakeReference(model->NewIterator(vtkModelFaceType));
  for(faces->Begin();!faces->IsAtEnd();faces->Next())
    {
    vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
    vtkCmbModelFaceMesh* faceMesh = vtkCmbModelFaceMesh::SafeDownCast(
      mesh->GetModelEntityMesh(face));
    }
  //end timing
  tl->StopTimer();

  model->Reset();
  modelWrapper->Delete();
  return tl->GetElapsedTime();
}

double TimeMapMesher(const char* fileName, double MaxArea, double MinAngle)
{
  vtkSmartPointer<vtkCmbMapReader> reader = vtkSmartPointer<vtkCmbMapReader>::New();
  reader->SetFileName(fileName);
  reader->Update();

  vtkSmartPointer<vtkCmbTriangleMesher> mesher = vtkSmartPointer<vtkCmbTriangleMesher>::New();
  mesher->SetInputConnection(reader->GetOutputPort());
  mesher->SetPreserveBoundaries(true);
  mesher->SetMaxArea(MaxArea);
  mesher->SetMaxAreaMode(2); //Relative to bounds is what Sim Mesher does
  mesher->SetUseMinAngle(true);
  mesher->SetMinAngle(MinAngle);

  vtkSmartPointer<vtkTimerLog> tl = vtkSmartPointer<vtkTimerLog>::New();
  tl->StartTimer();
  mesher->Update();
  tl->StopTimer();
  return tl->GetElapsedTime();
}


int main(int argc, char ** argv)
{
  //the point of this test is to compare the relative speeds
  //of the sim face mesher compared to the map mesher
  //we will configure the two mesher with the same parameters to
  //minimize the variables
  if(argc != 3)
    {
    vtkGenericWarningMacro("Not enough arguments");
    vtkGenericWarningMacro("Pass in two files first that is cmb, second that is map file");
    vtkGenericWarningMacro("Make sure the represent the same data for a fair fight!");
    return 1;
    }
  double MaxArea = 0.005;
  double MinAngle = 30;
  double time = TimeSimMesher(argv[1],MaxArea,MinAngle);
  double time2 = TimeMapMesher(argv[2],MaxArea,MinAngle);

  std::cout << "Sim Mesher time is: " << time << std::endl;
  std::cout << "Map Mesher time is: " << time2 << std::endl;
  return 0;
}
