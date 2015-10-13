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

  smtkInfoMacro(this->log(), "[remus] Asking Server if it supports job reqs");
  remus::proto::Job job = remus::proto::make_invalidJob();
  if (client.canMesh(reqs))
    {
    job = client.submitJob(submission);
    smtkInfoMacro(this->log(), "[remus] Submitted Job to Server: " << remus::proto::to_string(job));
    }

  //once the job is submitted, we wait for the results to come back
  bool haveResultFromWorker = false;
  if( job.valid() )
    {
    smtkInfoMacro(this->log(), "[remus] Querying Status of: " << remus::proto::to_string(job));
    remus::proto::JobStatus currentWorkerStatus = client.jobStatus(job);

    while( currentWorkerStatus.good() )
      { //we need this to not be a busy wait
        //for now lets call sleep to make this less 'heavy'
      // boost::this_thread::sleep( boost::posix_time::milliseconds(250) );
      currentWorkerStatus = client.jobStatus(job);
      }
    smtkInfoMacro(this->log(), "[remus] Final Status of: "
                  << remus::proto::to_string(job)
                  << "is: "
                  << remus::to_string( currentWorkerStatus.status() ) );
    haveResultFromWorker = currentWorkerStatus.finished();
    }

  //the results are the model which we accept
  OperatorResult result = this->createResult( haveResultFromWorker ? OPERATION_SUCCEEDED :
                                                                     OPERATION_FAILED);
  if(haveResultFromWorker)
    {
    //now fetch the latest results from the server
    remus::proto::JobResult updatedModel = client.retrieveResults(job);

    //parse the job result as a json string
    smtk::io::ImportJSON::intoModelManager(updatedModel.data(), this->manager());

    cJSON* root = cJSON_Parse(updatedModel.data());
    smtk::io::ImportJSON::ofMeshesOfModel(root, this->manager());
    cJSON_Delete(root);

    // Now set or increment the SMTK_MESH_GEN_PROP property for the models
    // The client can use this property to check if there is an analysis mesh created
    // for the model
    smtk::model::Models::iterator it;
    for (it = models.begin(); it != models.end(); ++it)
      {
      IntegerList& gen(this->manager()->integerProperty(it->entity(), SMTK_MESH_GEN_PROP));
      if (gen.empty())
        gen.push_back(0);
      else
        ++gen[0];
      }

    //current question is how do we know how to mark the tessellations
    //of the model as modified?
    this->addEntitiesToResult(result, models, MODIFIED);
    result->findModelEntity("mesh_created")->setValues(models.begin(), models.end());
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
