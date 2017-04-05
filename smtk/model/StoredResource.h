//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_StoredResource_h
#define smtk_model_StoredResource_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/Resource.h"

namespace smtk {
  namespace model {

class SMTKCORE_EXPORT StoredResource : public smtk::common::Resource
{
public:
  static StoredResourcePtr create();
  virtual ~StoredResource();

  std::string url() const;
  bool setURL(const std::string& url, bool isModified = false);

  void markModified(bool isDirty = true);
  bool isModified() const;
  int generation() const;

  bool exists(const std::string& prefix = "") const;

  virtual Resource::Type resourceType() const { return MODEL; }

  bool addEntity(const EntityRef& ent);
  bool removeEntity(const EntityRef& ent);
  const EntityRefs& entities() const { return this->m_entities; }

protected:
  StoredResource();

  void setGeneration(int gen);

  std::string m_url; // The location where this resource is stored.
  int m_generation; // The generation number the last time an entity in the resource was modified.
  int m_urlGeneration; // The generation number at the last time the resource was written to/read from disk.
  EntityRefs m_entities; // The model entities stored by this resource.
};

  } // namespace model
} // namespace smtk

#endif // smtk_model_StoredResource_h
