//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/operators/SetMeshName.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/operators/SetProperty.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ComponentPhraseContent::ComponentPhraseContent()
  : m_component(nullptr)
  , m_mutability(0)
{
}

ComponentPhraseContent::~ComponentPhraseContent()
{
}

ComponentPhraseContent::Ptr ComponentPhraseContent::setup(
  const smtk::resource::ComponentPtr& component, int mutability)
{
  m_component = component;
  m_mutability = mutability;
  return shared_from_this();
}

DescriptivePhrasePtr ComponentPhraseContent::createPhrase(
  const smtk::resource::ComponentPtr& component, int mutability, DescriptivePhrasePtr parent)
{
  auto result =
    DescriptivePhrase::create()->setup(DescriptivePhraseType::COMPONENT_SUMMARY, parent);
  auto content = ComponentPhraseContent::create()->setup(component, mutability);
  result->setContent(content);
  content->setLocation(result);
  return result;
}

bool ComponentPhraseContent::editable(ContentType attr) const
{
  if (m_mutability & static_cast<int>(attr))
  {
    if (attr == TITLE || attr == COLOR)
    {
      auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
      auto meshComp = dynamic_pointer_cast<smtk::mesh::Component>(m_component);
      // Models may be assigned a color and a name; meshes may be assigned a name.
      return !!modelComp || (attr == TITLE && !!meshComp);
    }
  }
  return false;
}

std::string ComponentPhraseContent::stringValue(ContentType attr) const
{
  if (!m_component)
  {
    return std::string();
  }

  switch (attr)
  {
    case PhraseContent::TITLE:
    {
      return m_component->name();
    }
    break;
    case PhraseContent::SUBTITLE:
    {
      auto modelComp = m_component->as<smtk::model::Entity>();
      if (modelComp)
      {
        return modelComp->flagSummary();
      }

      auto attrComp = m_component->as<smtk::attribute::Attribute>();
      if (attrComp)
      {
        return attrComp->type();
      }

      auto meshComp = m_component->as<smtk::mesh::Component>();
      if (meshComp)
      {
        std::ostringstream meshSummary;
        meshSummary << meshComp->mesh().cells().size() << " cells";
        return meshSummary.str();
      }
      return std::string();
    }
    break;

    // We will not provide strings for these:
    case PhraseContent::COLOR:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    default:
      break;
  }
  return std::string();
}

int ComponentPhraseContent::flagValue(ContentType attr) const
{
  if (!m_component)
  {
    return -1;
  }

  switch (attr)
  {
    case PhraseContent::COLOR:
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    // This should return non-default values once we allow icons to be registered
    // for components by their metadata.
    default:
      break;
  }
  return -1;
}

resource::FloatList ComponentPhraseContent::colorValue(ContentType attr) const
{
  if (!m_component)
  {
    return resource::FloatList({ 0., 0., 0., -1.0 });
  }

  switch (attr)
  {
    case PhraseContent::COLOR:
    {
      auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(m_component);
      if (ent)
      {
        return ent->referenceAs<smtk::model::EntityRef>().color();
      }
    }
    break;
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    default:
      break;
  }
  smtk::resource::FloatList rgba({ 0., 0., 0., -1.0 });
  return rgba;
}

bool ComponentPhraseContent::editStringValue(ContentType attr, const std::string& val)
{
  // Lets try to get the local operation manager
  auto dp = this->location();
  smtk::operation::ManagerPtr opManager;
  if (dp != nullptr)
  {
    auto model = dp->phraseModel();
    if (model != nullptr)
    {
      opManager = model->operationManager();
    }
  }
  if (attr == TITLE)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
    if (modelComp)
    {
      smtk::model::SetProperty::Ptr op;
      if (opManager)
      {
        op = opManager->create<smtk::model::SetProperty>();
      }
      else
      {
        op = smtk::model::SetProperty::create();
      }

      if (op->parameters()->associate(modelComp))
      {
        op->parameters()->findString("name")->setValue("name");
        op->parameters()->findString("string value")->appendValue(val);
        auto res = op->operate();
        if (res->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          return true;
        }
      }
    }
    auto meshComp = std::dynamic_pointer_cast<smtk::mesh::Component>(m_component);
    if (meshComp)
    {
      smtk::mesh::SetMeshName::Ptr op;
      if (opManager)
      {
        op = opManager->create<smtk::mesh::SetMeshName>();
      }
      else
      {
        op = smtk::mesh::SetMeshName::create();
      }
      if (op->parameters()->associate(meshComp))
      {
        op->parameters()->findString("name")->setValue(val);
        auto res = op->operate();
        if (res->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool ComponentPhraseContent::editFlagValue(ContentType attr, int val)
{
  (void)attr;
  (void)val;
  return false;
}

bool ComponentPhraseContent::editColorValue(ContentType attr, const resource::FloatList& val)
{
  // Lets try to get the local operation manager
  auto dp = this->location();
  smtk::operation::ManagerPtr opManager;
  if (dp != nullptr)
  {
    auto model = dp->phraseModel();
    if (model != nullptr)
    {
      opManager = model->operationManager();
    }
  }
  if (attr == COLOR)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
    if (modelComp)
    {
      smtk::model::SetProperty::Ptr op;
      if (opManager)
      {
        op = opManager->create<smtk::model::SetProperty>();
      }
      else
      {
        op = smtk::model::SetProperty::create();
      }
      if (op->parameters()->associate(modelComp))
      {
        op->parameters()->findString("name")->setValue("color");
        op->parameters()->findDouble("float value")->setValues(val.begin(), val.end());
        auto res = op->operate();
        if (res->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
        {
          return true;
        }
      }
    }
  }
  return false;
}

smtk::resource::ResourcePtr ComponentPhraseContent::relatedResource() const
{
  if (!m_component)
  {
    return nullptr;
  }

  return m_component->resource();
}

smtk::resource::ComponentPtr ComponentPhraseContent::relatedComponent() const
{
  return m_component;
}

void ComponentPhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // view namespace
} // smtk namespace
