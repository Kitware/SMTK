//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtOperationTypeModel.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/attribute/Resource.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/operation/groups/InternalGroup.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"
#include "smtk/view/OperationDecorator.h"
#include "smtk/view/Selection.h"

#include <algorithm>

using smtk::extension::qtOperationLauncher;

qtOperationTypeModel::Item::Item(
  smtk::operation::Operation::Index typeIndex,
  const std::string& typeName,
  const std::string& label,
  Associability associability,
  EditableParameters editability,
  const std::string& buttonLabel,
  const std::string& toolTip,
  int precedence)
  : m_index(typeIndex)
  , m_associability(associability)
  , m_editability(editability)
  , m_shortName(buttonLabel)
  , m_typeName(typeName)
  , m_label(label)
  , m_toolTip(toolTip)
  , m_precedence(precedence)
{
}

bool qtOperationTypeModel::Item::operator<(const Item& other) const
{
  return m_index < other.m_index;
}

qtOperationTypeModel::qtOperationTypeModel(const smtk::view::Information& info, QObject* parent)
  : QAbstractListModel(parent)
{
  const auto* config = info.configuration();
  if (!config)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No configuration for qtOperationTypeModel.");
    return;
  }
  int modelConfigIdx = config->details().findChild("Model");
  const auto& managers = info.get<std::shared_ptr<smtk::common::Managers>>();
  if (!managers)
  {
    return;
  }
  m_operationManager = managers->get<smtk::operation::Manager::Ptr>();
  if (!m_operationManager)
  {
    return;
  }
  // NB: We must set m_operationDecorator before adding an observer
  // to the m_operationManager so any pre-registered operations
  // get filtered/decorated properly.
  m_operationDecorator = managers->get<std::shared_ptr<smtk::view::OperationDecorator>>();
  if (!m_operationDecorator)
  {
    int decoratorConfigIdx = -1;
    if (
      modelConfigIdx >= 0 &&
      (decoratorConfigIdx =
         config->details().child(modelConfigIdx).findChild("OperationDecorator")) >= 0)
    {
      m_operationDecorator = std::make_shared<smtk::view::OperationDecorator>(
        m_operationManager, config->details().child(modelConfigIdx).child(decoratorConfigIdx));
    }
  }
  m_operationManager->metadataObservers().insert(
    [this](const smtk::operation::Metadata& metadata, bool adding) {
      this->metadataUpdate(metadata, adding);
    },
    std::numeric_limits<smtk::operation::MetadataObservers::Priority>::lowest(),
    /* initialize */ true,
    "qtOperationTypeModel metadataUpdate");
  m_viewManager = managers->get<smtk::view::Manager::Ptr>();
  m_selection = managers->get<smtk::view::Selection::Ptr>();

  // Find or create an operation launcher.
  auto* launcher = m_operationManager->launchers()[qtOperationLauncher::type_name]
                     .target<smtk::extension::qt::Launcher>();
  if (!launcher)
  {
    m_operationManager->launchers()[qtOperationLauncher::type_name] =
      smtk::extension::qt::Launcher();
    launcher = m_operationManager->launchers()[qtOperationLauncher::type_name]
                 .target<smtk::extension::qt::Launcher>();
  }
  m_launcher = launcher->get();

  std::string selectionValueLabel;
  if (modelConfigIdx >= 0)
  {
    const auto& modelConfig(config->details().child(modelConfigIdx));
    std::string disableLevel;
    if (modelConfig.attribute("DisableLevel", disableLevel))
    {
      m_disableMismatchLevel = associabilityEnum(disableLevel);
    }
    if (!modelConfig.attribute("SelectionValueLabel", selectionValueLabel))
    {
      int selectionValueInt;
      if (modelConfig.attributeAsInt("SelectionValue", selectionValueInt))
      {
        m_selectionValue = selectionValueInt;
      }
    }
    modelConfig.attributeAsBool("SelectionExact", m_selectionExact);
    bool autorun = false;
    modelConfig.attributeAsBool("Autorun", autorun);
    if (autorun)
    {
      QObject::connect(
        this,
        &qtOperationTypeModel::runOperation,
        this,
        &qtOperationTypeModel::runOperationWithDefaults);
    }
  }
  if (m_selection)
  {
    // Before we add an observer that will invoke methods to
    // evaluate our current state, finish configuring m_selectionValue.
    if (!selectionValueLabel.empty())
    {
      m_selectionValue = m_selection->findOrCreateLabeledValue(selectionValueLabel);
    }
    m_selectionObserverKey = m_selection->observers().insert(
      [this](const std::string& src, std::shared_ptr<smtk::view::Selection> seln) {
        if (!m_operations.empty())
        {
          this->updateSelectionAssociability(src, seln, 0, this->rowCount(QModelIndex()) - 1);
        }
      },
      std::numeric_limits<smtk::view::SelectionObservers::Priority>::lowest(),
      /* initialize */ true,
      "qtOperationTypeModel associability updater");
  }
}

