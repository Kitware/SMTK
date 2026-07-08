//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/WorkflowObjectMonitor.h"

#include "smtk/task/Manager.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace task
{

class WorkflowObjectMonitor::Internal
{
public:
  Internal(WorkflowObjectMonitor* self)
    : m_self(self)
  {
    if (!self || !self->m_task)
    {
      throw std::logic_error("Cannot monitor nothing.");
    }
    auto* task = self->m_task;

    // Compute the set of tracked objects.
    m_objects = task->manager()->workflowObjects(m_self->m_spec, task);

    // Start observing operations for updates.
    bool monitoring = false;
    if (auto context = task->managers())
    {
      auto taskManager = task->manager();
      if (auto operationManager = context->get<smtk::operation::Manager::Ptr>())
      {
        monitoring = true;
        m_opObserver = operationManager->observers().insert(
          [&](
            const smtk::operation::Operation& op,
            smtk::operation::EventType event,
            smtk::operation::Operation::Result result) -> int {
            this->updateObjects(op, event, result);
            return 0; // Never cancel an operation.
          },
          /* priority */ 0,
          /* initialize */ false,
          "Workflow-objects monitor.");
      }
    }
    if (!monitoring)
    {
      throw std::logic_error("Cannot monitor without managers.");
    }
  }

  ~Internal() = default;

  void recomputeWorkflowObjectsIfPortModified(const smtk::attribute::ComponentItem::Ptr& modified)
  {
    auto taskManager = m_self->m_task->manager();
    bool haveRecomputed = false;
    for (const auto& object : *modified)
    {
      if (auto port = std::dynamic_pointer_cast<smtk::task::Port>(object))
      {
        if (port->parent() == m_self->m_task && !haveRecomputed)
        {
          // A port for this task was modified; recompute the set of workflow objects.
          auto nextObjects = taskManager->workflowObjects(m_self->m_spec, m_self->m_task);
          haveRecomputed = true;

          // Iterate over objects now in the working set and notify of their
          // addition (or removal for components).
          for (auto [rsrc, objects] : nextObjects)
          {
            auto nit = objects.find(nullptr);
            auto pit = m_objects.find(rsrc);
            if (pit != m_objects.end())
            {
              // The resource is represented in both m_objects and nextObjects.
              // A. Is the resource itself in nextObjects but not in m_objects?
              if (nit != objects.end() && pit->second.find(nullptr) != pit->second.end())
              {
                m_self->m_addNotifier(rsrc->shared_from_this());
              }
              // B. Both nextObjects and m_objects have entries for rsrc. See if components
              //    in the resource are added/removed.
              for (const auto& obj : objects)
              {
                if (!obj)
                {
                  // The resource itself. Skip as it is handled above.
                  continue;
                }
                if (pit->second.find(obj) == pit->second.end())
                {
                  // obj is being added
                  m_self->m_addNotifier(obj->shared_from_this());
                }
              }
              for (const auto& obj : pit->second)
              {
                if (!obj)
                {
                  // The resource itself. Skip as it is handled above.
                  continue;
                }
                if (objects.find(obj) == objects.end())
                {
                  // obj is being removed.
                  m_self->m_removeNotifier(obj->shared_from_this());
                }
              }
            }
            else
            {
              // rsrc is in nextObjects but not m_objects.
              // Add all the entries in objects.
              for (const auto& obj : objects)
              {
                if (!obj)
                {
                  // The resource itself is tracked.
                  m_self->m_addNotifier(rsrc->shared_from_this());
                }
                else
                {
                  m_self->m_addNotifier(obj->shared_from_this());
                }
              }
            }
          }
          // Now we must identify whether resources were in the previous
          // working set but are no longer present. Notify of their removal
          // (and if components in them were being tracked, of their removal).
          for (auto [rsrc, objects] : m_objects)
          {
            auto it = nextObjects.find(rsrc);
            if (it == nextObjects.end())
            {
              for (const auto& obj : objects)
              {
                if (!obj)
                {
                  m_self->m_removeNotifier(rsrc->shared_from_this());
                }
                else
                {
                  m_self->m_removeNotifier(obj->shared_from_this());
                }
              }
            }
            else
            {
              // Both nextObjects and m_objects have records for this
              // resource; skip as we have already handled above.
            }
          }
          // Now that we have notified observers, update the working set.
          m_objects = nextObjects;
        }
      }
    }
  }

  void updateObjects(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result)
  {
    (void)op;
    if (event == smtk::operation::EventType::WILL_OPERATE)
    {
      return;
    }
    // First, we need to see if the port is marked modified.
    // That may completely re-write the set of tracked objects.
    auto modified = result->findComponent("modified");
    this->recomputeWorkflowObjectsIfPortModified(modified);
    // Now, m_objects is up to date, see if any tracked objects are modified.
    for (const auto& obj : *modified)
    {
      if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
      {
        auto it = m_objects.find(rsrc.get());
        if (it != m_objects.end() && it->second.find(nullptr) != it->second.end())
        {
          m_self->m_modifiedNotifier(rsrc);
        }
      }
      else if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj))
      {
        auto it = m_objects.find(comp->parentResource());
        if (it != m_objects.end() && it->second.find(comp.get()) != it->second.end())
        {
          m_self->m_modifiedNotifier(comp);
        }
      }
    }
    // Finally, see if any tracked objects are being deleted.
    for (const auto& obj : *result->findComponent("expunged"))
    {
      if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
      {
        auto it = m_objects.find(rsrc.get());
        if (it != m_objects.end() && it->second.find(nullptr) != it->second.end())
        {
          m_self->m_removeNotifier(rsrc);
        }
      }
      else if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj))
      {
        auto it = m_objects.find(comp->parentResource());
        if (it != m_objects.end() && it->second.find(comp.get()) != it->second.end())
        {
          m_self->m_removeNotifier(comp);
        }
      }
    }
  }

  smtk::operation::Observers::Key m_opObserver;
  smtk::task::Manager::ResourceObjectMap m_objects;
  WorkflowObjectMonitor* m_self{ nullptr };
};

WorkflowObjectMonitor::WorkflowObjectMonitor(
  smtk::task::Task* task,
  const nlohmann::json& spec,
  ObjectAddedNotifier addNotifier,
  ObjectRemovedNotifier removeNotifier,
  ObjectModifiedNotifier modifiedNotifier)
  : m_task(task)
  , m_spec(spec)
  , m_addNotifier(addNotifier)
  , m_removeNotifier(removeNotifier)
  , m_modifiedNotifier(modifiedNotifier)
  , m_p(std::make_unique<Internal>(this))
{
}

WorkflowObjectMonitor::WorkflowObjectMonitor(
  smtk::task::Task* task,
  const smtk::view::Configuration::Component& spec,
  ObjectAddedNotifier addNotifier,
  ObjectRemovedNotifier removeNotifier,
  ObjectModifiedNotifier modifiedNotifier)
  : m_task(task)
  , m_spec(task->manager()->workflowViewConfigurationToJSONSpec(spec))
  , m_addNotifier(addNotifier)
  , m_removeNotifier(removeNotifier)
  , m_modifiedNotifier(modifiedNotifier)
  , m_p(std::make_unique<Internal>(this))
{
}

WorkflowObjectMonitor::~WorkflowObjectMonitor() = default;

} // namespace task
} // namespace smtk
