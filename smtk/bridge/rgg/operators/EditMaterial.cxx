//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditMaterial.h"

#include "smtk/bridge/rgg/Material.h"
#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/EditMaterial_xml.h"

#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditMaterial::operateInternal()
{
  // Access the associated model
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for Edit Material op");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }
  smtk::model::EntityRef model = entities[0];

  // Access the material label (a short form of the name; mostly unused).
  std::string label = this->specification()->findAs<attribute::StringItem>("label")->value();

  // Access the material color.
  smtk::attribute::DoubleItemPtr colorI =
    this->specification()->findAs<attribute::DoubleItem>("color");
  smtk::model::FloatList color = smtk::model::FloatList(colorI->begin(), colorI->end());

  // Construct an instance of our material class, which converts to/from a
  // string representation.
  Material material;

  // Access the material name.
  material.m_name = this->specification()->findAs<attribute::StringItem>("name")->value();

  // Access the material temperature in Kelvin.
  material.m_temperature =
    this->specification()->findAs<attribute::DoubleItem>("temperature")->value();

  // Access the material thermal expansion coefficient (set to 0 by default).
  material.m_thermalExpansion =
    this->specification()->findAs<attribute::DoubleItem>("thermalExpansion")->value();

  // Access the material density. This value is qualified by density type
  // (below).
  material.m_density = this->specification()->findAs<attribute::DoubleItem>("density")->value();

  // Access the density type (atoms/barn-cm or g/cm^3).
  material.m_densityType =
    this->specification()->findAs<attribute::StringItem>("densityType")->value();

  // Access the material composition type (weight fractions, atom fractions,
  // atom densities or weight densities).
  material.m_compositionType =
    this->specification()->findAs<attribute::StringItem>("compositionType")->value();

  // Each component has a name and an associated content value (weight/atom
  // fraction/density).
  smtk::attribute::StringItemPtr componentsI =
    this->specification()->findAs<attribute::StringItem>("component");
  material.m_components.insert(
    material.m_components.begin(), componentsI->begin(), componentsI->end());

  smtk::attribute::DoubleItemPtr contentI =
    this->specification()->findAs<attribute::DoubleItem>("content");
  material.m_content.insert(material.m_content.begin(), contentI->begin(), contentI->end());

  // If the number of component names does not match the number of content
  // values, something went wrong.
  if (material.m_components.size() != material.m_content.size())
  {
    smtkErrorMacro(this->log(), "Size mismatch between components and contents.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  // The material name is used as the lookup index for both the label and
  // color.
  model.setStringProperty(material.m_name, label);
  model.setFloatProperty(material.m_name, color);

  // The name is stored using the label "materials". If it does not yet exist,
  // we create it and populate it with a default "No Cell Material" material.
  if (!model.hasStringProperty("materials"))
  {
    std::string nm = "No Cell Material";
    model.setStringProperty(nm, nm);
    model.setFloatProperty(nm, { 1, 1, 1, 1 });

    std::vector<std::string> materialVec = { nm, material.m_name };
    model.setStringProperty("materials", materialVec);
  }
  else
  {
    model.stringProperty("material").push_back(material.m_name);
  }

  // To avoid collision with the preexisting mechanisms for material
  // description, we construct a new string property with the label
  // "material_descriptions" that holds the SON descriptions of all of our
  // material.
  if (!model.hasStringProperty(Material::label))
  {
    // If the property does not yet exist, create it and seed it with the
    // current material description.
    model.setStringProperty(Material::label, std::vector<std::string>(1, material));
  }
  else
  {
    // If the property does exist, we search for a description of the current
    // material. If one already exists, we overwrite it with our new
    // description. Otherwise, we append our new material description to the
    // property list.
    StringList& materialDescriptions = model.stringProperty(Material::label);
    bool found = false;
    std::stringstream ss;
    ss << "material ( " << material.m_name << " )";
    for (std::size_t i = 0; i < materialDescriptions.size(); i++)
    {
      if (materialDescriptions[i].find(ss.str()) != std::string::npos)
      {
        found = true;
        materialDescriptions[i] = material;
      }
    }
    if (!found)
    {
      model.stringProperty(Material::label).push_back(material);
    }
  }

  auto result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  this->addEntityToResult(result, model, MODIFIED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditMaterial,
  rgg_edit_material, "edit material", EditMaterial_xml, smtk::bridge::rgg::Session);