qtOperationTypeModel::Column qtOperationTypeModel::columnEnumFromName(const std::string& colName)
{
  std::string name(colName);
  std::transform(
    name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
  // Trim leading namespacing as needed.
  if (name.substr(0, 22) == "qtoperationtypemodel::")
  {
    name = name.substr(22);
  }
  if (name.substr(0, 8) == "column::")
  {
    name = name.substr(8);
  }
  if (name == "count")
  {
    return Column::Count;
  }
  else if (name == "typeindex")
  {
    return Column::TypeIndex;
  }
  else if (name == "typename")
  {
    return Column::TypeName;
  }
  else if (name == "label")
  {
    return Column::Label;
  }
  else if (name == "buttonlabel")
  {
    return Column::ButtonLabel;
  }
  else if (name == "tooltip")
  {
    return Column::ToolTip;
  }
  else if (name == "widgetaction")
  {
    return Column::WidgetAction;
  }
  else if (name == "associability")
  {
    return Column::Associability;
  }
  else if (name == "editability")
  {
    return Column::Editability;
  }
  // else if (name == "precedence")
  return Column::Precedence;
}

std::string qtOperationTypeModel::columnNameFromEnum(Column enumerant)
{
  switch (enumerant)
  {
    case Column::TypeIndex:
      return "TypeIndex";
    case Column::TypeName:
      return "TypeName";
    case Column::Label:
      return "Label";
    case Column::ButtonLabel:
      return "ButtonLabel";
    case Column::ToolTip:
      return "ToolTip";
    case Column::WidgetAction:
      return "WidgetAction";
    case Column::Associability:
      return "Associability";
    case Column::Editability:
      return "Editability";
    case Column::Precedence:
      return "Precedence";
    case Column::Count:
      break;
  }
  return "Count";
}

int qtOperationTypeModel::rowCount(const QModelIndex& parent) const
{
  (void)parent;
  assert(!parent.isValid());
  return static_cast<int>(m_operations.size());
}

int qtOperationTypeModel::columnCount(const QModelIndex& parent) const
{
  (void)parent;
  assert(!parent.isValid());
  return static_cast<int>(Column::Count);
}

QVariant qtOperationTypeModel::data(const QModelIndex& index, int role) const
{
  (void)role;
  QVariant value;

  int row = index.row();
  int col = index.column();
  if (row < 0 || row >= static_cast<int>(m_operations.size()))
  {
    return value;
  }
  if (col < 0 || col >= static_cast<int>(Column::Count))
  {
    return value;
  }

  switch (static_cast<Column>(col))
  {
    case Column::TypeIndex:
      value = static_cast<qulonglong>(m_operations[row].m_index);
      break;
    case Column::TypeName:
      value = QString::fromStdString(m_operations[row].m_typeName);
      break;
    case Column::Label:
      value = QString::fromStdString(m_operations[row].m_label);
      break;
    case Column::ButtonLabel:
      value = QString::fromStdString(m_operations[row].m_shortName);
      break;
    case Column::ToolTip:
      value = QString::fromStdString(m_operations[row].m_toolTip);
      break;
    case Column::WidgetAction:
      value.setValue(this->actionFor(index));
      break;
    case Column::Associability:
      value = static_cast<int>(m_operations[row].m_associability);
      break;
    case Column::Editability:
      value = static_cast<int>(m_operations[row].m_editability);
      break;
    case Column::Precedence:
      value = m_operations[row].m_precedence;
      break;
    default:
      break;
  }
  return value;
}

QVariant qtOperationTypeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  (void)role;
  QVariant label;
  if (orientation == Qt::Horizontal)
  {
    switch (static_cast<Column>(section))
    {
      case Column::TypeIndex:
        label = "index";
        break;
      case Column::TypeName:
        label = "type_name";
        break;
      case Column::Label:
        label = "label";
        break;
      case Column::WidgetAction:
        label = "action";
        break;
      case Column::Associability:
        label = "associability";
        break;
      case Column::Editability:
        label = "editability";
        break;
      default:
        label = "error";
        break;
    }
  }
  return label;
}

