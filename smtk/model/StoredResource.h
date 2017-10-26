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
#include "smtk/model/URLDisposition.h"

#include "smtk/resource/Resource.h"

#include <vector>

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT StoredResource : public smtk::resource::Resource
{
public:
  smtkTypeMacro(smtk::model::StoredResource);
  smtkCreateMacro(smtk::model::StoredResource);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  virtual ~StoredResource();

  std::string url() const;
  bool setURL(const std::string& url, bool isModified = false);

  void markModified(bool isDirty = true);
  bool isModified() const;
  int generation() const;

  bool exists(const std::string& prefix = "") const;

  Resource::Type type() const override { return smtk::resource::Resource::MODEL; }
  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override;

  bool addEntity(const EntityRef& ent);
  bool removeEntity(const EntityRef& ent);
  const EntityRefs& entities() const { return this->m_entities; }
  SessionRef session() const;

  size_t addURLDisposition(const URLDisposition& disposition);
  const std::vector<URLDisposition>& dispositions() const { return this->m_dispositions; }
  bool clearDispositions();

protected:
  StoredResource();

  void setGeneration(int gen);

  std::string m_url; // The location where this resource is stored.
  int m_generation;  // The generation number the last time an entity in the resource was modified.
  int
    m_urlGeneration; // The generation number at the last time the resource was written to/read from disk.
  EntityRefs m_entities; // The model entities stored by this resource.
  std::vector<URLDisposition>
    m_dispositions; // Used to plan a write operation; only valid during writes.
};

} // namespace model
} // namespace smtk

#endif // smtk_model_StoredResource_h
