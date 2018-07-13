//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_Metadata_h
#define smtk_operation_Metadata_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/MetadataObserver.h"
#include "smtk/operation/Operation.h"

#include <functional>
#include <set>
#include <string>

namespace smtk
{
namespace operation
{
/// Operations are registered to an operation manager at runtime with an instance
/// of smtk::operation::Metadata. Instances of this class must provide
/// <typeName>, a unique (to the manager) string used to describe the operation
/// within the manager. They must also provide a functor for the creation of the
/// operation.
class SMTKCORE_EXPORT Metadata
{
public:
  using Observer = MetadataObserver;
  using Observers = MetadataObservers;
  using Association = smtk::attribute::ConstReferenceItemDefinitionPtr;

  Metadata(const std::string& typeName, Operation::Index index,
    Operation::Specification specification,
    std::function<std::shared_ptr<smtk::operation::Operation>(void)> createFunctor);

  const std::string& typeName() const { return m_typeName; }
  const Operation::Index& index() const { return m_index; }
  Operation::Specification specification() const { return m_specification; }
  bool acceptsComponent(const smtk::resource::ComponentPtr& c) const
  {
    return m_acceptsComponent(c);
  }
  /**\brief Return the ReferenceItemDefinition to use when finding available operations.
    *
    * While an operation is associated with an entire attribute resource, and
    * that resource may define multiple attribute types, applications should
    * be able to identify whether an operation is suitable for a selection
    * using a single reference item. This method will return that item or,
    * if the operation takes no persistent objects, a null pointer.
    *
    * For now, the item is inferred; it is assumed to be the associationRule()
    * entry of the operator's parameters() definition.
    * A null pointer is returned if there is no association rule.
    * In the future, this may change so that operations can
    * specify how selections should be used to determine suitability.
    */
  Association primaryAssociation() const { return m_primaryAssociation; }

  std::set<std::string> groups() const;

  std::function<std::shared_ptr<smtk::operation::Operation>(void)> create;

private:
  std::string m_typeName;
  Operation::Index m_index;
  Operation::Specification m_specification;
  std::function<bool(const smtk::resource::ComponentPtr&)> m_acceptsComponent;
  Association m_primaryAssociation;
};
}
}

#endif // smtk_operation_Metadata_h
