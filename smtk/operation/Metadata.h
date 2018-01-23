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
#include "smtk/operation/NewOp.h"

#include <functional>
#include <set>
#include <string>

namespace smtk
{
namespace operation
{
/// Operators are registered to an operator manager at runtime with an instance
/// of smtk::operator::Metadata. Instances of this class must provide
/// <uniqueName>, a unique (to the manager) string used to describe the operator
/// within the manager. They must also provide a functor for the creation of the
/// operator.
class SMTKCORE_EXPORT Metadata
{
public:
  typedef MetadataObserver Observer;
  typedef MetadataObservers Observers;

  Metadata(const std::string& uniqueName, NewOp::Index index, NewOp::Specification specification,
    std::function<std::shared_ptr<smtk::operation::NewOp>(void)> createFunctor);

  const std::string& uniqueName() const { return m_uniqueName; }
  const NewOp::Index& index() const { return m_index; }
  NewOp::Specification specification() const { return m_specification; }
  bool acceptsComponent(const smtk::resource::ComponentPtr& c) const
  {
    return m_acceptsComponent(c);
  }

  std::function<std::shared_ptr<smtk::operation::NewOp>(void)> create;

private:
  std::string m_uniqueName;
  NewOp::Index m_index;
  NewOp::Specification m_specification;
  std::function<bool(const smtk::resource::ComponentPtr&)> m_acceptsComponent;
};
}
}

#endif // smtk_operation_Metadata_h