qtOperationAction* qtOperationTypeModel::actionFor(const QModelIndex& index) const
{
  QPointer<qtOperationAction> action;
  int row = index.row();
  if (!index.isValid() || row < 0 || row >= this->rowCount(index.parent()))
  {
    return action;
  }

  if (!m_operations[row].m_action)
  {
    // We need a non-const version of ourself for connections to the action.
    auto* self = const_cast<qtOperationTypeModel*>(this);
    auto metadataIt = m_operationManager->metadata().get<smtk::operation::IndexTag>().find(
      m_operations[row].m_index);
    if (metadataIt != m_operationManager->metadata().get<smtk::operation::IndexTag>().end())
    {
      m_operations[row].m_action = new qtOperationAction(
        m_operationManager,
        m_viewManager,
        metadataIt->index(),
        /* parent */ self);
      // Watch for the action to be activated.
      QObject::connect(
        self->m_operations[row].m_action,
        &qtOperationAction::acceptDefaults,
        self,
        &qtOperationTypeModel::runSendingOperation);
      QObject::connect(
        self->m_operations[row].m_action,
        &qtOperationAction::editParameters,
        self,
        &qtOperationTypeModel::editSendingOperationParameters);
    }
  }
  action = m_operations[row].m_action;
  return action;
}

QModelIndex qtOperationTypeModel::findByTypeIndex(smtk::operation::Operation::Index typeIndex) const
{
  std::size_t sz = m_operations.size() - 1;
  if (
    m_operations.empty() || m_operations[0].m_index > typeIndex ||
    m_operations[sz].m_index < typeIndex)
  {
    return QModelIndex();
  }
  auto it = std::lower_bound(
    m_operations.begin(),
    m_operations.end(),
    typeIndex,
    [](const Item& a, smtk::operation::Operation::Index rhs) { return a.m_index < rhs; });
  if (it == m_operations.end())
  {
    return QModelIndex();
  }
  std::size_t row = it - m_operations.begin();
  return this->index(static_cast<int>(row), 0);
}

void qtOperationTypeModel::setDecorator(
  const std::shared_ptr<smtk::view::OperationDecorator>& decorator)
{
  if (decorator == m_operationDecorator)
  {
    return;
  }
  this->beginResetModel();
  m_operationDecorator = decorator;
  m_operations.clear();
  this->endResetModel();
  if (m_operationManager)
  {
    // TODO: We could be more efficient by not discarding the model
    // and starting over, but that would be a lot of code to maintain
    // for very little benefit.
    const auto& metaIndex = m_operationManager->metadata().get<smtk::operation::IndexTag>();
    for (const auto& metadata : metaIndex)
    {
      this->metadataUpdate(metadata, /*adding*/ true);
    }
  }
}

std::shared_ptr<smtk::view::OperationDecorator> qtOperationTypeModel::decorator() const
{
  return m_operationDecorator;
}

