//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/operators/Write.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Write_xml.h"
#include "smtk/session/mesh/operators/Export.h"

#include "smtk/common/Paths.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/model/json/jsonResource.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace mesh
{

bool Write::ableToOperate()
{
  if (!this->smtk::operation::XMLOperation::ableToOperate())
  {
    return false;
  }

  if (this->parameters()->associations()->numberOfValues() < 1)
  {
    return false;
  }

  return true;
}

Write::Result Write::operateInternal()
{
  auto resourceItem = this->parameters()->associations();

  smtk::session::mesh::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->objectValue());

  // Serialize resource into a set of JSON records:
  nlohmann::json j = resource;
  if (j.is_null())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the model associated with this resource
  smtk::model::Models models =
    resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  if (models.size() < 1)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::model::Model model = models[0];

  std::string meshFilename = resource->session()->topology(model)->m_collection->location();

  if (meshFilename.empty())
  {
    meshFilename = smtk::common::Paths::stem(resource->location()) + ".h5m";
  }

  j["Mesh URL"] = meshFilename;

  {
    std::ofstream file(resource->location());
    if (!file.good())
    {
      smtkErrorMacro(log(), "Unable to open \"" << resource->location() << "\" for writing.");
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    file << j.dump(2);
    file.close();
  }

  // Create an export operator
  smtk::session::mesh::Export::Ptr exportOp = smtk::session::mesh::Export::create();
  exportOp->parameters()->findFile("filename")->setValue(meshFilename);

  // Set the entity association
  exportOp->parameters()->associateEntity(model);

  // Execute the operation
  smtk::operation::Operation::Result exportOpResult = exportOp->operate();

  // Test for success
  return (exportOpResult->findInt("outcome")->value() ==
           static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    ? this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED)
    : this->createResult(smtk::operation::Operation::Outcome::FAILED);
}

const char* Write::xmlDescription() const
{
  return Write_xml;
}

bool write(const smtk::resource::ResourcePtr& resource)
{
  Write::Ptr write = Write::create();
  write->parameters()->associate(resource);
  Write::Result result = write->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Write::Outcome::SUCCEEDED));
}

} // namespace mesh
} // namespace session
} // namespace smtk
