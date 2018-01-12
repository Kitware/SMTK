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

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

#include "smtk/common/Paths.h"

namespace smtk
{
namespace view
{

ResourcePhraseContent::ResourcePhraseContent()
  : m_resource(nullptr)
  , m_mutability(0)
{
}

ResourcePhraseContent::~ResourcePhraseContent()
{
}

ResourcePhraseContent::Ptr ResourcePhraseContent::setup(
  const smtk::resource::ResourcePtr& rsrc, int mutability)
{
  m_mutability = mutability;
  m_resource = rsrc;
  return shared_from_this();
}

DescriptivePhrasePtr ResourcePhraseContent::createPhrase(
  const smtk::resource::ResourcePtr& rsrc, int mutability, DescriptivePhrase::Ptr parent)
{
  auto result = DescriptivePhrase::create()->setup(DescriptivePhraseType::RESOURCE_SUMMARY, parent);
  auto content = ResourcePhraseContent::create()->setup(rsrc, mutability);
  result->setContent(content);
  return result;
}

std::string ResourcePhraseContent::stringValue(ContentType attr) const
{
  if (!m_resource)
  {
    return std::string();
  }

  switch (attr)
  {
    case PhraseContent::TITLE:
    {
      std::string locn = m_resource->location();
      std::string file = smtk::common::Paths::filename(locn);
      std::string dir = smtk::common::Paths::directory(locn);
      return dir.empty() ? file : (file + " (" + dir + ")");
    }
    break;
    case PhraseContent::SUBTITLE:
      return m_resource->uniqueName();
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

int ResourcePhraseContent::flagValue(ContentType attr) const
{
  if (!m_resource)
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

resource::FloatList ResourcePhraseContent::colorValue(ContentType attr) const
{
  if (!m_resource)
  {
    return resource::FloatList(4, -1.);
  }

  switch (attr)
  {
    case PhraseContent::COLOR:
      return smtk::resource::FloatList(4, 0.0);
      break;
    case PhraseContent::TITLE:
    case PhraseContent::SUBTITLE:
    case PhraseContent::VISIBILITY:
    case PhraseContent::ICON:
    default:
      break;
  }
  smtk::resource::FloatList rgba(4, -1);
  return rgba;
}

bool ResourcePhraseContent::editStringValue(ContentType attr, const std::string& val)
{
  // This should create and call a "set entity property" operator on the
  // related component's name for attr == TITLE.
  (void)attr;
  (void)val;
  return false;
}

bool ResourcePhraseContent::editFlagValue(ContentType attr, int val)
{
  (void)attr;
  (void)val;
  return false;
}

bool ResourcePhraseContent::editColorValue(ContentType attr, const resource::FloatList& val)
{
  // This should create and call a "set entity property" operator on the
  // related component's color for attr == COLOR.
  (void)attr;
  (void)val;
  return false;
}

smtk::resource::ResourcePtr ResourcePhraseContent::relatedResource() const
{
  return m_resource;
}

void ResourcePhraseContent::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // view namespace
} // smtk namespace
