//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/view/PhraseContent.h"

#include "smtk/view/Manager.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

#include "smtk/common/Paths.h"

namespace smtk
{
namespace project
{
namespace view
{

PhraseContent::PhraseContent() = default;

PhraseContent::~PhraseContent() = default;

PhraseContent::Ptr PhraseContent::setup(const smtk::project::ProjectPtr& project, int mutability)
{
  m_mutability = mutability;
  m_resource = project;
  m_isProject = true;
  return shared_from_this();
}

PhraseContent::Ptr PhraseContent::setup(const smtk::resource::ResourcePtr& resource, int mutability)
{
  m_mutability = mutability;
  m_resource = resource;
  m_isProject = false;
  return shared_from_this();
}

smtk::view::DescriptivePhrasePtr PhraseContent::createPhrase(
  const smtk::project::ProjectPtr& project,
  int mutability,
  smtk::view::DescriptivePhrase::Ptr parent)
{
  auto result = smtk::view::DescriptivePhrase::create()->setup(
    smtk::view::DescriptivePhraseType::RESOURCE_SUMMARY, parent);
  auto content = PhraseContent::create()->setup(project, mutability);
  content->setLocation(result);
  result->setContent(content);
  return result;
}

smtk::view::DescriptivePhrasePtr PhraseContent::createPhrase(
  const smtk::resource::ResourcePtr& resource,
  int mutability,
  smtk::view::DescriptivePhrase::Ptr parent)
{
  auto result = smtk::view::DescriptivePhrase::create()->setup(
    smtk::view::DescriptivePhraseType::RESOURCE_SUMMARY, parent);
  auto content = smtk::project::view::PhraseContent::create()->setup(resource, mutability);
  content->setLocation(result);
  result->setContent(content);
  return result;
}

std::string PhraseContent::stringValue(ContentType contentType) const
{
  if (auto resource = m_resource.lock())
  {
    switch (contentType)
    {
      case PhraseContent::EDITABLE_TITLE:
      {
        std::string name = resource->name();
        if (name.empty())
        {
          name = "New Project";
        }
        return name;
      }
      break;
      case PhraseContent::TITLE:
      {
        if (m_isProject)
        {
          std::string name = resource->name();
          std::string locn = resource->location();
          std::string file = smtk::common::Paths::filename(locn);
          std::string dir = smtk::common::Paths::directory(locn);
          if (dir.empty())
          {
            dir = smtk::common::Paths::currentDirectory();
          }
          if (name.empty())
          {
            name = "New Project";
          }
          return name + " (" + (locn.empty() ? dir : locn) + ")";
        }
        else
        {
          std::string name = resource->name();
          const std::string& role = detail::role(resource);
          if (name.empty())
          {
            name = "New Resource";
          }
          return role + ": " + name;
        }
      }
      break;
      case PhraseContent::SUBTITLE:
        return resource->typeName();
        break;

      // We will not provide strings for these:
      case PhraseContent::COLOR:
      case PhraseContent::VISIBILITY:
      case PhraseContent::ICON_LIGHTBG:
      {
        if (smtk::view::DescriptivePhrasePtr location = m_location.lock())
        {
          if (location->phraseModel())
          {
            if (smtk::view::ManagerPtr viewManager = location->phraseModel()->manager())
            {
              return viewManager->objectIcons().createIcon(*relatedObject(), "#000000");
            }
          }
        }
      }
      break;
      case PhraseContent::ICON_DARKBG:
      {
        if (smtk::view::DescriptivePhrasePtr location = m_location.lock())
        {
          if (location->phraseModel())
          {
            if (smtk::view::ManagerPtr viewManager = location->phraseModel()->manager())
            {
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
  // else if (auto resource = m_resource.lock())
  return std::string();
}

int PhraseContent::flagValue(ContentType contentType) const
{
  if (auto resource = m_resource.lock())
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

bool PhraseContent::editStringValue(ContentType contentType, const std::string& val)
{
  if (auto resource = m_resource.lock())
  {
    switch (contentType)
    {
      case PhraseContent::TITLE:
        return resource->setName(val);
        break;
      case PhraseContent::SUBTITLE:
        return resource->setLocation(val);
        break;

      // We will not provide strings for these:
      case PhraseContent::COLOR:
      case PhraseContent::VISIBILITY:
      case PhraseContent::ICON_LIGHTBG:
      case PhraseContent::ICON_DARKBG:
      default:
        break;
    }
  }
  return false;
}

bool PhraseContent::editFlagValue(ContentType contentType, int val)
{
  (void)contentType;
  (void)val;
  return false;
}

smtk::resource::ResourcePtr PhraseContent::relatedResource() const
{
  return m_resource.lock();
}

smtk::project::ProjectPtr PhraseContent::relatedProject() const
{
  return m_isProject ? std::static_pointer_cast<smtk::project::Project>(m_resource.lock())
                     : smtk::project::ProjectPtr();
}

void PhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}
} // namespace view
} // namespace project
} // namespace smtk
