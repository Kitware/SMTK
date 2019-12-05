//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtAvailableOperations.h"

#include "smtk/operation/MetadataContainer.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/view/AvailableOperations.h"

namespace smtk
{
namespace extension
{

qtAvailableOperations::qtAvailableOperations(QWidget* parent)
  : QWidget(parent)
  , m_operationList(nullptr)
  , m_layout(nullptr)
  , m_operationSource(nullptr)
  , m_operationSourceObserverId()
  , m_useLabels(false)
{
  m_operationList = new QListWidget(this);
  m_layout = new QVBoxLayout(this);
  this->setLayout(m_layout);
  m_layout->addWidget(m_operationList);
}

qtAvailableOperations::~qtAvailableOperations()
{
}

void qtAvailableOperations::setOperationSource(smtk::view::AvailableOperationsPtr avail)
{
  if (avail == m_operationSource)
  {
    return;
  }

  if (m_operationSource)
  {
    m_operationSource->observers().erase(m_operationSourceObserverId);
  }
  m_operationSource = avail;
  if (m_operationSource)
  {
    m_operationSourceObserverId = m_operationSource->observers().insert(
      [this](smtk::view::AvailableOperationsPtr) { this->updateList(); },
      0,    // assign a neutral priority
      true, // immediatelyInvoke
      "qtAvailableOperations: Update list of available operations.");
  }
}

void qtAvailableOperations::updateList()
{
  m_operationList->clear();
  if (!m_operationSource)
  {
    return;
  }

  int precedence = -1;
  const auto& ops = m_operationSource->availableOperations();
  auto mgr = m_operationSource->operationManager();
  const auto& meta = mgr->metadata().get<smtk::operation::IndexTag>();
  for (auto op : ops)
  {
    ++precedence;
    auto data = m_operationSource->operationData(op);
    std::string label;
    std::string icon;
    std::string toolTip;
    if (data)
    {
      label = data->name;
      icon = data->iconName;
    }
    else
    {
      const auto& opMeta = meta.find(op);
      if (opMeta == meta.end())
      {
        continue;
      }
      auto opDef =
        smtk::operation::extractParameterDefinition(opMeta->specification(), opMeta->typeName());
      if (opDef)
      {
        toolTip = opDef->briefDescription();
        if (m_useLabels)
        {
          label = opDef->displayedTypeName();
        }
      }
      if (!(m_useLabels && opDef))
      {
        label = opMeta->typeName();
      }
    }
    auto item = new QListWidgetItem(m_operationList);
    item->setData(Qt::UserRole + 47, // TODO: why 47?
      QVariant::fromValue(op));      // Store the operation's index with the list item.
    item->setText(label.c_str());
    if (!toolTip.empty())
    {
      item->setToolTip(toolTip.c_str());
    }
    m_operationList->addItem(item);
  }
  if (!m_operationSource->workflowFilter())
  {
    m_operationList->sortItems();
  }
}
}
}