std::shared_ptr<smtk::operation::Operation> qtOperationTypeModel::prepareOperation(
  smtk::operation::Operation::Index typeIndex) const
{
  if (!m_operationManager)
  {
    return nullptr;
  }

  auto operation = m_operationManager->create(typeIndex);
  if (!operation)
  {
    return nullptr;
  }

  if (m_selection)
  {
    m_selection->configureItem(
      operation->parameters()->associations(),
      m_selectionValue,
      m_selectionExact,
      /*clearItem*/ true);
  }
  return operation;
}

void qtOperationTypeModel::runOperationWithDefaults(
  smtk::operation::Operation::Index typeIndex) const
{
  auto operation = this->prepareOperation(typeIndex);
  if (operation)
  {
    if (operation->ableToOperate())
    {
      this->runOperationWithParameters(operation);
    }
    else
    {
      Q_EMIT this->editOperationParameters(typeIndex);
    }
  }
}

void qtOperationTypeModel::runOperationWithParameters(
  const std::shared_ptr<smtk::operation::Operation>& operation) const
{
  if (m_launcher)
  {
    auto resultPromise = (*m_launcher)(operation);
  }
}

void qtOperationTypeModel::runSendingOperation()
{
  // std::cout << "Run sending op " << this->sender()->metaObject()->className() << "\n";
  auto* action = dynamic_cast<qtOperationAction*>(this->sender());
  if (action)
  {
    Q_EMIT this->runOperation(action->operationIndex());
  }
}

void qtOperationTypeModel::editSendingOperationParameters()
{
  auto* action = dynamic_cast<qtOperationAction*>(this->sender());
  if (action)
  {
    this->editOperationParameters(action->operationIndex());
  }
}

void qtOperationTypeModel::metadataUpdate(const smtk::operation::Metadata& metadata, bool adding)
{
  auto metaOverride = smtk::view::OperationDecorator::none();
  if (adding)
  {
    smtk::operation::InternalGroup internalGroup(m_operationManager);
    if (internalGroup.contains(metadata.index()))
    {
      return; // Skip internal operations.
    }
    if (m_operationDecorator)
    {
      metaOverride = m_operationDecorator->at(metadata.index());
      if (!metaOverride.first)
      {
        return; // Skip operations not whitelisted.
      }
    }
  }
  std::string label;
  auto parameterDef =
    smtk::operation::extractParameterDefinition(metadata.specification(), metadata.typeName());
  label = parameterDef ? parameterDef->label() : metadata.typeName();
  std::string toolTip = parameterDef ? parameterDef->briefDescription() : std::string();
  std::string buttonLabel;
  int precedence = -1;
  if (metaOverride.first)
  {
    const auto& meta(metaOverride.second.get());
    if (!meta.m_label.empty())
    {
      label = meta.m_label;
    }
    if (!meta.m_buttonLabel.empty())
    {
      buttonLabel = meta.m_buttonLabel;
    }
    if (!meta.m_toolTip.empty())
    {
      toolTip = meta.m_toolTip;
    }
    precedence = meta.m_precedence;
  }
  Item entry(
    metadata.index(),
    metadata.typeName(),
    label,
    Associability::Unneeded, // Filled in below by a call to updateSelectionAssociability().
    qtOperationTypeModel::editabilityFor(metadata),
    buttonLabel,
    toolTip,
    precedence);
  std::size_t srow =
    std::upper_bound(m_operations.begin(), m_operations.end(), entry) - m_operations.begin();
  int row = static_cast<int>(srow);
  if (adding)
  {
    this->beginInsertRows(QModelIndex(), row, row);
    m_operations.insert(m_operations.begin() + row, entry);
    this->updateSelectionAssociability("initialize", m_selection, row, row);
    this->endInsertRows();
  }
  else
  {
    // TODO: Sanitize row number... if invalid, do not erase.
    this->beginRemoveRows(QModelIndex(), row, row);
    m_operations.erase(m_operations.begin() + row);
    this->endRemoveRows();
  }
}

