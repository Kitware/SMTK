//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/AddMaterial.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/rgg/AddMaterial_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult AddMaterial::operateInternal()
{
  smtk::model::OperatorResult result;
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for add Material op");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }
  smtk::model::EntityRef model = entities[0];

  smtk::attribute::StringItemPtr nameI = this->findString("name");
  smtk::attribute::StringItemPtr labelI = this->findString("label");
  smtk::attribute::DoubleItemPtr colorI = this->findDouble("color");
  std::string name = nameI->value();
  std::string label = labelI->value();
  smtk::model::FloatList color = smtk::model::FloatList(colorI->begin(), colorI->end());
  model.setStringProperty(name, label);
  model.setFloatProperty(name, color);

  if (!model.hasStringProperty("materials"))
  {
    std::string nm = "No Cell Material";
    model.setStringProperty(nm, nm);
    model.setFloatProperty(nm, { 1, 1, 1, 1 });

    std::vector<std::string> materials = { nm, name };
    model.setStringProperty("materials", materials);
  }
  else
  {
    model.stringProperty("materials").push_back(name);
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  this->addEntityToResult(result, model, MODIFIED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::AddMaterial, rgg_add_material,
  "add material", AddMaterial_xml, smtk::bridge::rgg::Session);
