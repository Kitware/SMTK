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

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/operators/SetProperty.h"

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
      return !!modelComp;
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
      auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
      if (modelComp)
      {
        return modelComp->referenceAs<smtk::model::EntityRef>().name();
      }

      auto attrComp = dynamic_pointer_cast<smtk::attribute::Attribute>(m_component);
      if (attrComp)
      {
        return attrComp->name();
      }

      // We don't know what type of component it is, but we know it's resource type and UUID:
      std::ostringstream txt;
      txt << m_component->resource()->typeName() << " " << m_component->id().toString();
      return txt.str();
    }
    break;
    case PhraseContent::SUBTITLE:
    {
      auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
      if (modelComp)
      {
        return modelComp->flagSummary();
      }

      auto attrComp = dynamic_pointer_cast<smtk::attribute::Attribute>(m_component);
      if (attrComp)
      {
        return attrComp->type();
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
  if (attr == TITLE)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
    if (modelComp)
    {
      auto op = smtk::model::SetProperty::create();
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
  if (attr == COLOR)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(m_component);
    if (modelComp)
    {
      auto op = smtk::model::SetProperty::create();
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
