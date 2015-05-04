//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/remus/MeshOperator.h"

#include <remus/client/Client.h>

#include "smtk/model/Session.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/ExportJSON.txx"
#include "smtk/io/ImportJSON.h"

//todo: remove this once remus Issue #183 has been resolved.
// #include <boost/date_time/posix_time/posix_time.hpp>
// #include <boost/thread/thread.hpp>


#include "smtk/extension/remus/MeshOperator_xml.h"


using namespace smtk::model;

namespace
{

std::string extractModelUUIDSAsJSON(smtk::model::Models const& models)
  {
  typedef smtk::model::Models::const_iterator model_const_it;
  smtk::common::UUIDArray modelIds;
  for(model_const_it i=models.begin(); i!=models.end(); ++i)
    {
    modelIds.push_back(i->entity());
    //do we need to export submodels?
    }

  cJSON* top = cJSON_CreateObject();
  cJSON_AddItemToObject(top,
                        "ids",
                        smtk::io::ExportJSON::createUUIDArray(modelIds) );
  char* json = cJSON_Print(top);
  std::string uuidsSerialized(json);
  free(json);
  cJSON_Delete(top);
  return uuidsSerialized;
}

}

namespace smtk {
  namespace model {

//-----------------------------------------------------------------------------
MeshOperator::MeshOperator()
{
}

bool MeshOperator::ableToOperate()
{
  return
    this->Superclass::ableToOperate() &&
    this->specification()->findModelEntity("model")->value().isValid();
  // TODO: Add tests to verify that model dimension matches job requirements.
}

//-----------------------------------------------------------------------------
OperatorResult MeshOperator::operateInternal()
{
  smtk::attribute::ModelEntityItemPtr mspec =
    this->specification()->findModelEntity("model");
  smtk::model::Models models(mspec->begin(), mspec->end());
  smtk::attribute::StringItemPtr endpointItem = this->findString("endpoint");
  smtk::attribute::StringItemPtr requirementsItem = this->findString("remusRequirements");
  smtk::attribute::StringItemPtr attributeItem = this->findString("meshingControlAttributes");


  //deserialize the reqs from the string
  std::istringstream buffer( requirementsItem->value() );
  remus::proto::JobRequirements reqs; buffer >> reqs;

  remus::proto::JobSubmission submission(reqs);
  remus::proto::JobContent meshingControls(reqs.formatType(), attributeItem->value() );

  //create a JobContent that contains the location were to save the output
  //of the mesher
  submission["meshing_attributes"] = meshingControls;

  std::string modelSerialized = smtk::io::ExportJSON::fromModelManager(this->manager());
  submission["model"] = remus::proto::make_JobContent(modelSerialized);

  std::string modelUUIDSSerialized = extractModelUUIDSAsJSON(models);
  submission["modelUUIDS"] = remus::proto::make_JobContent( modelUUIDSSerialized );

  //now that we have the submission, construct a remus client to submit it
  const remus::client::ServerConnection conn =
    remus::client::make_ServerConnection( endpointItem->value() );

  remus::client::Client client(conn);

  //the worker could have gone away while this operator was invoked, or maybe somebody submitted
  //the job using stale data from an older server
  std::cout << "asking server if it can handle job reqs" << std::endl;
  remus::proto::Job job = remus::proto::make_invalidJob();
  if (client.canMesh(reqs))
    {
    std::cout << "Submitting job to server " << std::endl;
    job = client.submitJob(submission);
    }

  //once the job is submitted, we wait for the results to come back
  bool haveResultFromWorker = false;
  if( job.valid() )
    {
    std::cout << " job submitted and is valid, checking the status " << std::endl;
    remus::proto::JobStatus currentWorkerStatus = client.jobStatus(job);
    std::cout << remus::to_string( currentWorkerStatus.status() ) << std::endl;

    while( currentWorkerStatus.good() )
      { //we need this to not be a busy wait
        //for now lets call sleep to make this less 'heavy'
      // boost::this_thread::sleep( boost::posix_time::milliseconds(250) );
      currentWorkerStatus = client.jobStatus(job);
      }
    std::cout << remus::to_string( currentWorkerStatus.status() ) << std::endl;
    haveResultFromWorker = currentWorkerStatus.finished();
    }

  //the results are the model which we accept
  OperatorResult result = this->createResult( haveResultFromWorker ? OPERATION_SUCCEEDED :
                                                                     OPERATION_FAILED);
  if(haveResultFromWorker)
    {
    //now fetch the latest results from the server
    remus::proto::JobResult newModel = client.retrieveResults(job);

    //parse the job result as a json string
    smtk::io::ImportJSON::intoModelManager(newModel.data(), this->manager());

    //current question is how do we know how to mark the tessellations
    //of the model as modified?
    this->addEntitiesToResult(result, models, MODIFIED);
    }
  return result;
}

  } // namespace model
} // namespace smtk

smtkImplementsModelOperator(
  SMTKREMUSEXT_EXPORT,
  smtk::model::MeshOperator,
  remus_mesh,

  "mesh",
  MeshOperator_xml,
  smtk::model::Session);
