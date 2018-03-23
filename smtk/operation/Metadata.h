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
  typedef MetadataObserver Observer;
  typedef MetadataObservers Observers;

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

  std::set<std::string> groups() const;

  std::function<std::shared_ptr<smtk::operation::Operation>(void)> create;

private:
  std::string m_typeName;
  Operation::Index m_index;
  Operation::Specification m_specification;
  std::function<bool(const smtk::resource::ComponentPtr&)> m_acceptsComponent;
};
}
}

#endif // smtk_operation_Metadata_h
