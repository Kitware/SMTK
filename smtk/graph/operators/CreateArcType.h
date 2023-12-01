//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_CreateArcType_h
#define smtk_graph_CreateArcType_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/evaluators/OwnersOf.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include <map>
#include <set>

namespace smtk
{
namespace graph
{

/**\brief Add a new run-time arc type to a resource.
  *
  * This operation doesn't create any arcs but rather creates a
  * new *type* of arcs.
  *
  * Upon completion, the resource is marked as modified but no
  * components are modified.
  */
class SMTKCORE_EXPORT CreateArcType : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::graph::CreateArcType);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkCreateMacro(smtk::operation::Operation);

  /// The CreateArcType operation will register any arc type you create
  /// with the ArcCreator operation-group. By default, it will register
  /// the smtk::graph::CreateArc operation with the arc type. If your
  /// application wishes to use other operations (see the markup::Resource
  /// for an example), you can insert a non-null
  /// `std::shared_ptr<ArcCreationOperation>` object into the
  /// smtk::common::Managers object that all operations are provided.
  /// If it exists, CreateArcType will ask it what operation to register
  /// with the ArcCreator group.
  struct ArcCreationOperation
  {
    /// Return the index of an operation which should be used to create
    /// arcs of the given \a arcType along with the item-name for the
    /// ReferenceItem holding the arc's destination endpoint.
    virtual CreateArcType::Index operationForArcType(const std::string& arcType) = 0;
    virtual std::string destinationItemNameForArcType(const std::string& arcType) = 0;
  };

protected:
  CreateArcType();
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace graph
} // namespace smtk

#endif
