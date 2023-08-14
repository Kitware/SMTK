//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_SubmitOperation_h
#define smtk_task_SubmitOperation_h

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Resource.h"
#include "smtk/task/Task.h"

#include "smtk/common/Visit.h"

namespace smtk
{
// Forward declarations
namespace attribute
{
class Item;
}
namespace task
{
namespace adaptor
{
class ConfigureOperation;
class ResourceAndRole;
} // namespace adaptor

/**\brief SubmitOperation helps users prepare and optionally run an operation.
  *
  * This task creates an operation, optionally pre-configures a subset of its
  * parameters, and may allow users to run the operation once or repeatedly.
  * See the user's guide for more information on how to configure this operation.
  */
class SMTKCORE_EXPORT SubmitOperation : public Task
{
public:
  smtkTypeMacro(smtk::task::SubmitOperation);
  smtkSuperclassMacro(smtk::task::Task);
  smtkCreateMacro(smtk::task::Task);

  /// Specify how users interact with the operation.
  enum RunStyle
  {
    Iteratively, //!< Users edit parameters and run the operation, possibly repeatedly.
    Once,        //!< Users run the operation once (at which point the task is marked complete).
    OnCompletion //!< Users do not run the operation; the task launches it when marked complete.
  };

  /// Specify what is allowed to configure an operation parameter.
  ///
  /// Note that this enumeration does not include a value for specifying
  /// that an item may *only* be configured by a user. For that behavior,
  /// simply omit the item's path from the task configuration.
  enum class ConfiguredBy
  {
    Task,    //!< Static task configuration (not an adaptor) provides values.
    Adaptor, //!< An adaptor is allowed to modify the parameter whenever it runs.
    User     //!< Tasks, adaptors, and the user are allowed to edit the parameter.
  };

  /// Specify whether an item and its children should be visible to users.
  enum class ItemVisibility
  {
    Off,          //!< Hide this item but not its children.
    RecursiveOff, //!< Recursively hide this item and its children.
    On            //!< Show this item and its children.
  };

  /// Per-parameter configuration of item values.
  struct ParameterSpec
  {
    /// The path to the item to be configured.
    std::string m_itemPath;
    /// True if an item is optional and should be enabled.
    bool m_enable;
    /// If the parameter is a ReferenceItem, the role specifies objects
    /// in the project that are allowed as values in the item.
    std::string m_role;
    /// Values to assign to the item (UUIDs for reference items).
    nlohmann::json m_values;
    /// What should be allowed to modify the parameter.
    ConfiguredBy m_configuredBy;
    /// Should this item (and potentially its children) be shown or hidden.
    ItemVisibility m_visibility;
  };

  /// Convert to/from a ConfigureBy enumerant.
  static smtk::string::Token ConfiguredByToken(ConfiguredBy value);
  static ConfiguredBy ConfiguredByValue(smtk::string::Token token);
  /// Convert to/from an ItemVisibility enumerant.
  static smtk::string::Token ItemVisibilityToken(ItemVisibility value);
  static ItemVisibility ItemVisibilityValue(smtk::string::Token token);
  /// Convert to/from a RunStyle enumerant.
  static smtk::string::Token RunStyleToken(RunStyle value);
  static RunStyle RunStyleValue(smtk::string::Token token);

  /// Signatures of functors that visit resources-by-role predicates.
  using ParameterSpecVisitor = std::function<smtk::common::Visit(ParameterSpec&)>;
  using ConstParameterSpecVisitor = std::function<smtk::common::Visit(const ParameterSpec&)>;

  SubmitOperation();
  SubmitOperation(
    const Configuration& config,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);
  SubmitOperation(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~SubmitOperation() override = default;

  /// Parse configuration information to initialize this instance.
  void configure(const Configuration& config);

  /// We override this method in order to launch the operation when
  /// the task's RunStyle is OnCompletion.
  bool markCompleted(bool completed) override;

  /// Return the operation this task requires users to configure and submit.
  smtk::operation::Operation* operation() const { return m_operation.get(); }

  /// Return the manner in which this task expects users to submit the operation.
  RunStyle runStyle() const { return m_runStyle; }

  /// Visit the specification for each item this task potentially configures.
  smtk::common::Visit visitParameterSpecs(ConstParameterSpecVisitor visitor) const;
  smtk::common::Visit visitParameterSpecs(ParameterSpecVisitor visitor);

  /// Return the specification for the associations of the operation this task configures.
  ///
  /// Note that if the operation is not intended to auto-configure the associations,
  /// the m_itemPath member of the parameters should be set to "-ignore-" (rather
  /// than empty or populated with the name of the association's ReferenceItem).
  ParameterSpec& associationSpec() { return m_associationSpec; }
  ParameterSpec associationSpec() const { return m_associationSpec; }

  /// True if the operation has *successfully* run since its parameters were last edited.
  bool runSinceEdited() const { return m_runSinceEdited; }

  /// Modify view to hide items specified in task's style
  void configureHiddenItems(smtk::view::ConfigurationPtr view, const nlohmann::json& jItemArray)
    const;

protected:
  friend class adaptor::ResourceAndRole;
  friend class adaptor::ConfigureOperation;

  /// Respond to operations that may change task state.
  int update(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Apply the task's parameter specifications to the operation.
  bool applyParameterSpecifications();

  /// Apply a parameter specification to the given item.
  bool applyParameterSpecification(
    const ParameterSpec& spec,
    const std::shared_ptr<smtk::attribute::Item>& item);

  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const;

  smtk::common::Managers::Ptr m_managers;
  smtk::operation::Observers::Key m_observer;
  std::vector<ParameterSpec> m_parameterSpecs;
  ParameterSpec m_associationSpec;
  RunStyle m_runStyle{ RunStyle::Iteratively };
  bool m_runSinceEdited{ false };
  std::shared_ptr<smtk::operation::Operation> m_operation;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_SubmitOperation_h
