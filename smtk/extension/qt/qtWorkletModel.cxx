//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtWorkletModel.h"

#include "smtk/io/Logger.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/operation/groups/InternalGroup.h"
#include "smtk/project/Project.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"
#include "smtk/view/OperationDecorator.h"
#include "smtk/view/Selection.h"

#include <QByteArray>
#include <QMimeData>

#include <algorithm>

using smtk::extension::qtOperationLauncher;

qtWorkletModel::Item::Item(const std::shared_ptr<smtk::task::Worklet>& worklet)
  : m_worklet(worklet)
{
}

bool qtWorkletModel::Item::operator<(const Item& other) const
{
  return m_worklet->name() < other.m_worklet->name();
}

qtWorkletModel::qtWorkletModel(const smtk::view::Information& info, QObject* parent)
  : QAbstractListModel(parent)
{
  const auto* config = info.configuration();
  if (!config)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No configuration for qtWorkletModel.");
    return;
  }
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
  m_operationObserverKey = m_operationManager->observers().insert(
    [this](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result result) {
      if (event == smtk::operation::EventType::DID_OPERATE)
      {
        this->workletUpdate(op, result);
      }
      return 0;
    },
    /* priority */ 0,
    /* initialize */ true,
    "qtWorkletModel updater");
  m_viewManager = managers->get<smtk::view::Manager::Ptr>();

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
}

qtWorkletModel::Column qtWorkletModel::columnEnumFromName(const std::string& colName)
{
  std::string name(colName);
  std::transform(
    name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
  // Trim leading namespacing as needed.
  if (name.substr(0, 22) == "qtworkletmodel::")
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
  else if (name == "operation" || name == "workletop")
  {
    return Column::WorkletOp;
  }
  else if (name == "label")
  {
    return Column::Label;
  }
  else if (name == "description")
  {
    return Column::Description;
  }
  smtkWarningMacro(
    smtk::io::Logger::instance(), "Unknown Column Name: " << name << ", returning Count Column");
  return Column::Count;
}

std::string qtWorkletModel::columnNameFromEnum(Column enumerant)
{
  switch (enumerant)
  {
    case Column::Label:
      return "Label";
    case Column::WorkletOp:
      return "WorkletOp";
    case Column::Description:
      return "Description";
    case Column::Count:
      break;
  }
  return "Count";
}

int qtWorkletModel::rowCount(const QModelIndex& parent) const
{
  (void)parent;
  assert(!parent.isValid());
  return static_cast<int>(m_worklets.size());
}

int qtWorkletModel::columnCount(const QModelIndex& parent) const
{
  (void)parent;
  assert(!parent.isValid());
  return static_cast<int>(Column::Count);
}

QVariant qtWorkletModel::data(const QModelIndex& index, int role) const
{
  (void)role;
  QVariant value;

  int row = index.row();
  int col = index.column();
  if (row < 0 || row >= static_cast<int>(m_worklets.size()))
  {
    return value;
  }
  if (col < 0 || col >= static_cast<int>(Column::Count))
  {
    return value;
  }
  if (role != Qt::DisplayRole)
  {
    return value;
  }

  std::string name = m_worklets[row].m_worklet->name();
  switch (static_cast<Column>(col))
  {
    case Column::Label:
      value = QString::fromStdString(m_worklets[row].m_worklet->name());
      break;
    case Column::WorkletOp:
      value = QString::fromStdString(m_worklets[row].m_worklet->operationName());
      break;
    case Column::Description:
      value = QString::fromStdString(m_worklets[row].m_worklet->description());
      break;
    default:
      break;
  }
  return value;
}

QVariant qtWorkletModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant label;
  if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal))
  {
    switch (static_cast<Column>(section))
    {
      case Column::Label:
        label = "label";
        break;
      case Column::WorkletOp:
        label = "operation";
        break;
      case Column::Description:
        label = "description";
        break;
      default:
        label = "error";
        break;
    }
  }
  return label;
}

Qt::ItemFlags qtWorkletModel::flags(const QModelIndex& index) const
{
  return index.isValid()
    ? Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren
    : Qt::NoItemFlags;
}

QStringList qtWorkletModel::mimeTypes() const
{
  QStringList formats;
  formats << "application/x-smtk-worklet-name";
  return formats;
}

QMimeData* qtWorkletModel::mimeData(const QModelIndexList& indexes) const
{
  auto* mimeData = new QMimeData;
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);
  std::set<int> rows;
  for (const auto& index : indexes)
  {
    auto it = rows.find(index.row());
    if (index.isValid() && it == rows.end())
    {
      auto text =
        this->data(index.siblingAtColumn(static_cast<int>(Column::Label)), Qt::DisplayRole)
          .toString();
      stream << text;
      rows.insert(index.row());
    }
  }
  mimeData->setData("application/x-smtk-worklet-name", encodedData);
  return mimeData;
}

QModelIndex qtWorkletModel::findByName(smtk::string::Token label) const
{
  std::size_t sz = m_worklets.size() - 1;
  for (std::size_t ii = 0; ii <= sz; ++ii)
  {
    if (m_worklets[ii].m_worklet->name() == label)
    {
      return this->index(static_cast<int>(ii), 0);
    }
  }
  return QModelIndex();
}

void qtWorkletModel::workletUpdate(
  const smtk::operation::Operation& operation,
  smtk::operation::Operation::Result result)
{
  (void)operation;
  auto created = result->findComponent("created");
  auto modified = result->findComponent("modified");
  auto expunged = result->findComponent("expunged");
  auto expungedResources = result->findResource("resourcesToExpunge");
  int row;
  for (const auto& comp : *created)
  {
    if (auto worklet = std::dynamic_pointer_cast<smtk::task::Worklet>(comp))
    {
      row = static_cast<int>(m_worklets.size());
      this->beginInsertRows(QModelIndex(), row, row);
      m_worklets.emplace_back(worklet);
      this->endInsertRows();
    }
  }
  for (const auto& comp : *modified)
  {
    if (auto worklet = std::dynamic_pointer_cast<smtk::task::Worklet>(comp))
    {
      // The worklet name may have changed, so just iterate until we
      // find a matching component:
      for (row = 0; row < static_cast<int>(m_worklets.size()); ++row)
      {
        if (m_worklets[row].m_worklet == worklet)
        {
          auto idx = this->index(row, 0);
          this->dataChanged(idx, idx);
        }
      }
    }
  }
  for (const auto& comp : *expunged)
  {
    if (auto worklet = std::dynamic_pointer_cast<smtk::task::Worklet>(comp))
    {
      // Just iterate until we find a matching component:
      for (row = 0; row < static_cast<int>(m_worklets.size()); ++row)
      {
        if (m_worklets[row].m_worklet == worklet)
        {
          this->beginRemoveRows(QModelIndex(), row, row);
          m_worklets.erase(m_worklets.begin() + row);
          this->endRemoveRows();
          break;
        }
      }
    }
  }
  for (const auto& res : *expungedResources)
  {
    // Ignore all non-project resources being removed
    if (std::dynamic_pointer_cast<smtk::project::Project>(res) == nullptr)
    {
      continue;
    }
    // Look at all of the worklets and remove those that are owned by the project being removed
    for (row = 0; row < static_cast<int>(m_worklets.size());)
    {
      if (m_worklets[row].m_worklet->resource() == res)
      {
        this->beginRemoveRows(QModelIndex(), row, row);
        m_worklets.erase(m_worklets.begin() + row);
        this->endRemoveRows();
      }
      else
      {
        ++row;
      }
    }
  }
}
