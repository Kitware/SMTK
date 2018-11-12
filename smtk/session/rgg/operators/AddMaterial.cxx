//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/AddMaterial.h"

#include "smtk/session/rgg/Material.h"
#include "smtk/session/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/rgg/AddMaterial_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace rgg
{

AddMaterial::Result AddMaterial::operateInternal()
{
  // Access the associated model
  auto entities = this->parameters()->associatedModelEntities<smtk::model::EntityRefArray>();
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for Add Materials op");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  smtk::model::EntityRef model = entities[0];

  // Access the material label (a short form of the name; mostly unused).
  std::string label = this->parameters()->findAs<attribute::StringItem>("label")->value();

  // Access the material color.
  smtk::attribute::DoubleItemPtr colorI =
    this->parameters()->findAs<attribute::DoubleItem>("color");
  smtk::model::FloatList color = smtk::model::FloatList(colorI->begin(), colorI->end());

  // Construct an instance of our material class, which converts to/from a
  // string representation.
  Material material;

  // Access the material name.
  material.setName(this->parameters()->findAs<attribute::StringItem>("name")->value());

  // Access the material temperature in Kelvin.
  material.setTemperature(
    this->parameters()->findAs<attribute::DoubleItem>("temperature")->value());

  // Access the material thermal expansion coefficient (set to 0 by default).
  material.setThermalExpansion(
    this->parameters()->findAs<attribute::DoubleItem>("thermalExpansion")->value());

  // Access the material density. This value is qualified by density type
  // (below).
  material.setDensity(this->parameters()->findAs<attribute::DoubleItem>("density")->value());

  // Access the density type (atoms/barn-cm or g/cm^3).
  material.setDensityType(
    this->parameters()->findAs<attribute::StringItem>("densityType")->value());

  // Access the material composition type (weight fractions, atom fractions,
  // atom densities or weight densities).
  material.setCompositionType(
    this->parameters()->findAs<attribute::StringItem>("compositionType")->value());

  // Each component has a name and an associated content value (weight/atom
  // fraction/density).
  smtk::attribute::StringItemPtr componentsI =
    this->parameters()->findAs<attribute::StringItem>("component");
  for (auto comp = componentsI->begin(); comp != componentsI->end(); ++comp)
  {
    material.addComponent(*comp);
  }

  smtk::attribute::DoubleItemPtr contentI =
    this->parameters()->findAs<attribute::DoubleItem>("content");
  for (auto content = contentI->begin(); content != contentI->end(); ++content)
  {
    material.addContent(*content);
  }

  // The material name is used as the lookup index for both the label and
  // color.
  model.setStringProperty(material.name(), label);
  model.setFloatProperty(material.name(), color);

  // The name is stored using the label "materials". If it does not yet exist,
  // we create it and populate it with a default "No Cell Material" material.
  if (!model.hasStringProperty("materials"))
  {
    std::string nm = "No Cell Material";
    model.setStringProperty(nm, nm);
    model.setFloatProperty(nm, { 1, 1, 1, 1 });

    std::vector<std::string> materialVec = { nm, material.name() };
    model.setStringProperty("materials", materialVec);
  }
  else
  {
    model.stringProperty("materials").push_back(material.name());
  }

  // To avoid collision with the preexisting mechanisms for material
  // description, we construct a new string property with the label
  // "material_descriptions" that holds the SON descriptions of all of our
  // materials.
  if (!model.hasStringProperty(Material::label))
  {
    // If the property does not yet exist, create it and seed it with the
    // current material description.
    model.setStringProperty(Material::label, std::vector<std::string>(1, material));
  }
  else
  {
    // If the property does exist, we search for a description of the current
    // material. If one already exists, we exit with failure. Otherwise, we
    // append our new material description to the
    // property list.
    StringList& materialDescriptions = model.stringProperty(Material::label);
    std::stringstream ss;
    ss << "material ( " << material.name() << " )";
    for (std::size_t i = 0; i < materialDescriptions.size(); i++)
    {
      if (materialDescriptions[i].find(ss.str()) != std::string::npos)
      {
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
    }

    model.stringProperty(Material::label).push_back(material);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");
    modifiedItem->appendValue(model.component());
  }

  return result;
}

const char* AddMaterial::xmlDescription() const
{
  return AddMaterial_xml;
}

} // namespace rgg
} //namespace session
} // namespace smtk
