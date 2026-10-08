//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/job/qtQueueItem.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/job/Manager.h"
#include "smtk/job/Queue.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/common/Managers.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QWidget>

namespace smtk
{
namespace qt
{
namespace
{

class QueueModel : public QAbstractItemModel
{
public:
  QueueModel()
  {
    // Use a null pointer to hold the "active" option (i.e., use
    // whatever queue is active).
    m_data.push_back(nullptr);
  }

  /// An enum used to index columns in the model.
  enum class Column : int
  {
    Name,
    UUID,
    Count
  };

  QModelIndex index(int row, int column, const QModelIndex& parent) const override
  {
    if (
      parent.isValid() || row < 0 || row >= this->rowCount(QModelIndex()) || column < 0 ||
      column >= this->columnCount(QModelIndex()))
    {
      return QModelIndex(); // No cells have children.
    }
    return this->createIndex(row, column);
  }

  QModelIndex parent(const QModelIndex& index) const override
  {
    (void)index;
    // Every item is a direct child of the root node.
    return QModelIndex();
  }

  int rowCount(const QModelIndex& parent) const override
  {
    return parent.isValid() ? 0 : static_cast<int>(m_data.size());
  }

  int columnCount(const QModelIndex& parent) const override
  {
    (void)parent;
    return static_cast<int>(QueueModel::Column::Count);
  }

