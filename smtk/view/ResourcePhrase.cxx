//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ResourcePhrase.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

ResourcePhrase::ResourcePhrase()
  : m_resource(nullptr)
{
}

ResourcePhrase::~ResourcePhrase()
{
}

ResourcePhrase::Ptr ResourcePhrase::setup(
  const smtk::resource::ResourcePtr& rsrc, int mutability, DescriptivePhrase::Ptr parent)
{
  this->DescriptivePhrase::setup(DescriptivePhraseType::RESOURCE_SUMMARY, parent);
  m_mutability = mutability;
  m_resource = rsrc;
  return shared_from_this();
}

std::string ResourcePhrase::title()
{
  if (!m_resource)
  {
    return std::string();
  }

  return m_resource->location();
}

bool ResourcePhrase::isTitleMutable() const
{
  return m_mutability & TITLE;
}

bool ResourcePhrase::setTitle(const std::string& newTitle)
{
  (void)newTitle;
  return false; // Should return whether title was actually modified.
}

std::string ResourcePhrase::subtitle()
{
  if (!m_resource)
  {
    return std::string();
  }

  return m_resource->uniqueName();
}

smtk::resource::ResourcePtr ResourcePhrase::relatedResource() const
{
  return m_resource;
}

smtk::resource::FloatList ResourcePhrase::relatedColor() const
{
  smtk::resource::FloatList rgba(4, -1);
  return rgba;
}

bool ResourcePhrase::isRelatedColorMutable() const
{
  return m_mutability & COLOR ? true : false;
}

bool ResourcePhrase::setRelatedColor(const smtk::resource::FloatList& rgba)
{
  (void)rgba;
  return false;
}

void ResourcePhrase::setMutability(int whatsMutable)
{
  m_mutability = whatsMutable;
}

} // view namespace
} // smtk namespace
