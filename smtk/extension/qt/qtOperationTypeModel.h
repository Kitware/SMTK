//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtOperationTypeModel_h
#define smtk_extension_qt_qtOperationTypeModel_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtOperationLauncher.h"

#include <QAbstractListModel>
#include <QPointer>

#include "smtk/attribute/utility/Queries.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/MetadataObserver.h"
#include "smtk/operation/Operation.h"
#include "smtk/view/Information.h"
#include "smtk/view/SelectionObserver.h"

class qtOperationAction;
namespace smtk
{
namespace view
{
class OperationDecorator;
}
} // namespace smtk

/**\brief A QAbstractListModel populated with SMTK operation types.
  *
  * The columns of the model are fixed by the Column enumeration.
  * Operations are ordered by their type index.
  * By using a QSortFilterProxyModel, you can subset and sort operations
  * using a variety of techniques.
  *
  * The class constructor accepts configuration via an
  * smtk::view::Information instance; it expects to find a
  * configuration component named "Model" in `info.configuration()->details()`.
  *
  * If you configure the model with a selection instance (i.e., if
  * the Information type-container holds a selection), then
  * it will add a selection observer that updates the Associability
  * column with a measure of how well each operation fits the current
  * selection.
  * The configuration should include either a "SelectionValueLabel" (string) or
  * a "SelectionValue" (integer), that indicate the value(s) in the selection
  * map which should be used to determine operation associability.
  * If "SelectionExact" (a boolean) is true, then only the exact number
  * specified by "SelectionValueLabel" or "SelectionValue" is accepted.
  * Otherwise, any value that has a superset of the selection-value's bits
  * is considered.
  *
  * By default, when a selection is provided the model will disable
  * actions in its WidgetAction column for operations with an associability
  * of GoodIncomplete or lower. You can override this with the
  * "DisableLevel" configuration attribute (see the example below).
  *
  * There are two ways to configure the model with an operation decorator:
  * 1. Insert a std::shared_ptr<OperationDecorator> into a
  *    std::shared_ptr<smtk::common::Managers> instance in the Information
  *    object passed to configure the model; or
  * 2. Include XML (deserialized into a Configuration::Component) that
  *    specifies the operation decorator.
  * If both are specified, the instance in the smtk::common::Managers
  * will take precedence.
  * Only operations that match entries in the decorator will be
  * presented – and they may have their names, tool-tips, etc.
  * overridden.
  *
  * If you specify "Autorun" as true (the default is false), then
  * the model will launch operations automatically rather than
  * only emitting the runOperation() signal. (The runOperation()
  * signal will always be emitted.)
  *
  * Below is an example showing an XML configuration snippet
  * that can be included in a view::Configuration::Component
  * that will be used by the model.
  *
  * ```xml
  *   <Model
  *     DisableLevel="GoodIncomplete"
  *     SelectionValueLabel="selected"
  *     SelectionValue="1"
  *     SelectionExact="false"
  *     Autorun="false"
  *     >
  *     <OperationDecorator ... see class documentation for details ... />
  *   </Model>
  * ```
  */
class SMTKQTEXT_EXPORT qtOperationTypeModel : public QAbstractListModel
{
  Q_OBJECT
public:
  qtOperationTypeModel(const smtk::view::Information& info, QObject* parent = nullptr);
  ~qtOperationTypeModel() override = default;

  /// The columns of the model.
  enum class Column
  {
    TypeIndex,     //!< The type index that uniquely identifies the operation.
    TypeName,      //!< The operation's type-name (not its label).
    Label,         //!< The operation's label (either from its XML or an override).
    ButtonLabel,   //!< A version of the Label column with linebreaks to squarify it.
    ToolTip,       //!< The operation's tool-tip (either from its XML or an override).
    WidgetAction,  //!< A QWidgetAction object that users can activate to run the operation.
    Associability, //!< Whether the active selection associates properly with the operation.
    Editability,   //!< Whether the operation has user-editable parameters and default values.
    Precedence,    //!< An application-provided ordering for sorting.
    // ----
    Count //!< Not a valid value; when cast to an integer it is the number of columns.
  };

  /// Convert to/from a Column enumerant.
  static Column columnEnumFromName(const std::string& colName);
  static std::string columnNameFromEnum(Column enumerant);

  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /// Return the action for a particular operation-type.
  qtOperationAction* actionFor(const QModelIndex& index) const;
  template<typename OperationType>
  qtOperationAction* actionFor() const
  {
    return this->actionFor(this->findByTypeIndex<OperationType>());
  }

  /// Return the index of the row with the given operation type-index.
  QModelIndex findByTypeIndex(smtk::operation::Operation::Index typeIndex) const;
  template<typename OperationType>
  QModelIndex findByTypeIndex() const
  {
    return this->findByTypeIndex(std::type_index(typeid(OperationType)).hash_code());
  }

