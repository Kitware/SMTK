
#include "smtk/bridge/discrete/operators/MeshOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include <remus/client/Client.h>

#include "smtk/model/Session.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"

//todo: remove this once remus Issue #183 has been resolved.
// #include <boost/date_time/posix_time/posix_time.hpp>
// #include <boost/thread/thread.hpp>


#include "MeshOperator_xml.h"


using namespace smtk::model;

namespace
{

//------------------------------------------------------------------------------
remus::proto::JobContent make_FilePath(const std::string& filePath)
{
  //encode the file path as a fileHandle, with user encoding
  return remus::proto::make_JobContent( remus::common::FileHandle(filePath) );
}
}

namespace smtk {
  namespace bridge {

  namespace discrete {

//-----------------------------------------------------------------------------
MeshOperator::MeshOperator()
{
}


//-----------------------------------------------------------------------------
bool MeshOperator::ableToOperate()
{
  smtk::model::Model model;

  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity()) &&
    // The endpoint must be filled in:
    this->findString("endpoint") != smtk::attribute::StringItemPtr() &&
    // The requirements must be filled in:
    this->findString("remusRequirements") != smtk::attribute::StringItemPtr() &&
    // The attributes to control the meshing must be filled in:
    this->findString("meshingControlAttributes") != smtk::attribute::StringItemPtr()
    ;
}

//-----------------------------------------------------------------------------
OperatorResult MeshOperator::operateInternal()
{

  smtk::model::Model model = this->specification()->findModelEntity(
    "model")->value().as<smtk::model::Model>();
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

  //lastly all we have to do is serialize the model
  submission["model"] = remus::proto::make_JobContent( smtk::io::ExportJSON::fromModelManager( this->manager()) );

  //now that we have the submission, construct a remus client to submit it
  const remus::client::ServerConnection conn =
    remus::client::make_ServerConnection( endpointItem->value() );

  remus::client::Client client(conn);
  remus::proto::Job job = client.submitJob(submission);

  //once the job is submitted, we wait for the results to come back
  bool haveResultFromWorker = false;
  if( job.valid() )
    {
    remus::proto::JobStatus currentWorkerStatus = client.jobStatus(job);
    while( currentWorkerStatus.good() )
      { //we need this to not be a busy wait
        //for now lets call sleep to make this less 'heavy'
      // boost::this_thread::sleep( boost::posix_time::milliseconds(250) );
      currentWorkerStatus = client.jobStatus(job);
      }
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
    this->addEntityToResult(result, model, MODIFIED);
    }
  return result;
}

smtk::bridge::discrete::Session* MeshOperator::discreteSession() const
{
  return dynamic_cast<smtk::bridge::discrete::Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::MeshOperator,
  discrete_mesh,
  "mesh",
  MeshOperator_xml,
  smtk::model::Session);
