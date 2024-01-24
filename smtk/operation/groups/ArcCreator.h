//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_ArcCreator_h
#define smtk_operation_ArcCreator_h

#include "smtk/CoreExports.h"

#include "smtk/common/TypeName.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include <string>
#include <unordered_set>

namespace smtk
{
namespace operation
{
class Manager;

/**\brief A group holding operations which can connect a pair of Components instances.
  *
  * All operations added to this group must
  * + accept an association to a component serving as the "source" or "from"
  *   node of the arc; and
  * + have a ReferenceItem whose name is "to" accepts a component to serve as
  *   the "target" or "to" node of the arc.
  *
  * Also, operations in this group *may* have a StringItem named "arc type".
  * This exists so that a single operation may be used to create arcs of many
  * types. Each type of arc that an operation may create should be registered with
  * the group.
  */
class SMTKCORE_EXPORT ArcCreator : protected Group
{
public:
  static constexpr const char* const type_name = "arcCreator";

  ArcCreator(std::shared_ptr<smtk::operation::Manager> manager)
    : Group(type_name, manager)
    , m_arcDestinationItemName(manager)
  {
  }

  std::string arcDestinationItemNameForOperation(const std::string& typeName) const;
  std::string arcDestinationItemNameForOperation(const Operation::Index& index) const;
  smtk::attribute::ReferenceItemDefinition::Ptr arcDestinationItemDefinitionForOperation(
    const std::string& typeName) const;
  smtk::attribute::ReferenceItemDefinition::Ptr arcDestinationItemDefinitionForOperation(
    const Operation::Index& index) const;

  template<typename OperationType>
  bool registerOperation(
    const std::set<std::string>& arcTypes,
    const std::string& arcDestinationItemName = s_defaultArcDestinationItemName);

  bool registerOperation(
    Operation::Index operationIndex,
    const std::set<std::string>& arcTypes,
    const std::string& arcDestinationItemName = s_defaultArcDestinationItemName);

  using Group::unregisterOperation;

  /// Return the set of all arc types which have operations that can create them.
  std::set<std::string> allArcTypes() const;

  /// Return the set of all arc types, including the operation(s) that can create them.
  std::set<std::pair<std::string, Operation::Index>> allArcCreators() const;

  /// Given an ordered pair of objects, return operations that can create an arc between them.
  std::unordered_set<Operation::Index> matchingOperations(
    const smtk::resource::PersistentObject& from,
    const smtk::resource::PersistentObject& to) const;

protected:
  /// A sub-group holding the name of the destination-component ReferenceItem.
  class ArcDestinationItemName : public smtk::operation::Group
  {
  public:
    static constexpr const char* const type_name = "arcDestinationItemName";

    ArcDestinationItemName(std::shared_ptr<smtk::operation::Manager> manager)
      : Group(type_name, manager)
    {
    }
  };

  ArcDestinationItemName m_arcDestinationItemName;
  static const std::string s_defaultArcDestinationItemName;
};

template<typename OperationType>
bool ArcCreator::registerOperation(
  const std::set<std::string>& arcTypes,
  const std::string& arcDestinationItemName)
{
  Operation::Index index = std::type_index(typeid(OperationType)).hash_code();
  return this->registerOperation(index, arcTypes, arcDestinationItemName);
}

} // namespace operation
} // namespace smtk

#endif // smtk_operation_ArcCreator_h
