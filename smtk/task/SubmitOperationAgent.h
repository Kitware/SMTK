//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_SubmitOperationAgent_h
#define smtk_task_SubmitOperationAgent_h

#include "smtk/task/Agent.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
// Forward declarations
namespace attribute
{
class ComponentItem;
class DoubleItem;
class IntItem;
class ReferenceItem;
class StringItem;
} // namespace attribute
namespace task
{

///\brief SubmitOperationAgent verifies that attributes are valid.
///
class SMTKCORE_EXPORT SubmitOperationAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::SubmitOperationAgent);

  SubmitOperationAgent(Task* owningTask);
  ~SubmitOperationAgent() override = default;

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
    Static, //!< Static agent configuration (not port data) provides values.
    Port,   //!< Port data is allowed to modify the parameter whenever updated.
    User    //!< Tasks, ports, and the user are allowed to edit the parameter.
  };

  /// Specify whether an item and its children should be visible to users.
  enum class ItemVisibility
  {
    Off,          //!< Hide this item but not its children.
    RecursiveOff, //!< Recursively hide this item and its children.
    On            //!< Show this item and its children.
  };

  /// Methods for configuring operation parameters mentioned in port-data.
  enum class PortDataHandler
  {
    /// Append objects to an operation parameter/associations.
    ///
    /// If a component is subsequently expunged from a resource or a resource
    /// is released from its manager, it will be removed from the
    /// operation's parameters.
    AddObjects,
    /// Reset an operation parameter/association to the objects present.
    ///
    /// If a component is subsequently expunged from a resource or a resource
    /// is released from its manager, it will be removed from the
    /// operation's parameters.
    SetObjects,
    /// Assign an attribute's item to an operation-parameter's item.
    ///
    /// If the source item is subsequently modified, the operation will be
    /// updated to match the new values.
    AssignFromAttribute,
    /// Find matching attributes in a resource and assign one of its items
    /// to an operation-parameter's item.
    ///
    /// If the source item is subsequently modified, the operation will be
    /// updated to match the new values.
    AssignFromAttributeResource,
    /// Accept any attribute (either as a lone object or inside an attribute
    /// resource) that matches the specification, assigning one of its items
    /// to an operation-parameter's item.
    ///
    /// This is identical to the combination of AssignFromAttribute and
    /// AssignFromAttributeResource.
    ///
    /// If the source item is subsequently modified, the operation will be
    /// updated to match the new values.
    AssignMatchingAttributes
  };

  /// Per-parameter configuration of item values.
  ///
  /// This class directs the agent how to ingest data from any input port(s) of
  /// the parent task. The agent requires port-data to be of type ObjectsInRoles
  /// and stores a nested collection of ParameterSpec instances indexed first by
  /// port name and then by role name. When a port is marked as having updated
  /// port data, ParameterSpec instances describe how to act on the objects
  /// present at a given port under a given role using m_portDataHandler:
  /// + If objects are in an expected role, they may be added to a reference item
  ///   in the operation parameters if (1) they pass the m_resourceTypeName
  ///   and m_componentSelector filters (2) they or their parent resource has a
  ///   matching m_resourceTypeName and (if valid) m_resourceTemplate.
  ///   This also accommodates associating the objects to the operation since
  ///   associations are held in a ReferenceItem that is explicitly named.
  ///   These may be strictly additive (`AddObjects`) – so that no objects are
  ///   removed from the item unless they are expunged – or reset the item on each
  ///   update (`SetObjects`).
  /// + If a port-data object is an attribute (`AssignFromAttribute`) or
  ///   attribute resource (`AssignFromAttributeResource`) with the given
  ///   m_resourceTemplate and it contains attribute instances matching
  ///   m_componentSelector, then an item (or association) may be copied to
  ///   an item (or association) of the operation parameters.
  ///   If you wish both cases to be handled, use `AssignMatchingAttributes`.
  struct ParameterSpec
  {
    /// How to ingest objects present in port data.
    PortDataHandler m_portDataHandler;
    /// Resource type-name specifier (`smtk::attribute::Resource` for now)
    std::string m_resourceTypeName;
    /// Resource schema specifier (or empty when no match is required).
    smtk::string::Token m_resourceTemplate;
    /// Component selector to identify which components in the resource can provide
    /// a value for the parameter (e.g. "attribute[type='Foo']{string['name'='Bar']}")
    std::string m_componentSelector;
    /// A recipe to extract a value from matching components. (For attributes, this is an item path.)
    std::string m_sourceItemPath;
    /// The destination for the extracted value in the operation's parameters (an item path).
    std::string m_targetItemPath;
    /// What should be allowed to modify the parameter.
    ConfiguredBy m_configuredBy = ConfiguredBy::Static;
    /// False by default; true when user has modified the parameter. Skip when true and m_configuredBy == User.
    bool m_userOverride;
    /// Values to assign to the item (UUIDs for reference items) if m_configuredBy == Static.
    nlohmann::json m_values;
    /// Should this item (and potentially its children) be shown or hidden.
    ItemVisibility m_visibility = ItemVisibility::On;
  };

  /// A reference to a ParameterSpec within m_parameters.
  struct ParameterSpecRef
  {
    ParameterSpecRef() = default;
    ParameterSpecRef(
      smtk::string::Token portName,
      smtk::string::Token roleName,
      std::size_t idx,
      bool exp = false)
      : m_portName(portName)
      , m_roleName(roleName)
      , m_specIndex(idx)
      , m_expunge(exp)
    {
    }
    ParameterSpecRef(const ParameterSpecRef&) = default;
    ParameterSpecRef& operator=(const ParameterSpecRef&) = default;
    /// The port on which the input parameter was identified.
    smtk::string::Token m_portName;
    /// The role in which the input component serves.
    smtk::string::Token m_roleName;
    /// An offset into the vector of parameters identifying a single ParameterSpec.
    std::size_t m_specIndex;
    /// True if the objects should be removed rather than added.
    bool m_expunge{ false };

    bool operator==(const ParameterSpecRef& other) const
    {
      // clang-format off
      return
        m_portName == other.m_portName &&
        m_roleName == other.m_roleName &&
        m_specIndex == other.m_specIndex &&
        m_expunge == other.m_expunge;
      // clang-format on
    }

    bool operator<(const ParameterSpecRef& other) const
    {
      // clang-format off
      return m_portName < other.m_portName || (
        m_portName == other.m_portName && (
          m_roleName < other.m_roleName || (
            m_roleName == other.m_roleName && (
              m_specIndex < other.m_specIndex || (
                m_specIndex == other.m_specIndex && m_expunge < other.m_expunge )))));
      // clang-format on
    }
  };

  /// All of the ways the operation accepts port-data into its configuration.
  ///
  /// This specifies how port-data should be ingested. When matching port-data
  /// is detected (during `portDataUpdated()`), then entries are added to
  /// the ParameterWatchMap.
  using ParameterSpecMap = std::unordered_map<
    smtk::string::Token,
    std::unordered_map<smtk::string::Token, std::vector<ParameterSpec>>>;

  /// A set of component UUIDs that provide values to be mapped into parameters.
  ///
  /// This is used by the operation observer to update the operation's configuration.
  using ParameterWatchMap = std::unordered_map<smtk::common::UUID, std::set<ParameterSpecRef>>;

  /// A set of objects from which to assign values.
  using ObjectSet = std::unordered_set<smtk::resource::PersistentObject*>;

  /// Used internally to gather objects contributing to the same parameter.
  using ParameterUpdateMap = std::map<ParameterSpecRef, ObjectSet>;

  /// Convert to/from a ConfigureBy enumerant.
  static smtk::string::Token ConfiguredByToken(ConfiguredBy value);
  static ConfiguredBy ConfiguredByValue(smtk::string::Token token);
  /// Convert to/from an ItemVisibility enumerant.
  static smtk::string::Token ItemVisibilityToken(ItemVisibility value);
  static ItemVisibility ItemVisibilityValue(smtk::string::Token token);
  /// Convert to/from a RunStyle enumerant.
  static smtk::string::Token RunStyleToken(RunStyle value);
  static RunStyle RunStyleValue(smtk::string::Token token);
  /// Convert to/from a PortDataHandler enumerant.
  static smtk::string::Token PortDataHandlerToken(PortDataHandler value);
  static PortDataHandler PortDataHandlerValue(smtk::string::Token token);

  ///\brief Return the current state of the agent.
  ///
  /// This is Unavailable when no resources/attributes/definitions have
  /// been configured for validation; it is Incomplete when at least one
  /// attribute is configured for validation but is not in a valid state;
  /// and Completable when all the configured attributes are valid.
  State state() const override { return m_internalState; }

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return the agent's current configuration.
  Configuration configuration() const override;

  ///\brief Return user-presentable troubleshooting information.
  std::string troubleshoot() const override;

  ///\brief Return port data provided by the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief  Tell the agent that the data on \a port has been updated.
  void portDataUpdated(const Port* port) override;

  /// Return the operation this task requires users to configure and submit.
  smtk::operation::Operation* operation() const { return m_operation.get(); }

  /// Return the manner in which this task expects users to submit the operation.
  RunStyle runStyle() const { return m_runStyle; }

  /// Return the specification for the associations of the operation this task configures.
  ///
  /// Note that if the operation is not intended to auto-configure the associations,
  /// the m_itemPath member of the parameters should be set to "-ignore-" (rather
  /// than empty or populated with the name of the association's ReferenceItem).
  ParameterSpec& associationSpec() { return m_associationSpec; }
  ParameterSpec associationSpec() const { return m_associationSpec; }

  /// Return the list of operation parameters to copy from an input attribute resource.
  ///
  /// The input attribute resource is expected to be provided via an input port.
  const ParameterSpecMap& parameterSpecs() const { return m_parameterSpecs; }

  /// Modify view to hide items specified in task's style
  void configureHiddenItems(smtk::view::ConfigurationPtr view, const nlohmann::json& jItemArray)
    const;

  /// True if the operation has *successfully* run since its parameters were last edited.
  bool runSinceEdited() const { return m_runSinceEdited; }

  /// Force the task into an incomplete state because its input parameters have changed.
  ///
  /// This is invoked by \a configureHiddenItems() if any items are modified.
  /// The returned value is true if the call had any effect (i.e., m_runSinceEdited
  /// was true before the call and false afterward).
  ///
  /// Calling this method will generally result in a state change (to Incomplete)
  /// if true is returned.
  bool setNeedsToRun();

  // bool editableCompletion() const override;

