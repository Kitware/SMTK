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

#include "vtkCMBMeshServerJobSubmitter.h"

#include "vtkCellArray.h"
#include "vtkDataArrayIteratorMacro.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkModelEntity.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include "vtkModelItemIterator.h"
#include "vtkDiscreteModelRegion.h"

#include <iterator>
#include <vector>
#include <string>
#include <iostream>

#include <remus/client/Client.h>

vtkStandardNewMacro(vtkCMBMeshServerJobSubmitter)

namespace
{
  //----------------------------------------------------------------------------
  template< typename ValueType>
  inline bool writeToStream(std::ostream& buffer,
                            const ValueType* begin,
                            const ValueType* end,
                            vtkIdType numTuples,
                            vtkIdType numComponents )
  {
    const std::size_t expectedSize(numTuples*numComponents);
    if( std::distance(begin,end) != expectedSize )
      { return false; }

    //we have contiguous storage
    const char* csrc = reinterpret_cast<const char*>(begin);
    const std::streamsize size = sizeof(ValueType)*(numTuples*numComponents);

    buffer << vtkTypeTraits<ValueType>::VTKTypeID() << std::endl;
    buffer << numTuples << std::endl;
    buffer << numComponents << std::endl;
    buffer.write(csrc,size);
    buffer << std::endl;

    return true;
  }

  //----------------------------------------------------------------------------
  template< typename ValueType>
  inline bool writeToStream(std::ostream& buffer,
                     vtkTypedDataArrayIterator<ValueType> begin,
                     vtkTypedDataArrayIterator<ValueType> end,
                     vtkIdType numTuples, vtkIdType numComponents )
  {
    //allocate temporary storage for the conversion to char* so we can
    //handle mapped iterators
    std::vector<ValueType> storage;
    storage.reserve( numTuples * numComponents );

    std::copy(begin,end, std::back_inserter(storage));

    const ValueType* storage_start = &(storage[0]);
    const ValueType* storage_end = &(storage[storage.size()-1]);

    return writeToStream(buffer, storage_start, storage_end,
                         numTuples, numComponents);

  }

  //----------------------------------------------------------------------------
  inline  bool writeToStream(std::ostream& buffer, vtkDataArray* data)
  {
    bool written = false;
    switch (data->GetDataType())
      {
      vtkDataArrayIteratorMacro(data,
        written =  writeToStream<vtkDAValueType>(buffer,
                                             vtkDABegin, vtkDAEnd,
                                             data->GetNumberOfTuples(),
                                             data->GetNumberOfComponents())
      );
      default:
        break;
      }
    return written;
  }

  //---------------------------------------------------------------------------
  template<typename T>
  inline void serialize_mesh(T &buffer, vtkSmartPointer<vtkPolyData> mesh)
  {
    writeToStream(buffer, mesh->GetPoints()->GetData());
    writeToStream(buffer, mesh->GetPolys()->GetData());
    buffer << std::endl;
  }

  //---------------------------------------------------------------------------
  template<typename T>
  inline void serialize_classification(T &buffer, vtkDiscreteModel* model)
  {
    typedef vtkDiscreteModel::ClassificationType ClassificationType;
    ClassificationType& meshClassification = model->GetMeshClassification();


    //find the number of faces we need to query
    const vtkIdType size = meshClassification.size( ClassificationType::FACE_DATA );
    vtkNew<vtkIdTypeArray> cellClassification;
    cellClassification->SetNumberOfValues(size);

    //fill a vtkDataArray with all the cell classification ids
    //we will need to verify that this works when we have a mixed mode model
    for(vtkIdType i=0; i < size; ++i)
      {
      vtkDiscreteModelGeometricEntity* entity = meshClassification.GetEntity(i);
      const vtkIdType persitentId =
                      entity->GetThisModelEntity()->GetUniquePersistentId();
      cellClassification->SetValue(i, persitentId);
      }

    writeToStream(buffer, cellClassification.GetPointer());
    buffer << std::endl;
  }

