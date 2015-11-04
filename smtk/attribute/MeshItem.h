//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_MeshItem_h
#define __smtk_attribute_MeshItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include "smtk/mesh/MeshSet.h"

#include <string>
#include <set>
#include <map>

namespace smtk {
  namespace attribute {

/**\brief Provide a way for an attribute to refer to meshsets.
  *
  */
class SMTKCORE_EXPORT MeshItem : public Item
{
public:
  typedef std::map<smtk::common::UUID, smtk::mesh::MeshSet >::const_iterator const_mesh_it;
  typedef std::map<smtk::common::UUID, smtk::mesh::MeshSet >::iterator mesh_it;

  smtkTypeMacro(MeshItem);
  virtual ~MeshItem();
  virtual Item::Type type() const;

  std::size_t numberOfRequiredValues() const;
  bool isExtensible() const;

  bool setValue(const smtk::common::UUID&, const smtk::mesh::MeshSet&);
  bool appendValue(const smtk::common::UUID&, const smtk::mesh::MeshSet&);
  void removeValue(const smtk::common::UUID&, const smtk::mesh::MeshSet&);

  std::size_t numberOfValues() const;
  smtk::mesh::MeshSet value(const smtk::common::UUID&) const;
  virtual void reset();
  virtual void copyFrom(
    const smtk::attribute::ItemPtr sourceItem,
    smtk::attribute::Item::CopyInfo& info);

  const_mesh_it begin() const;
  const_mesh_it end() const;

protected:
  friend class MeshItemDefinition;

  MeshItem(Attribute *owningAttribute, int itemPosition);
  MeshItem(Item *owningItem, int position, int subGroupPosition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
  std::map<smtk::common::UUID, smtk::mesh::MeshSet >m_meshValues;

};

  } // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_MeshItem_h
