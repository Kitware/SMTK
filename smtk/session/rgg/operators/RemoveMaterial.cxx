//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/RemoveMaterial.h"

#include "smtk/session/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/rgg/RemoveMaterial_xml.h"

using namespace smtk::model;

namespace
{
std::string materialDescriptionLabel = "material_descriptions";
}

namespace smtk
{
namespace session
{
namespace rgg
{

RemoveMaterial::Result RemoveMaterial::operateInternal()
{
  // Access the associated model
  EntityRefArray entities =
    this->parameters()->associatedModelEntities<smtk::model::EntityRefArray>();
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for Remove Materials op");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::EntityRef model = entities[0];

  // Access the material name.
  std::string name = this->parameters()->findAs<attribute::StringItem>("name")->value();

  // The material name is used as the lookup index for both the label and
  // color.
  model.removeStringProperty(name);
  model.removeFloatProperty(name);

  if (model.hasStringProperty("materials"))
  {
    StringList& vec = model.stringProperty("materials");
    vec.erase(std::remove(vec.begin(), vec.end(), name), vec.end());
  }

  if (model.hasStringProperty(materialDescriptionLabel))
  {
    StringList& materialDescriptions = model.stringProperty(materialDescriptionLabel);
    std::stringstream ss;
    ss << "material ( " << name << " )";
    for (std::size_t i = 0; i < materialDescriptions.size(); i++)
    {
      if (materialDescriptions[i].find(ss.str()) != std::string::npos)
      {
        materialDescriptions.erase(materialDescriptions.begin() + i);
        break;
      }
    }
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  result->findComponent("modified")->appendValue(model.component());
  return result;
}

const char* RemoveMaterial::xmlDescription() const
{
  return RemoveMaterial_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