  //---------------------------------------------------------------------------
  template<typename T>
  inline void serialize_region(T &buffer, vtkDiscreteModel* model)
  {
    const vtkIdType numRegions = model->GetNumberOfModelEntities(vtkModelRegionType);

    //4 components because we have region id, and x,y,z for coordinate inside
    //region
    vtkNew<vtkDoubleArray> regionInfo;
    regionInfo->SetNumberOfComponents(4);
    regionInfo->SetNumberOfTuples(numRegions);

    vtkModelItemIterator* regions = model->NewIterator(vtkModelRegionType);
    vtkIdType i=0;
    for(regions->Begin();
        !regions->IsAtEnd();
        regions->Next(), i++)
      {
      vtkDiscreteModelRegion* region =
        vtkDiscreteModelRegion::SafeDownCast(regions->GetCurrentItem());

      const vtkIdType regionId = region->GetUniquePersistentId();
      double *pointInside = region->GetPointInside();

      double values[4]={regionId,0,0,0};
      if(pointInside)
        {
        values[1]=pointInside[0];
        values[2]=pointInside[1];
        values[3]=pointInside[2];
        }
      regionInfo->SetTuple(i,values);
    }

    writeToStream(buffer,regionInfo.GetPointer());
    buffer << std::endl;
  }

}

//-----------------------------------------------------------------------------
vtkCMBMeshServerJobSubmitter::vtkCMBMeshServerJobSubmitter():
  Endpoint(),
  Submission(),
  LastSubmittedJob()
{

}

//-----------------------------------------------------------------------------
vtkCMBMeshServerJobSubmitter::~vtkCMBMeshServerJobSubmitter()
{

}

//-----------------------------------------------------------------------------
void vtkCMBMeshServerJobSubmitter::Operate(
                                         vtkDiscreteModelWrapper* modelWrapper)
{
  this->OperateSucceeded = 0;

  if(this->Submission.size() == 0 || this->Endpoint.size() == 0 ||
     modelWrapper == NULL)
    {
    return;
    }

  using namespace remus;

  vtkDiscreteModel* model = modelWrapper->GetModel();
  const DiscreteMesh& mesh = model->GetMesh();

  //write out the models mesh, the mesh classification, and the regions info
  //which is the region id and a point inside the region
  std::ostringstream serializedDiscreteMesh;
  serialize_mesh(serializedDiscreteMesh, mesh.ShallowCopyFaceData() );
  serialize_classification(serializedDiscreteMesh,model);
  serialize_region(serializedDiscreteMesh,model);

  //currently can't do a zero copy for model content
  remus::proto::JobContent discreteContent(remus::common::ContentFormat::XML,
                                           serializedDiscreteMesh.str());

  Client rc(client::make_ServerConnection(this->Endpoint));

  //we need to add the model information but we submit
  proto::JobSubmission submission = proto::to_JobSubmission(this->Submission);
  submission["discrete_mesh"] = discreteContent;

  proto::Job j = rc.submitJob( submission );

  this->OperateSucceeded = static_cast<int>(j.valid());

  //always update the LastSubmittedJob, that way people can grab this
  //and check the valid flag to see if they got a proper remus job id.
  this->LastSubmittedJob = proto::to_string(j);
}


//-----------------------------------------------------------------------------
void vtkCMBMeshServerJobSubmitter::SetEndpoint(const char* ep)
{
  this->Endpoint = std::string(ep);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBMeshServerJobSubmitter::SetSubmission(const char* submission)
{
  this->Submission = std::string(submission);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBMeshServerJobSubmitter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Endpoint: " << this->Endpoint << std::endl;
  os << indent << "Submission: " << this->Submission << std::endl;
  os << indent << "LastSubmittedJob: " << this->LastSubmittedJob << std::endl;
}