  /// Set/get the operation decorator used to override operation labels, tooltips, etc..
  void setDecorator(const std::shared_ptr<smtk::view::OperationDecorator>& decorator);
  std::shared_ptr<smtk::view::OperationDecorator> decorator() const;

  /// Create an operation of the given type and associate it with the current selection.
  std::shared_ptr<smtk::operation::Operation> prepareOperation(
    smtk::operation::Operation::Index typeIndex) const;

public Q_SLOTS:
  /**\brief Run an operation of the given type with its default parameters.
    *
    * This calls prepareOperation to create an instance of the operation and
    * associate it to the current selection (if available). Then it calls
    * runOperationWithParameters() with the operation it created.
    */
  virtual void runOperationWithDefaults(smtk::operation::Operation::Index typeIndex) const;

  /// Run the provided operation using a launcher.
  virtual void runOperationWithParameters(
    const std::shared_ptr<smtk::operation::Operation>& operation) const;

Q_SIGNALS:
  //@{
  /**\brief One of these signals is emitted when users activate an action this model exposes.
    *
    * This model has a WidgetAction column whose value is a qtOperationAction* (which
    * inherits QWidgetAction).
    * That class emits two Q_SIGNALS: one for editing an operation's parameters and
    * another for running the operation with the default parameters.
    * The qtOperationTypeModel forwards these signals (from all of its actions)
    * to this signal, which observers can watch for *any* row's operation action to be
    * activated.
    *
    * As a convenience, applications that use qtOperationTypeModel may connect
    * the runOperation signal to the runOperationWithDefaults() slot that this
    * class also provides. If your application chooses not to connect this
    * signal/slot pair, then you can use these signals to perform custom
    * processing before running the operation.
    * Configure the view to make the connection by setting the "Autorun" parameter
    * in the view configuration to true.
    */
  void runOperation(smtk::operation::Operation::Index);
  void editOperationParameters(smtk::operation::Operation::Index) const;
  //@}

protected Q_SLOTS:
  void runSendingOperation();
  void editSendingOperationParameters();

protected:
  using EditableParameters = smtk::attribute::utility::EditableParameters;

  /**\brief A measure of how well the view::Selection matches an operation's associations.
    *
    * This enumeration can be thought of as the tensor product of 2 states:
    * + Whether all, some, or no selected objects can be associated to the operation.
    * + Whether the operation's associations are
    *   + disallowed (unneeded) because the operation doesn't allow *any* associations;
    *   + allowed but only partially complete (unmet) because the operation needs more objects;
    *   + allowed and sufficient (fulfilled) because the minimum is met and maximum not exceeded; or
    *   + allowed but only in part (excess) because more objects are selected than
    *     the operation allows.
    *
    * | Match || Unneeded        | Unmet            | Fulfilled        | Excess           |
    * +-------++-----------------+------------------+------------------+------------------+
    * | All   || Unneeded        | GoodIncomplete   | GoodComplete     | GoodExcess       |
    * | Some  || Unneeded        | UglyIncomplete   | UglyComplete     | UglyExcess       |
    * | None  || Unneeded        | BadIncomplete    | BadComplete      | BadExcess        |
    *
    * Note that the enumerants above are sorted from least desirable to most desirable
    * rather than ordered in the table: excess selection is regarded as the least
    * desirable.
    *
    * In the table above,
    *
    * + "Good" means that all of the selection is applicable to the operation's association rule;
    * + "Ugly" means that part of the selection is applicable to the operation's association rule;
    * + "Bad" means none of the selection is applicable to the operation's association rule;
    * + "Unneeded" means that the operation has no association rule and is always applicable;
    * + "Complete" means that the selection satisfied the minimum and maximum constraints of the
    *    operation's association rule;
    * + "Incomplete" means that the selection fails the minimum constraint of the operation's
    *    association rule; and
    * + "Excess" means that the selection fails the maximum constraint of the operation's
    *    association rule.
    *
    * These terms are combined to form descriptions of each specific table entry.
    * Note that "BadComplete" means that the operation's association rule has a minimum
    * constraint of 0 and hence can behave like "Unneeded."
    */
  enum Associability
  {
    BadExcess,
    UglyExcess,
    GoodExcess,
    Unneeded,
    BadIncomplete,
    UglyIncomplete,
    GoodIncomplete,
    BadComplete,
    UglyComplete,
    GoodComplete
  };

  /// A type-conversion operation to cast Associability enumerants to strings.
  inline std::string associabilityName(const Associability& aa)
  {
    static std::array<std::string, 10> names{ "BadExcess",      "UglyExcess",    "GoodExcess",
                                              "Unneeded",       "BadIncomplete", "UglyIncomplete",
                                              "GoodIncomplete", "BadComplete",   "UglyComplete",
                                              "GoodComplete" };
    return names[static_cast<int>(aa)];
  }

