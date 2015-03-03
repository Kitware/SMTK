//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_MeshSelectionItem_h
#define __smtk_attribute_MeshSelectionItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <set>
#include <map>

namespace smtk {
  namespace attribute {

class MeshSelectionItemDefinition;

/// Enumeration of mesh values modification type.
enum MeshSelectionMode {
  NONE,             //!< Cancel current operation mode
  RESET,            //!< Reset the existing list)
  MERGE,            //!< Append to the existing list
  SUBTRACT,         //!< Subtract from existing list
  ACCEPT,           //!< Accept the existing list)
  NUM_OF_MODES      //!< The number of mesh selection modes.
};

/**\brief Provide a way for an attribute to refer to mesh entities.
  *
  */
class SMTKCORE_EXPORT MeshSelectionItem : public Item
{
public:
  typedef std::map<smtk::common::UUID, std::set<int> >::const_iterator const_sel_map_it;

  smtkTypeMacro(MeshSelectionItem);
  virtual ~MeshSelectionItem();
  virtual Item::Type type() const;

  void setValues(const smtk::common::UUID&, const std::set<int>&);
  void unionValues(const smtk::common::UUID&, const std::set<int>&);
  void removeValues(const smtk::common::UUID&, const std::set<int>&);
  void setMeshSelectMode(MeshSelectionMode mode)
    { this->m_selectMode = mode; }
  MeshSelectionMode meshSelectMode() const
    {return this->m_selectMode;}
  void setCtrlKeyDown(bool val)
    { this->m_isCtrlKeyDown = val; }
  bool isCtrlKeyDown() const
    {return this->m_isCtrlKeyDown;}

  std::size_t numberOfValues() const;
  const std::set<int>& values(const smtk::common::UUID&);
  virtual void reset();
  virtual void copyFrom(
    const smtk::attribute::ItemPtr sourceItem,
    smtk::attribute::Item::CopyInfo& info);

  const_sel_map_it begin() const;
  const_sel_map_it end() const;

  static std::string selectMode2String(
    MeshSelectionMode m);
  static MeshSelectionMode string2SelectMode(
    const std::string &s);

protected:
  friend class MeshSelectionItemDefinition;

  MeshSelectionItem(Attribute *owningAttribute, int itemPosition);
  MeshSelectionItem(Item *owningItem, int position, int subGroupPosition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
  std::map<smtk::common::UUID, std::set<int> >m_selectionValues;
  MeshSelectionMode m_selectMode;
  bool m_isCtrlKeyDown;
};

  } // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_MeshSelectionItem_h
