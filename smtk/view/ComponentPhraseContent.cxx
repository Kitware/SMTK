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
#include "smtk/view/Manager.h"
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

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/SetProperty.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ComponentPhraseContent::ComponentPhraseContent()
  : m_mutability(0)
{
}

ComponentPhraseContent::~ComponentPhraseContent() = default;

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

bool ComponentPhraseContent::editable(ContentType contentType) const
{
  if (m_mutability & static_cast<int>(contentType))
  {
    if (contentType == COLOR)
    {
      // All components can have color.
      return true;
    }
    else if (contentType == TITLE)
    {
      if (auto component = m_component.lock())
      {
        auto modelComponent = dynamic_pointer_cast<smtk::model::Entity>(component);
        auto meshComponent = dynamic_pointer_cast<smtk::mesh::Component>(component);
        // Models and meshes may be assigned a name.
        return !!modelComponent || !!meshComponent;
      }
    }
  }
  return false;
}

std::string ComponentPhraseContent::stringValue(ContentType contentType) const
{
  if (auto component = m_component.lock())
  {
    switch (contentType)
    {
      case PhraseContent::TITLE:
      {
        return component->name();
      }
      break;
      case PhraseContent::SUBTITLE:
      {
        auto modelComponent = component->as<smtk::model::Entity>();
        if (modelComponent)
        {
          return modelComponent->flagSummary();
        }

        auto attributeComponent = component->as<smtk::attribute::Attribute>();
        if (attributeComponent)
        {
          return attributeComponent->type();
        }

        auto meshComponent = component->as<smtk::mesh::Component>();
        if (meshComponent)
        {
          std::ostringstream meshSummary;
          auto mesh = meshComponent->mesh();
          if (mesh.isValid())
          {
            meshSummary << mesh.cells().size() << " cells";
          }
          return meshSummary.str();
        }
        return std::string();
      }
      break;

      // We will not provide strings for these:
      case PhraseContent::COLOR:
      case PhraseContent::VISIBILITY:
      case PhraseContent::ICON_LIGHTBG:
      {
        if (DescriptivePhrasePtr location = m_location.lock())
        {
          if (PhraseModelPtr phraseModel = location->phraseModel())
          {
            if (ManagerPtr viewManager = phraseModel->manager())
            {
              // TODO: propagate secondary color instead of hard-coding it here
              return viewManager->objectIcons().createIcon(*relatedObject(), "#000000");
            }
          }
        }
      }
      break;
      case PhraseContent::ICON_DARKBG:
      {
        if (DescriptivePhrasePtr location = m_location.lock())
        {
          if (PhraseModelPtr phraseModel = location->phraseModel())
          {
            if (ManagerPtr viewManager = phraseModel->manager())
            {
              // TODO: propagate secondary color instead of hard-coding it here
              return viewManager->objectIcons().createIcon(*relatedObject(), "#ffffff");
            }
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return std::string();
}

int ComponentPhraseContent::flagValue(ContentType contentType) const
{
  if (auto component = m_component.lock())
  {
    switch (contentType)
    {
      case PhraseContent::COLOR:
      case PhraseContent::TITLE:
      case PhraseContent::SUBTITLE:
      case PhraseContent::VISIBILITY:
      case PhraseContent::ICON_LIGHTBG:
      case PhraseContent::ICON_DARKBG:
      default:
        break;
    }
  }
  return -1;
}

bool ComponentPhraseContent::editStringValue(ContentType contentType, const std::string& val)
{
  if (auto component = m_component.lock())
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
    if (contentType == TITLE)
    {
      smtk::operation::Operation::Ptr op;
      if (auto meshComponent = std::dynamic_pointer_cast<smtk::mesh::Component>(component))
      {
        op = opManager ? opManager->create<smtk::mesh::SetMeshName>()
                       : smtk::mesh::SetMeshName::create();
        ;
        op->parameters()->findString("name")->setValue(val);
      }
      else
      {
        op = opManager ? opManager->create<smtk::operation::SetProperty>()
                       : smtk::operation::SetProperty::create();
        ;
        op->parameters()->findString("name")->setValue("name");
        op->parameters()->findString("string value")->appendValue(val);
      }

      if (op->parameters()->associate(component))
      {
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

bool ComponentPhraseContent::editFlagValue(ContentType contentType, int val)
{
  (void)contentType;
  (void)val;
  return false;
}

smtk::resource::PersistentObjectPtr ComponentPhraseContent::relatedObject() const
{
  if (auto component = this->relatedComponent())
  {
    return component;
  }
  return this->PhraseContent::relatedObject();
}

smtk::resource::ResourcePtr ComponentPhraseContent::relatedResource() const
{
  if (auto component = this->relatedComponent())
  {
    return component->resource();
  }

  return nullptr;
}

smtk::resource::ComponentPtr ComponentPhraseContent::relatedComponent() const
{
  return m_component.lock();
}

void ComponentPhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // view namespace
} // smtk namespace
