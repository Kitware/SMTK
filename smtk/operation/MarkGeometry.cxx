//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace operation
{

MarkGeometry::MarkGeometry(const smtk::geometry::ResourcePtr& resource)
  : m_resource(resource)
{
}

void MarkGeometry::markModified(const smtk::resource::PersistentObjectPtr& object)
{
  if (object)
  {
    if (m_resource)
    {
      m_resource->visitGeometry([&object](
        std::unique_ptr<geometry::Geometry>& provider) { provider->markModified(object); });
    }
    else
    {
      smtk::geometry::ResourcePtr rsrc;
      auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(object);
      if (comp)
      {
        rsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(comp->resource());
        if (rsrc)
        {
          rsrc->visitGeometry([&object](
            std::unique_ptr<geometry::Geometry>& provider) { provider->markModified(object); });
        }
      }
      else if ((rsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(object)))
      {
        rsrc->visitGeometry([&object](
          std::unique_ptr<geometry::Geometry>& provider) { provider->markModified(object); });
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger::instance(),
          "MarkGeometry must be constructed with a resource to be used this way.");
      }
    }
  }
}

void MarkGeometry::erase(const smtk::resource::PersistentObjectPtr& object)
{
  if (object)
  {
    if (m_resource)
    {
      m_resource->visitGeometry([&object](
        std::unique_ptr<geometry::Geometry>& provider) { provider->erase(object->id()); });
    }
    else
    {
      smtk::geometry::ResourcePtr rsrc;
      auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(object);
      if (comp)
      {
        rsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(comp->resource());
        if (rsrc)
        {
          rsrc->visitGeometry([&object](
            std::unique_ptr<geometry::Geometry>& provider) { provider->erase(object->id()); });
        }
      }
      else if ((rsrc = std::dynamic_pointer_cast<smtk::geometry::Resource>(object)))
      {
        rsrc->visitGeometry([&object](
          std::unique_ptr<geometry::Geometry>& provider) { provider->erase(object->id()); });
      }
      else
      {
        smtkErrorMacro(smtk::io::Logger::instance(),
          "MarkGeometry must be constructed with a resource to be used this way.");
      }
    }
  }
}

void MarkGeometry::erase(const smtk::common::UUID& objectId)
{
  if (!m_resource)
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "MarkGeometry must be constructed with a resource to use this method.");
    return;
  }
  if (m_resource)
  {
    m_resource->visitGeometry(
      [&objectId](std::unique_ptr<geometry::Geometry>& provider) { provider->erase(objectId); });
  }
}

void MarkGeometry::markModified(const smtk::attribute::ReferenceItemPtr& item)
{
  if (!item)
  {
    return;
  }
  for (auto it = item->begin(); it != item->end(); ++it)
  {
    this->markModified(*it);
  }
}

void MarkGeometry::erase(const smtk::attribute::ReferenceItemPtr& item)
{
  if (!item)
  {
    return;
  }
  for (auto it = item->begin(); it != item->end(); ++it)
  {
    this->erase(*it);
  }
}

void MarkGeometry::markResult(const smtk::operation::Operation::Result& result)
{
  if (!result)
  {
    // Warn here?
    return;
  }

  auto created = result->findComponent("created");
  this->markModified(created);

  auto modified = result->findComponent("modified");
  this->markModified(modified);

  auto expunged = result->findComponent("expunged");
  this->erase(expunged);
}

} // namespace operation
} // namespace smtk
