//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentPhrase.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ComponentPhrase::ComponentPhrase()
  : m_component(nullptr)
  , m_mutability(0)
{
}

ComponentPhrase::~ComponentPhrase()
{
}

ComponentPhrase::Ptr ComponentPhrase::setup(
  const smtk::resource::ComponentPtr& component, int mutability, DescriptivePhrase::Ptr parent)
{
  this->DescriptivePhrase::setup(DescriptivePhraseType::COMPONENT_SUMMARY, parent);
  m_component = component;
  m_mutability = mutability;
  return shared_from_this();
}

std::string ComponentPhrase::title()
{
  if (!m_component)
  {
    return std::string();
  }

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
  txt << m_component->resource()->uniqueName() << " " << m_component->id().toString();
  return txt.str();
}

bool ComponentPhrase::isTitleMutable() const
{
  return m_mutability & TITLE ? true : false;
}

bool ComponentPhrase::setTitle(const std::string& text)
{
  (void)text;
  return false; // Should return whether title was actually modified.
}

std::string ComponentPhrase::subtitle()
{
  if (!m_component)
  {
    return std::string();
  }

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

smtk::resource::ResourcePtr ComponentPhrase::relatedResource() const
{
  if (!m_component)
  {
    return nullptr;
  }

  return m_component->resource();
}

smtk::resource::ComponentPtr ComponentPhrase::relatedComponent() const
{
  return m_component;
}

smtk::resource::FloatList ComponentPhrase::relatedColor() const
{
  smtk::resource::FloatList rgba(4, -1);
  return rgba;
}

bool ComponentPhrase::isRelatedColorMutable() const
{
  return m_mutability & COLOR ? true : false;
}

bool ComponentPhrase::setRelatedColor(const smtk::resource::FloatList& rgba)
{
  (void)rgba;
  return false;
}

void ComponentPhrase::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // view namespace
} // smtk namespace