protected:
  /// Receive notification the parent Task's state has changed.
  void taskStateChanged(State prev, State& next) override;

  ///\brief Receive notification that a Task's state has changed.
  ///
  /// This method would allow a Task to delegate children state
  /// calculation to an SubmitOperationAgent instead
  void taskStateChanged(Task* task, State prev, State next) override;

  /// Respond to operations that may change task state.
  int update(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Insert objects that should be mapped to operation parameters into \a parametersToUpdate.
  ///
  /// The ComponentItem passed should be from an operation result's report of created, modified,
  /// or expunged components.
  ///
  /// This returns true if any entries were added to parametersToUpdate.
  bool prepareParameterUpdates(
    ParameterUpdateMap& parametersToUpdate,
    const std::shared_ptr<smtk::attribute::ComponentItem>& components,
    bool expunging,
    bool& requireStateCheck);

  /// Invoke copyToParameter on each entry of \a parametersToUpdate.
  ///
  /// In the future, this may fetch other objects from which parameter
  /// values should be drawn (when there is a need to combine values
  /// from multiple objects to set a single operation-parameter's value).
  bool copyToParameters(ParameterUpdateMap& parametersToUpdate);

  /// Copy values from \a objects into the referenced parameter (\a ref ).
  ///
  /// This returns true if any parameters were modified.
  bool copyToParameter(
    const ParameterSpecRef& ref,
    const std::unordered_set<smtk::resource::PersistentObject*>& objects);

  /// Copy values from \a objects into \a targetInt.
  bool copyToIntParameter(
    const ParameterSpec& spec,
    smtk::attribute::IntItem* targetInt,
    const ObjectSet& objects);
  /// Copy values from \a objects into \a targetDouble.
  bool copyToDoubleParameter(
    const ParameterSpec& spec,
    smtk::attribute::DoubleItem* targetDouble,
    const ObjectSet& objects);
  /// Copy values from \a objects into \a targetString.
  bool copyToStringParameter(
    const ParameterSpec& spec,
    smtk::attribute::StringItem* targetString,
    const ObjectSet& objects);
  /// Copy values from \a objects into \a targetReference.
  bool copyToReferenceParameter(
    const ParameterSpec& spec,
    smtk::attribute::ReferenceItem* targetReference,
    const ObjectSet& objects);

  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState();

  ParameterSpecMap m_parameterSpecs;
  ParameterWatchMap m_watching;

  State m_internalState{ State::Unavailable };
  smtk::common::Managers::Ptr m_managers;
  smtk::operation::Observers::Key m_observer;
  ParameterSpec m_associationSpec;
  RunStyle m_runStyle{ RunStyle::Iteratively };
  bool m_runSinceEdited{ false };
  std::shared_ptr<smtk::operation::Operation> m_operation;

  /// Allow users to name a port+role where this agent produces a
  /// set of resources reported as created/modified/expunged
  /// by the agent's most recent execution of its operation.
  /// These member variables are assigned by configure() and, if
  /// m_outputPortName is invalid, are ignored.
  smtk::string::Token m_outputPortName;
  smtk::string::Token m_outputRole;
  std::set<
    std::weak_ptr<smtk::resource::Resource>,
    std::owner_less<std::weak_ptr<smtk::resource::Resource>>>
    m_outputResources;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_SubmitOperationAgent_h
