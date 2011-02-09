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
#include "vtkCmbMesh.h"

#include <vtkCallbackCommand.h>
#include <vtkCMBModel.h>
#include "vtkCmbModelEdgeMeshClient.h"
#include "vtkCmbModelEdgeMeshServer.h"
#include "vtkCmbModelFaceMesh.h"
#include <vtkCMBModelGeometricEntity.h>
#include "vtkCmbModelVertexMesh.h"
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkMergeEventData.h>
#include <vtkModel.h>
#include <vtkModelEdge.h>
#include <vtkModelEntity.h>
#include <vtkModelFace.h>
#include <vtkModelGeometricEntity.h>
#include <vtkModelItemIterator.h>
#include <vtkModelVertex.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSplitEventData.h>
#include <vtkWeakPointer.h>

vtkCxxRevisionMacro(vtkCmbMesh, "");

//----------------------------------------------------------------------------
vtkCmbMesh::vtkCmbMesh()
{
  this->Visible = true;
  this->GlobalLength = 0;
  this->GlobalMaximumArea = 0;
  this->GlobalMinimumAngle = 0;
  this->Model = NULL;
}

//----------------------------------------------------------------------------
vtkCmbMesh::~vtkCmbMesh()
{
}

//----------------------------------------------------------------------------
void vtkCmbMesh::Reset()
{
  this->GlobalLength = 0;
  this->GlobalMaximumArea = 0;
  this->GlobalMinimumAngle = 0;
  this->Model = NULL;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkModel* vtkCmbMesh::GetModel()
{
  return this->Model;
}

//----------------------------------------------------------------------------
void vtkCmbMesh::ModelGeometricEntityChanged(
  vtkObject *caller, unsigned long event, void *cData, void *callData)
{
  vtkCmbMesh* cmbMesh = (vtkCmbMesh*) cData;
  vtkModel* model = cmbMesh->Model;
  if(event == ModelGeometricEntitySplit)
    {
    vtkSplitEventData* splitEventData = (vtkSplitEventData*) callData;
    if(model->GetModelDimension() == 2)
      {
      cmbMesh->ModelEdgeSplit(splitEventData);
      }
    else
      {
      vtkGenericWarningMacro("Model face split not implemented yet.")
      }
    }
  else if(event == ModelGeometricEntitiesAboutToMerge)
    {
    vtkMergeEventData* mergeEventData = (vtkMergeEventData*) callData;
    if(model->GetModelDimension() == 2)
      {
      cmbMesh->ModelEdgeMerge(mergeEventData);
      }
    else
      {
      vtkGenericWarningMacro("Model face merge not implemented yet.")
      }
    }
  else if(event == ModelGeometricEntityBoundaryModified)
    {
    cmbMesh->ModelEntityBoundaryModified((vtkModelGeometricEntity*)callData);
    }
}

//----------------------------------------------------------------------------
void vtkCmbMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Visible: " << this->Visible << "\n";
  os << indent << "GlobalLength: " << this->GlobalLength << "\n";
  os << indent << "GlobalMaximumArea: " << this->GlobalMaximumArea << "\n";
  os << indent << "GlobalMinimumAngle: " << this->GlobalMinimumAngle << "\n";
  if(this->Model)
    {
    os << indent << "Model: " << this->Model << "\n";
    }
  else
    {
    os << indent << "Model: (NULL)\n";
    }
}