  /// A type-conversion operation to cast strings to Associability enumerants.
  inline Associability associabilityEnum(const std::string& aa)
  {
    std::string associabilityName(aa);
    std::transform(
      associabilityName.begin(),
      associabilityName.end(),
      associabilityName.begin(),
      [](unsigned char c) { return std::tolower(c); });
    // Trim potential prefixes:
    if (associabilityName.substr(0, 22) == "qtoperationtypemodel::")
    {
      associabilityName = associabilityName.substr(22);
    }
    if (associabilityName.substr(0, 15) == "associability::")
    {
      associabilityName = associabilityName.substr(15);
    }
    // Match enumerants
    if (associabilityName == "badexcess")
    {
      return Associability::BadExcess;
    }
    else if (associabilityName == "uglyexcess")
    {
      return Associability::UglyExcess;
    }
    else if (associabilityName == "goodexcess")
    {
      return Associability::GoodExcess;
    }
    else if (associabilityName == "unneeded")
    {
      return Associability::Unneeded;
    }
    else if (associabilityName == "badincomplete")
    {
      return Associability::BadIncomplete;
    }
    else if (associabilityName == "uglyincomplete")
    {
      return Associability::UglyIncomplete;
    }
    else if (associabilityName == "goodincomplete")
    {
      return Associability::GoodIncomplete;
    }
    else if (associabilityName == "badcomplete")
    {
      return Associability::BadComplete;
    }
    else if (associabilityName == "uglycomplete")
    {
      return Associability::UglyComplete;
    }
    // else if (associabilityName == "goodcomplete")
    return Associability::GoodComplete;
  }

  /**\brief A row in the model's table.
    */
  struct Item
  {
    Item(
      smtk::operation::Operation::Index typeIndex,
      const std::string& typeName,
      const std::string& label,
      Associability associability,
      EditableParameters editability,
      const std::string& buttonLabel,
      const std::string& toolTip,
      int precedence);
    Item(const Item&) = default;
    bool operator<(const Item& other) const;

    smtk::operation::Operation::Index m_index;
    Associability m_associability{ Unneeded };
    EditableParameters m_editability{ EditableParameters::Mandatory };
    std::string m_shortName;
    std::string m_typeName;
    std::string m_label;
    std::string m_toolTip;
    int m_precedence;
    mutable QPointer<qtOperationAction> m_action;
  };

  /// Invoked when operation metadata is added or removed.
  void metadataUpdate(const smtk::operation::Metadata& metadata, bool adding);
  /// Invoked when the selection changes in order to update associability.
  void updateSelectionAssociability(
    const std::string& source,
    const std::shared_ptr<smtk::view::Selection>& selection,
    int first,
    int last);
  /// Invoked when creating an Item entry
  static EditableParameters editabilityFor(const smtk::operation::Metadata& metadata);

  std::vector<Item> m_operations;
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  std::shared_ptr<smtk::view::Manager> m_viewManager;
  std::shared_ptr<smtk::view::Selection> m_selection;
  std::shared_ptr<smtk::view::OperationDecorator> m_operationDecorator;
  /**\brief Which value or bits in the selection map determine associations?
    *
    * Set this by specifying either:
    *
    * + a selection label (see smtk::view::Selection::valueForLabel) as a
    *   string attribute named SelectionLabel on the model's configuration; or
    * + a selection value as an integer attribute named SelectionValue on
    *   the model's configuration.
    *
    * If both are present, SelectionLabel is preferred.
    * The default is a value of 1.
    * If the smtk::view::Selection instance passed to the model does not have
    * a selection label you specify, a new value will be allocated.
    */
  int m_selectionValue{ 0x01 };
  /**\brief Match the exact value or simply require the value's bits to match?
    *
    * Set this by specifying a boolean value in the SelectionExact attribute
    * on the model's configuration.
    * The default is false (i.e., only require bits from the value to match).
    */
  bool m_selectionExact{ false };
  /**\brief When associability is *worse* (lower) than this, disable its corresponding action.
    *
    * Set this by specifying an Associability enumerant as the string value
    * of an attribute named "DisableLevel" of the model's configuration.
    * The default is "BadExcess" (i.e., no operations will be disabled).
    */
  Associability m_disableMismatchLevel{ Associability::GoodIncomplete };
  smtk::operation::MetadataObservers::Key m_modelObserverKey;
  smtk::view::SelectionObservers::Key m_selectionObserverKey;
  std::string m_secondaryColor{ "#ffffff" };

  smtk::extension::qtOperationLauncher* m_launcher{ nullptr };

private:
  Q_DISABLE_COPY(qtOperationTypeModel);
};

#endif // smtk_extension_qt_qtOperationTypeModel_h
