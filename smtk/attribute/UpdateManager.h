//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_UpdateManager_h
#define smtk_attribute_UpdateManager_h

#include "smtk/attribute/update/AttributeUpdateFactory.h"
#include "smtk/attribute/update/ItemUpdateFactory.h"
#include "smtk/attribute/update/ResourceUpdateFactory.h"

#include "smtk/string/Token.h"

#include <map>

namespace smtk
{
namespace attribute
{

/**\brief Manage the attribute system.
  *
  */
class SMTKCORE_EXPORT UpdateManager
{
public:
  smtkTypeMacroBase(smtk::attribute::UpdateManager);
  smtkCreateMacro(smtk::attribute::UpdateManager);
  virtual ~UpdateManager() = default;

  /// Return a factory holding methods to update entire resources
  /// at a time (i.e., those that do not have a simple mapping from
  /// old to new attribute definitions and instances).
  update::ResourceUpdateFactory& resourceUpdaters() { return m_resourceUpdaters; }
  const update::ResourceUpdateFactory& resourceUpdaters() const { return m_resourceUpdaters; }

  /// Return a factory holding methods to update entire attributes
  /// at a time (i.e., those that do not have a simple mapping from
  /// old to new item definitions and instances).
  ///
  /// The input parameter is the name of a resource's template-type.
  /// \sa Resource::templateType()
  update::AttributeUpdateFactory& attributeUpdaters(const smtk::string::Token& resourceTemplate)
  {
    // This will create a new factory if none exist.
    return m_attributeUpdaters[resourceTemplate];
  }
  const update::AttributeUpdateFactory& attributeUpdaters(
    const smtk::string::Token& resourceTemplate) const
  {
    // This will return an immutably blank factory if there is no match.
    static thread_local update::AttributeUpdateFactory blank;
    auto it = m_attributeUpdaters.find(resourceTemplate);
    if (it == m_attributeUpdaters.end())
    {
      return blank;
    }
    return it->second;
  }

  /// Return a factory holding methods to update items of an attribute
  /// (i.e., those that do not have a simple mapping from old to new item definitions).
  ///
  /// The input parameters are the name of a resource's template-type
  /// and the type of an attribute::Definition.
  update::ItemUpdateFactory& itemUpdaters(
    smtk::string::Token resourceTemplate,
    smtk::string::Token attributeType)
  {
    // This will create a new factory if none exist.
    auto key = std::make_pair(resourceTemplate, attributeType);
    return m_itemUpdaters[key];
  }
  const update::ItemUpdateFactory& itemUpdaters(
    smtk::string::Token resourceTemplate,
    smtk::string::Token attributeType) const
  {
    auto key = std::make_pair(resourceTemplate, attributeType);
    // This will return an immutably blank factory if there is no match.
    static thread_local update::ItemUpdateFactory blank;
    auto it = m_itemUpdaters.find(key);
    if (it == m_itemUpdaters.end())
    {
      return blank;
    }
    return it->second;
  }

protected:
  using AttributeUpdateKey = smtk::string::Token;
  using ItemUpdateKey = std::pair<smtk::string::Token, smtk::string::Token>;
  UpdateManager() = default;

  update::ResourceUpdateFactory m_resourceUpdaters;
  std::map<AttributeUpdateKey, update::AttributeUpdateFactory> m_attributeUpdaters;
  std::map<ItemUpdateKey, update::ItemUpdateFactory> m_itemUpdaters;
};

} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_UpdateManager_h