void qtOperationTypeModel::updateSelectionAssociability(
  const std::string& source,
  const std::shared_ptr<smtk::view::Selection>& selection,
  int first,
  int last)
{
  if (!selection)
  {
    return;
  }
  (void)source;
  const auto& metaIndex = m_operationManager->metadata().get<smtk::operation::IndexTag>();
  int firstChanged = -1;
  int lastChanged = -1;
  bool didEnableDisable = false;
  for (int row = first; row <= last; ++row)
  {
    auto& item = m_operations[row];
    const auto& metaIt = metaIndex.find(item.m_index);
    auto previousAssociability = item.m_associability;
    if (metaIt != metaIndex.end())
    {
      const auto& metadata(*metaIt);
      auto parameterDefinition =
        smtk::operation::extractParameterDefinition(metadata.specification(), metadata.typeName());
      auto associations = parameterDefinition ? parameterDefinition->associationRule() : nullptr;
      if (!associations)
      {
        item.m_associability = Associability::Unneeded;
      }
      else
      {
        std::size_t mismatches = 0;
        std::size_t matches = 0;
        for (const auto& entry : selection->currentSelection())
        {
          if (
            (m_selectionExact && entry.second == m_selectionValue) ||
            (!m_selectionExact && (entry.second & m_selectionValue) == m_selectionValue))
          {
            if (associations->isValueValid(entry.first))
            {
              ++matches;
            }
            else
            {
              ++mismatches;
            }
          }
        }
        std::size_t nn = associations->numberOfRequiredValues();
        if (associations->isExtensible())
        {
          std::size_t mm = associations->maxNumberOfValues();
          if ((mm == 0 || mm >= matches) && (matches >= nn))
          {
            item.m_associability = (mismatches > 0 ? UglyComplete : GoodComplete);
          }
          else if (matches > mm)
          {
            item.m_associability = (mismatches > 0 ? UglyExcess : GoodExcess);
          }
          else
          {
            item.m_associability =
              (mismatches > 0 ? (matches == 0 ? BadIncomplete : UglyIncomplete)
                              : (matches == 0 && nn > 0 ? BadIncomplete : GoodIncomplete));
          }
        }
        else
        {
          if (matches < nn)
          {
            item.m_associability = (mismatches > 0 ? UglyIncomplete : GoodIncomplete);
          }
          else if (matches == nn)
          {
            item.m_associability = (mismatches > 0 ? UglyComplete : GoodComplete);
          }
          else
          {
            item.m_associability = (mismatches > 0 ? UglyExcess : GoodExcess);
          }
        }
      }
    }
    if (item.m_associability != previousAssociability)
    {
      // Update range of rows we will signal as modified.
      if (firstChanged < 0)
      {
        firstChanged = row;
      }
      lastChanged = row;

      // Enable/disable QActions for items that have them.
      if (m_disableMismatchLevel > Associability::BadExcess && item.m_action)
      {
        didEnableDisable = true;
        /* For debugging:
        std::cout
          << "  " << row
          << " " << static_cast<int>(item.m_associability)
          << " " << (item.m_associability > m_disableMismatchLevel ? "T" : "F")
          << " " << item.m_typeName
          << "\n";
          */
        item.m_action->setEnabled(item.m_associability > m_disableMismatchLevel);
      }
    }
  }
  if (firstChanged >= 0)
  {
    auto topLeft = this->index(
      firstChanged,
      static_cast<int>(didEnableDisable ? Column::WidgetAction : Column::Associability));
    auto bottomRight = this->index(lastChanged, static_cast<int>(Column::Associability));
    this->dataChanged(topLeft, bottomRight);
  }
}

smtk::attribute::utility::EditableParameters qtOperationTypeModel::editabilityFor(
  const smtk::operation::Metadata& metadata)
{
  auto parameterDefinition =
    smtk::operation::extractParameterDefinition(metadata.specification(), metadata.typeName());
  EditableParameters p = smtk::attribute::utility::userEditableParameters(parameterDefinition);
  return p;
}
