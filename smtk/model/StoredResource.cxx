//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/StoredResource.h"

#include <fstream>

namespace smtk {
  namespace model {

StoredResourcePtr StoredResource::create()
{
  return StoredResourcePtr(new StoredResource);
}

StoredResource::StoredResource() :
  m_generation(0), m_urlGeneration(0)
{
}

StoredResource::~StoredResource()
{
}

std::string StoredResource::url() const
{
  return this->m_url;
}

/**\brief Change the URL where this resource is stored.
  *
  * You may also pass \a isModified to indicate whether the in-memory
  * version of the resource is modified relative to the new URL.
  */
bool StoredResource::setURL(const std::string& url, bool isModified)
{
  if (url == this->m_url)
    {
    if (isModified != this->isModified())
      {
      this->markModified(isModified);
      return true;
      }
    return false;
    }
  this->markModified(isModified);
  this->m_url = url;
  return true;
}

/// Returns true when a resource has been modified since it was last saved to its \a url.
bool StoredResource::isModified() const
{
  return this->m_urlGeneration == this->m_generation;
}

/// Indicate the in-memory resource has been modified since it was last saved to its \a url.
void StoredResource::markModified(bool isDirty)
{
  if (isDirty)
    {
    ++this->m_generation;
    }
  else
    {
    this->m_urlGeneration = this->m_generation;
    }
}

/// Return the generation number of the resource
int StoredResource::generation() const
{
  return this->m_generation;
}

/**\brief Return whether the resource is already stored at its URL.
  *
  * If the URL is a relative path, \a prefix is prepended to it before giving up.
  */
bool StoredResource::exists(const std::string& prefix) const
{
  if (!this->m_url.empty())
    {
    std::ifstream tryToOpen(this->m_url);
    if (tryToOpen.good())
      {
      return true;
      }

    if (!prefix.empty())
      {
      std::ifstream tryToOpen2(prefix + this->m_url);
      if (tryToOpen2.good())
        {
        return true;
        }
      }
    }
  return false;
}

/**\brief Add an entity to this resource.
  *
  * Returns true when the entity was not already stored by this resource
  * and false otherwise.
  * When true is returned, the resource generation number is also
  * incremented by calling markModified(true).
  */
bool StoredResource::addEntity(const EntityRef& ent)
{
  if (this->m_entities.insert(ent).second)
    {
    this->markModified(true);
    return true;
    }
  return false;
}

/**\brief Remove an entity from this resource.
  *
  * Returns true when the entity was removed from this resource
  * and false otherwise (i.e., it was not stored by this resource).
  * When true is returned, the resource generation number is also
  * incremented by calling markModified(true).
  */
bool StoredResource::removeEntity(const EntityRef& ent)
{
  if (this->m_entities.erase(ent) > 0)
    {
    this->markModified(true);
    return true;
    }
  return false;
}

/// An internal method for restoring generation numbers; call markModified() instead of this.
void StoredResource::setGeneration(int gen)
{
  this->m_generation = gen;
}

  } // namespace model
} // namespace smtk
