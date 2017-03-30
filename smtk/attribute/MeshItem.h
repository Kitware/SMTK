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

#include <map>
#include <set>
#include <string>

namespace smtk
{
namespace attribute
{

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
  virtual bool isValid() const;

  std::size_t numberOfRequiredValues() const;
  bool isExtensible() const;
  /// associated item with collection's meshes given \a collectionid and its \a meshset
  bool appendValue(const smtk::mesh::MeshSet&);
  bool appendValues(const smtk::mesh::MeshList&);
  bool appendValues(const smtk::mesh::MeshSets&);
  bool hasValue(const smtk::mesh::MeshSet&) const;

  smtk::mesh::MeshSet value(std::size_t element = 0) const;
  bool setValue(const smtk::mesh::MeshSet& val);
  bool setValue(std::size_t element, const smtk::mesh::MeshSet& val);
  bool removeValue(std::size_t element);
  virtual bool isSet(std::size_t element = 0) const;
  virtual void unset(std::size_t element = 0);

  std::size_t numberOfValues() const;
  bool setNumberOfValues(std::size_t newSize);
  const smtk::mesh::MeshList& values() const;
  virtual void reset();
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured.
  virtual bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0);

  const_mesh_it begin() const;
  const_mesh_it end() const;
  std::ptrdiff_t find(const smtk::mesh::MeshSet& mesh) const;

protected:
  friend class MeshItemDefinition;

  MeshItem(Attribute* owningAttribute, int itemPosition);
  MeshItem(Item* owningItem, int position, int subGroupPosition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
  smtk::mesh::MeshList m_meshValues;
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_MeshItem_h
