//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ResourcePhraseContent.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

#include "smtk/common/Paths.h"

namespace smtk
{
namespace view
{

ResourcePhraseContent::ResourcePhraseContent() = default;

ResourcePhraseContent::~ResourcePhraseContent() = default;

ResourcePhraseContent::Ptr ResourcePhraseContent::setup(
  const smtk::resource::ResourcePtr& rsrc,
  int mutability)
{
  m_mutability = mutability;
  m_resource = rsrc;
  return shared_from_this();
}

DescriptivePhrasePtr ResourcePhraseContent::createPhrase(
  const smtk::resource::ResourcePtr& rsrc,
  int mutability,
  DescriptivePhrase::Ptr parent)
{
  auto result = DescriptivePhrase::create()->setup(DescriptivePhraseType::RESOURCE_SUMMARY, parent);
  auto content = ResourcePhraseContent::create()->setup(rsrc, mutability);
  content->setLocation(result);
  result->setContent(content);
  return result;
}

std::string ResourcePhraseContent::stringValue(ContentType contentType) const
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
          return "New Resource";
        }
        return name;
      }
      break;
      case PhraseContent::TITLE:
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
          name = "New Resource";
        }
        return name + " (" + (locn.empty() ? dir : locn) + ")";
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
        if (DescriptivePhrasePtr location = m_location.lock())
        {
          if (location->phraseModel())
          {
            if (ManagerPtr viewManager = location->phraseModel()->manager())
            {
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
          if (location->phraseModel())
          {
            if (ManagerPtr viewManager = location->phraseModel()->manager())
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
  return std::string();
}

int ResourcePhraseContent::flagValue(ContentType contentType) const
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

bool ResourcePhraseContent::editStringValue(ContentType contentType, const std::string& val)
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

bool ResourcePhraseContent::editFlagValue(ContentType contentType, int val)
{
  (void)contentType;
  (void)val;
  return false;
}

smtk::resource::PersistentObjectPtr ResourcePhraseContent::relatedObject() const
{
  if (auto resource = this->relatedResource())
  {
    return resource;
  }
  return this->PhraseContent::relatedObject();
}

smtk::resource::ResourcePtr ResourcePhraseContent::relatedResource() const
{
  return m_resource.lock();
}

void ResourcePhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // namespace view
} // namespace smtk