  QVariant data(const QModelIndex& index, int role) const override
  {
    QVariant result;
    if (index.row() < 0 || static_cast<std::size_t>(index.row()) >= m_data.size())
    {
      return result;
    }

    switch (role)
    {
      case Qt::EditRole: // fall through
      case Qt::DisplayRole:
        switch (index.column())
        {
          case static_cast<int>(QueueModel::Column::Name):
            if (m_data[index.row()])
            {
              result = QString::fromStdString(m_data[index.row()]->name());
            }
            else
            {
              result = "active";
            }
            break;
          case static_cast<int>(QueueModel::Column::UUID):
            if (m_data[index.row()])
            {
              result = QString::fromStdString(m_data[index.row()]->id().toString());
            }
            else
            {
              result = QString::fromStdString(smtk::common::UUID::null().toString());
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    return result;
  }

  void addQueue(const std::shared_ptr<smtk::job::Queue>& queue)
  {
    int sz = static_cast<int>(m_data.size());
    this->beginInsertRows(QModelIndex(), sz, sz);
    m_data.push_back(queue.get());
    this->endInsertRows();
  }

  void removeQueue(const std::shared_ptr<smtk::job::Queue>& queue)
  {
    if (!queue)
    {
      // Do not allow the "active" default to be removed.
      return;
    }
    for (int ii = 0; ii < static_cast<int>(m_data.size()); ++ii)
    {
      if (m_data[ii] == queue.get())
      {
        this->beginRemoveRows(QModelIndex(), ii, ii);
        m_data.erase(m_data.begin() + ii);
        this->endRemoveRows();
        return;
      }
    }
    // TODO: Warn on un-removed queue.
  }

  void updateQueueItem(const std::shared_ptr<smtk::job::Queue>& queue)
  {
    (void)queue;
    for (int ii = 0; ii < static_cast<int>(m_data.size()); ++ii)
    {
      if (m_data[ii] == queue.get())
      {
        Q_EMIT this->dataChanged(
          this->index(ii, 0, QModelIndex()),
          this->index(ii, static_cast<int>(QueueModel::Column::Count), QModelIndex()));
      }
    }
  }

  int find(const std::string& name)
  {
    int ii = 0;
    for (const auto& qq : m_data)
    {
      if ((qq && qq->name() == name) || (!qq && name == "active"))
      {
        return ii;
      }
      ++ii;
    }
    return -1;
  }

  smtk::job::Queue* at(int idx) const
  {
    if (idx < 0 || idx >= static_cast<int>(m_data.size()))
    {
      return nullptr;
    }
    return m_data[idx];
  }

protected:
  std::vector<smtk::job::Queue*> m_data;
};

} // anonymous namespace

class qtQueueItem::Internal
{
public:
  Internal(qtQueueItem* self, const smtk::extension::qtAttributeItemInfo& info)
    : m_self(self)
    , m_model(new QueueModel)
  {
    (void)info;
    if (auto uiManager = m_self->uiManager())
    {
      auto managers = uiManager->managers();
      if (auto om = managers.get<smtk::operation::Manager::Ptr>())
      {
        m_operationManager = om.get();
      }
      if (auto jm = managers.get<smtk::job::Manager::Ptr>())
      {
        m_jobManager = jm.get();
      }
      if (m_jobManager)
      {
        m_queueInstanceObserverKey = m_jobManager->queues().observers().insert(
          [this](
            smtk::common::InstanceEvent event, const std::shared_ptr<smtk::job::Queue>& queue) {
            switch (event)
            {
              case smtk::common::InstanceEvent::Managed:
                m_model->addQueue(queue);
                break;
              case smtk::common::InstanceEvent::Unmanaged:
                m_model->removeQueue(queue);
                break;
              default:
              case smtk::common::InstanceEvent::Modified:
                m_model->updateQueueItem(queue);
                break;
            }
          },
          /*priority*/ 0,
          /*initialize*/ true,
          "qtQueueItem: Track available queues");
      }
    }
  }

  void updateUI()
  {
    auto* iview = m_self->m_itemInfo.baseView();
    auto item = m_self->m_itemInfo.itemAs<smtk::attribute::StringItem>();
    auto itemDef = item->definitionAs<smtk::attribute::StringItemDefinition>();
    QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (m_widget)
    {
      delete m_widget;
      delete m_layout;
    }
    if (iview && !iview->displayItem(item))
    {
      m_widget = nullptr;
      m_layout = nullptr;
      return;
    }
    m_widget = new QWidget;
    m_layout = new QHBoxLayout;
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_label = new QLabel("Queue");
    m_label->setObjectName("label");
    m_label->setSizePolicy(sizeFixedPolicy);
    if (iview)
    {
      m_label->setFixedWidth(iview->fixedLabelWidth());
    }
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_queueChooser = new QComboBox;
    m_queueChooser->setModel(m_model.get());
    m_widget->setLayout(m_layout);
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_queueChooser);
    m_queueChooser->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
    if (m_self->parentWidget() && m_self->parentWidget()->layout())
    {
      m_self->parentWidget()->layout()->addWidget(m_widget);
    }
    // clang-format off
    QObject::connect(
      m_queueChooser, qOverload<int>(&QComboBox::currentIndexChanged),
      m_self, &qtQueueItem::selectQueue);
    // clang-format on
    this->updateItemData();
  }

  void updateItemData()
  {
    if (!m_queueChooser || !m_jobManager)
    {
      return;
    }
    auto dataObj = m_self->itemAs<smtk::attribute::StringItem>();
    m_label->setText(QString::fromStdString(dataObj->label()));
    int idx = m_model->find(dataObj->value());
    m_queueChooser->blockSignals(true);
    m_queueChooser->setCurrentIndex(idx);
    m_queueChooser->blockSignals(false);
  }

  void selectQueue(int queueIndex)
  {
    if (queueIndex < 0)
    {
      return;
    }
    auto dataObj = m_self->itemAs<smtk::attribute::StringItem>();
    auto att = dataObj->attribute();
    auto op = m_operationManager->create("smtk::attribute::EditAttributeItem");
    op->parameters()->associate(att);
    op->parameters()->findString("item path")->setValue("/Queue"); // TODO: Don't hardwire.
    op->parameters()->findString("value")->appendValue(m_queueChooser->currentText().toStdString());
    m_operationManager->launchers()(op);
  }

protected:
  friend class qtQueueItem;

  qtQueueItem* m_self{ nullptr };
  smtk::job::Manager* m_jobManager{ nullptr };
  smtk::operation::Manager* m_operationManager{ nullptr };
  smtk::job::QueueInstances::Observers::Key m_queueInstanceObserverKey;
  std::unique_ptr<QueueModel> m_model;
  QWidget* m_widget{ nullptr };
  QHBoxLayout* m_layout{ nullptr };
  QLabel* m_label{ nullptr };
  QComboBox* m_queueChooser{ nullptr };
};

smtk::extension::qtItem* qtQueueItem::createItemWidget(
  const smtk::extension::qtAttributeItemInfo& info)
{
  // So we support this type of item?
  if (!info.itemAs<smtk::attribute::StringItem>())
  {
    return nullptr;
  }
  return new qtQueueItem(info);
}

qtQueueItem::qtQueueItem(const smtk::extension::qtAttributeItemInfo& info)
  : smtk::extension::qtItem(info)
  , m_p(new Internal(this, info))
{
  this->createWidget();
}

qtQueueItem::~qtQueueItem() = default;

void qtQueueItem::setLabelVisible(bool visible)
{
  if (m_p->m_label)
  {
    m_p->m_label->setVisible(visible);
  }
}

bool qtQueueItem::isFixedWidth() const
{
  return true;
}

void qtQueueItem::updateItemData()
{
  m_p->updateItemData();
}

void qtQueueItem::selectQueue(int queueIndex)
{
  m_p->selectQueue(queueIndex);
}

void qtQueueItem::createWidget()
{
  auto dataObj = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  m_p->updateUI();
}

} // namespace qt
} // namespace smtk
