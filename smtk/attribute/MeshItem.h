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
  typedef smtk::mesh::MeshList::const_iterator const_mesh_it;
  typedef smtk::mesh::MeshList::iterator mesh_it;

  smtkTypeMacro(MeshItem);
  virtual ~MeshItem();
  virtual Item::Type type() const;

  std::size_t numberOfRequiredValues() const;
  bool isExtensible() const;
  /// associated item with collection's meshes given \a collectionid and its \a meshset
  bool setValue(const smtk::mesh::MeshSet& meshset);
  bool appendValue(const smtk::mesh::MeshSet&);
  bool appendValues(const smtk::mesh::MeshList&);
  void removeValue(const smtk::mesh::MeshSet&);

  std::size_t numberOfValues() const;
  const smtk::mesh::MeshList& values() const;
  virtual void reset();
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured.
  virtual bool assign(smtk::attribute::ConstItemPtr &sourceItem, unsigned int options = 0);

  const_mesh_it begin() const;
  const_mesh_it end() const;

protected:
  friend class MeshItemDefinition;

  MeshItem(Attribute *owningAttribute, int itemPosition);
  MeshItem(Item *owningItem, int position, int subGroupPosition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
  smtk::mesh::MeshList m_meshValues;

};

  } // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_MeshItem_h
