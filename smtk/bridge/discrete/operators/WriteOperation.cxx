//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "WriteOperation.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelItem.h"
#include <vtksys/SystemTools.hxx>

#include "WriteOperation_xml.h"

// #define SMTK_DISCRETE_SESSION_DEBUG

#if defined(SMTK_DISCRETE_SESSION_DEBUG)
#include "cJSON.h"
#include "smtk/io/SaveJSON.h"
#endif

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

WriteOperation::WriteOperation()
{
  this->m_currentversion = 5;
}

bool WriteOperation::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  smtk::model::Models models = this->parameters()->associatedModelEntities<smtk::model::Models>();

  // The SMTK model must be valid
  if (models.size() == 0 || !models[0].isValid())
  {
    return false;
  }

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(models[0].component()->resource());

  // The CMB model must exist:
  if (!resource->discreteSession()->findModelEntity(models[0].entity()))
  {
    return false;
  }

  std::string filename = this->parameters()->findFile("filename")->value();
  return !filename.empty();
}

WriteOperation::Result WriteOperation::operateInternal()
{
  std::string fname = this->parameters()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  // make sure the filename has .cmb extension
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fname);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext != ".cmb")
  {
    std::string tmpname = vtksys::SystemTools::GetFilenameWithoutLastExtension(fname);
    fname = vtksys::SystemTools::GetFilenamePath(fname);
    fname.append("/").append(tmpname).append(".cmb");
  }

  this->m_op->SetFileName(fname.c_str());

  // ableToOperate should have verified that the model exists
  smtk::model::Models models = this->parameters()->associatedModelEntities<smtk::model::Models>();
  if (models.size() == 0)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  smtk::model::Model model = models[0];

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());

  // write the file out.
  this->m_op->SetVersion(this->m_currentversion);
  this->m_op->Operate(modelWrapper, opsession.get());

  if (!this->m_op->GetOperateSucceeded())
  {
    std::cerr << "Could not write file \"" << fname << "\".\n";
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  this->parameters()->findFile("filename")->setValue(fname);
  // The model was not modified while writing cmb file.
  // this->addEntityToResult(result, model, MODIFIED);

  return result;
}

const char* WriteOperation::xmlDescription() const
{
  return WriteOperation_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk
