//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Delete_h
#define smtk_markup_Delete_h

#include "smtk/graph/operators/Delete.h"

#include "smtk/markup/Component.h"
#include "smtk/markup/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include <map>
#include <set>

namespace smtk
{
namespace markup
{

/**\brief Delete components from a resource.
  *
  * This operation subclasses the abstract smtk::graph::Delete
  * and – if you subclass the markup resource – you must subclass
  * this operation as well, overriding operationInternal() and
  * ableToOperate() in order to pass helper methods your resource's
  * Traits object.
  *
  * Given a set, S, of nodes to remove, this operation will examine
  * graph arcs connected to all members of S and, if any are marked
  * with OwnershipSemantics, either: (a) insert dependent nodes into
  * S so that they are also removed or (b) refuse to operate in order
  * to preserve semantic consistency. Which of these is done depends
  * on whether the "delete dependents" parameter is enabled or not.
  *
  * Assuming semantic consistency allows the operation to proceed,
  * 1. a DeletionCleanup query is initialized (if the resource provides
  *    one) for nodes in S that belong to the resource;
  * 2. all of the nodes in S (including additions above) are removed
  *    from their respective graph resources;
  * 3. properties of these nodes are removed;
  * 4. arcs connected to all nodes in S are removed;
  * 5. links to/from all nodes in S are invalidated;
  * 6. any DeletionCleanup queries from step 1 are finalized.
  */
class SMTKMARKUP_EXPORT Delete : public smtk::graph::Delete
{
public:
  smtkTypeMacro(smtk::markup::Delete);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::graph::Delete);
  smtkCreateMacro(smtk::operation::Operation);

  bool ableToOperate() override;

  /// Set/get whether log messages should be suppressed.
  ///
  /// Ephemeral cells created by primitive selections use this
  /// to prevent confusing the user when the selection is
  /// deleted. You should not normally need to use this feature.
  void setSuppressOutput(bool suppress) { m_suppressOutput = suppress; }
  bool suppressOutput() const { return m_suppressOutput; }

protected:
  Delete();
  Delete::Result operateInternal() override;
  const char* xmlDescription() const override;

  Result m_result; // TODO: Let subclass define?
  bool m_suppressOutput{ false };
};

} // namespace markup
} // namespace smtk

#endif
