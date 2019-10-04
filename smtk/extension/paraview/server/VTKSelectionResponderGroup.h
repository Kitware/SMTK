//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_VTKSelectionResponderGroup_h
#define smtk_view_VTKSelectionResponderGroup_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"

#include "smtk/operation/Group.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/common/TypeName.h"

#include <set>
#include <string>
#include <type_traits>

namespace smtk
{
namespace view
{
class Manager;

/**\brief A group of operations that update an SMTK selection based on a VTK selection.
  *
  * All operations registered with this group should be a subclass
  * of vtkSelectionResponder.
  */
class SMTKPVSERVEREXT_EXPORT VTKSelectionResponderGroup : protected smtk::operation::Group
{
public:
  using Operation = smtk::operation::Operation;
  using smtk::operation::Group::contains;
  using smtk::operation::Group::operations;
  using smtk::operation::Group::operationNames;
  using smtk::operation::Group::operationName;
  using smtk::operation::Group::operationLabel;
  using smtk::operation::Group::unregisterOperation;

  static constexpr const char* const type_name = "vtk selection responder";

  VTKSelectionResponderGroup(const std::shared_ptr<smtk::operation::Manager>& manager,
    const std::shared_ptr<smtk::resource::Manager>& resourceManager)
    : Group(type_name, manager)
    , m_resourceManager(resourceManager)
  {
  }

  /// Register an IO operation identified by its class type and the name of the
  /// resource it reads.
  template <typename ResourceType, typename OperationType>
  bool registerOperation();

  /// Given a resource, return the set of operators that are
  /// registered as supporting resources of that type.
  std::set<Operation::Index> operationsForResource(const smtk::resource::ResourcePtr&) const;

  /// Given an operation name, return the resource associated with the operation.
  std::string resourceForOperation(const std::string&) const;

  /// Given an operation index, return the resource associated with the operation.
  std::string resourceForOperation(const Operation::Index&) const;

  std::set<std::string> supportedResources() const;

protected:
  std::weak_ptr<smtk::resource::Manager> m_resourceManager;
};

template <typename ResourceType, typename OperationType>
bool VTKSelectionResponderGroup::registerOperation()
{
  static_assert(std::is_base_of<RespondToVTKSelection, OperationType>::value,
    "Operations assigned to VTKSelectionResponderGroup must inherit RespondToVTKSelection");
  return Group::registerOperation(
    std::type_index(typeid(OperationType)).hash_code(), { smtk::common::typeName<ResourceType>() });
}

} // namespace view
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_view_VTKSelectionResponderGroup_h
