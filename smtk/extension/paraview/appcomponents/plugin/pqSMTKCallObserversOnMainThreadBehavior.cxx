//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKCallObserversOnMainThreadBehavior.h"

#include "pqServer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

static pqSMTKCallObserversOnMainThreadBehavior* g_instance = nullptr;

pqSMTKCallObserversOnMainThreadBehavior::pqSMTKCallObserversOnMainThreadBehavior(QObject* parent)
  : Superclass(parent)
{
  // Whenever a new server is connected, mutate the calling logic of the
  // operation and resource Observers to force observation to occur on the main
  // thread.
  QObject::connect(pqSMTKBehavior::instance(),
    (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) & pqSMTKBehavior::addedManagerOnServer,
    this, &pqSMTKCallObserversOnMainThreadBehavior::forceObserversToBeCalledOnMainThread);
}

pqSMTKCallObserversOnMainThreadBehavior* pqSMTKCallObserversOnMainThreadBehavior::instance(
  QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKCallObserversOnMainThreadBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKCallObserversOnMainThreadBehavior::~pqSMTKCallObserversOnMainThreadBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}

void pqSMTKCallObserversOnMainThreadBehavior::forceObserversToBeCalledOnMainThread(
  pqSMTKWrapper* wrapper, pqServer* server)
{
  (void)server;

  if (!wrapper)
  {
    return;
  }

  // Override the resource Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  wrapper->smtkResourceManager()->observers().overrideWith(
    [this](const smtk::resource::Resource& rsrc, smtk::resource::EventType event) -> int {
      m_activeResources[rsrc.id()] = const_cast<smtk::resource::Resource&>(rsrc).shared_from_this();
      emit resourceEvent(
        QString::fromStdString(rsrc.id().toString()), static_cast<int>(event), QPrivateSignal());
      return 0;
    });

  // Connect to the above signal on the main thread and call the Observer
  // functors.
  std::weak_ptr<smtk::resource::Manager> resourceManager = wrapper->smtkResourceManager();
  QObject::connect(this,
    (void (pqSMTKCallObserversOnMainThreadBehavior::*)(QString, int, QPrivateSignal)) &
      pqSMTKCallObserversOnMainThreadBehavior::resourceEvent,
    this, [this, resourceManager](QString resourceId, int event) {
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

  // Override the operation Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  wrapper->smtkOperationManager()->observers().overrideWith(
    [this](const smtk::operation::Operation& oper, smtk::operation::EventType event,
      smtk::operation::Operation::Result result) -> int {
      auto id = smtk::common::UUID::random();
      m_activeOperationMutex.lock();
      m_activeOperations[id] = const_cast<smtk::operation::Operation&>(oper).shared_from_this();
      m_activeOperationMutex.unlock();
      emit operationEvent(QString::fromStdString(id.toString()), static_cast<int>(event),
        result ? QString::fromStdString(result->name()) : QString(), QPrivateSignal());
      return 0;
    });

  // Connect to the above signal on the main thread and call the Observer
  // functors.
  QObject::connect(this,
    (void (pqSMTKCallObserversOnMainThreadBehavior::*)(QString, int, QString, QPrivateSignal)) &
      pqSMTKCallObserversOnMainThreadBehavior::operationEvent,
    this, [this](QString operationId, int event, QString resultName) {
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

  // Override the selection Observers' call method to emit a private signal
  // instead of calling its Observer functors directly.
  wrapper->smtkSelection()->observers().overrideWith(
    [this](const std::string& str, smtk::view::Selection::Ptr selection) -> void {
      auto id = smtk::common::UUID::random();
      m_activeSelection[id] = selection;
      emit selectionEvent(
        QString::fromStdString(id.toString()), QString::fromStdString(str), QPrivateSignal());
    });

  // Connect to the above signal on the main thread and call the Observer
  // functors.
  QObject::connect(this,
    (void (pqSMTKCallObserversOnMainThreadBehavior::*)(QString, QString, QPrivateSignal)) &
      pqSMTKCallObserversOnMainThreadBehavior::selectionEvent,
    this, [this](QString selectionId, QString qstr) {
      auto id = smtk::common::UUID(selectionId.toStdString());
      auto sel = m_activeSelection[id];
      if (auto selection = sel)
      {
        selection->observers().callObserversDirectly(qstr.toStdString(), selection);
      }
      m_activeSelection.erase(id);
    });
}
