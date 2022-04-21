//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtSMTKCallObserversOnMainThreadBehavior.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

static qtSMTKCallObserversOnMainThreadBehavior* g_instance = nullptr;

qtSMTKCallObserversOnMainThreadBehavior::qtSMTKCallObserversOnMainThreadBehavior(QObject* parent)
  : Superclass(parent)
{
}

qtSMTKCallObserversOnMainThreadBehavior* qtSMTKCallObserversOnMainThreadBehavior::instance(
  QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new qtSMTKCallObserversOnMainThreadBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

qtSMTKCallObserversOnMainThreadBehavior::~qtSMTKCallObserversOnMainThreadBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}

void qtSMTKCallObserversOnMainThreadBehavior::forceObserversToBeCalledOnMainThread(
  const smtk::common::Managers::Ptr& mgrs)
{

  auto rsrcManager = mgrs->get<smtk::resource::Manager::Ptr>();
  // Override the resource Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  if (rsrcManager)
  {
    rsrcManager->observers().overrideWith(
      [this](const smtk::resource::Resource& rsrc, smtk::resource::EventType event) -> int {
        m_activeResources[rsrc.id()] =
          const_cast<smtk::resource::Resource&>(rsrc).shared_from_this();
        Q_EMIT resourceEvent(
          QString::fromStdString(rsrc.id().toString()), static_cast<int>(event), QPrivateSignal());
        return 0;
      });

    // Connect to the above signal on the main thread and call the Observer
    // functors.
    std::weak_ptr<smtk::resource::Manager> resourceManager =
      mgrs->get<smtk::resource::Manager::Ptr>();
    QObject::connect(
      this,
      (void (qtSMTKCallObserversOnMainThreadBehavior::*)(QString, int, QPrivateSignal)) &
        qtSMTKCallObserversOnMainThreadBehavior::resourceEvent,
      this,
      [this, resourceManager](QString resourceId, int event) {
        auto id = smtk::common::UUID(resourceId.toStdString());
        auto rsrc = m_activeResources[id];
        if (const auto& resource = rsrc)
        {
          if (auto manager = resourceManager.lock())
          {
            manager->observers().callObserversDirectly(
              *resource, static_cast<smtk::resource::EventType>(event));
          }
        }
        m_activeResources.erase(id);
      });
  }

  auto operationManager = mgrs->get<smtk::operation::Manager::Ptr>();
  // Override the operation Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  if (operationManager)
  {
    operationManager->observers().overrideWith(
      [this](
        const smtk::operation::Operation& oper,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        auto id = smtk::common::UUID::random();
        m_activeOperationMutex.lock();
        m_activeOperations[id] = const_cast<smtk::operation::Operation&>(oper).shared_from_this();
        m_activeOperationMutex.unlock();
        Q_EMIT operationEvent(
          QString::fromStdString(id.toString()),
          static_cast<int>(event),
          result ? QString::fromStdString(result->name()) : QString(),
          QPrivateSignal());
        return 0;
      });

    // Connect to the above signal on the main thread and call the Observer
    // functors.
    QObject::connect(
      this,
      (void (qtSMTKCallObserversOnMainThreadBehavior::*)(QString, int, QString, QPrivateSignal)) &
        qtSMTKCallObserversOnMainThreadBehavior::operationEvent,
      this,
      [this](QString operationId, int event, QString resultName) {
        auto id = smtk::common::UUID(operationId.toStdString());
        m_activeOperationMutex.lock();
        auto op = m_activeOperations[id];
        if (const auto& operation = op)
        {
          smtk::attribute::AttributePtr att;
          if (!resultName.isNull())
          {
            att = operation->specification()->findAttribute(resultName.toStdString());
          }
          operation->manager()->observers().callObserversDirectly(
            *operation, static_cast<smtk::operation::EventType>(event), att);
        }
        m_activeOperations.erase(id);
        m_activeOperationMutex.unlock();
      });
  }
  // Override the selection Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  auto selection = mgrs->get<smtk::view::Selection::Ptr>();
  if (selection)
  {
    selection->observers().overrideWith(
      [this](const std::string& str, smtk::view::Selection::Ptr selection) -> void {
        auto id = smtk::common::UUID::random();
        m_activeSelection[id] = selection;
        Q_EMIT selectionEvent(
          QString::fromStdString(id.toString()), QString::fromStdString(str), QPrivateSignal());
      });

    // Connect to the above signal on the main thread and call the Observer
    // functors.
    QObject::connect(
      this,
      (void (qtSMTKCallObserversOnMainThreadBehavior::*)(QString, QString, QPrivateSignal)) &
        qtSMTKCallObserversOnMainThreadBehavior::selectionEvent,
      this,
      [this](QString selectionId, QString qstr) {
        auto id = smtk::common::UUID(selectionId.toStdString());
        auto sel = m_activeSelection[id];
        if (auto selection = sel)
        {
          selection->observers().callObserversDirectly(qstr.toStdString(), selection);
        }
        m_activeSelection.erase(id);
      });
  }
}
