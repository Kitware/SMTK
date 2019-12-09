//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/GarbageCollector.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace resource
{

GarbageCollector::GarbageCollector() = default;

GarbageCollector::~GarbageCollector()
{
  for (const auto& entry : m_observers)
  {
    auto manager = entry.first.lock();
    auto observer = entry.second;
    if (observer.assigned() && manager)
    {
      manager->observers().erase(observer);
    }
  }
}

#if 0
GarbageCollector::Ptr GarbageCollector::create(const smtk::operation::WeakManagerPtr& mgr)
{
  GarbageCollector::Ptr result;
  auto manager = mgr.lock();
  if (!manager)
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Could not create garbage collector without an operation manager.");
    return result;
  }
  result = GarbageCollector::Ptr(new GarbageCollector(manager));
  return result;
}
#endif

bool GarbageCollector::add(const smtk::operation::OperationPtr& deleter)
{
  if (!deleter)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot collect garbage on null operation.");
    return false;
  }
  auto manager = deleter->manager();
  if (!manager)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot collect garbage on unmanaged operation.");
    return false;
  }
  auto oit = m_observers.find(manager);
  if (oit == m_observers.end())
  {
    Key key = manager->observers().insert(
      [this](const smtk::operation::Operation& op, smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        return this->GarbageCollector::collectGarbage(op, event, result);
      },
      /* priority */ 0,
      /* initialize */ false);
    m_observers.insert(std::pair<WeakManagerPtr, Key>(manager, key));
  }

  auto status = GarbageCollector::checkOperation(&*deleter);
  switch (status)
  {
    case Invalid:
      return false;
    default:
    case NotReady:
    case Ready:
      m_garbage.insert(deleter);
      return true;
  }
}

int GarbageCollector::collectGarbage(const smtk::operation::Operation& op,
  smtk::operation::EventType event, smtk::operation::Operation::Result /*unused*/)
{
  (void)op;
  if (event == smtk::operation::EventType::DID_OPERATE)
  {
    std::set<smtk::operation::OperationPtr>::iterator it = m_garbage.begin();
    while (it != m_garbage.end())
    {
      auto curr = it;
      ++it;

      Status status = GarbageCollector::checkOperation(&**curr);
      switch (status)
      {
        case Invalid:
          m_garbage.erase(curr);
          break;
        case Ready:
        {
          auto collector = *curr;
          auto manager = collector->manager();
          m_garbage.erase(curr);
          manager->launchers()(collector);
        }
          return 0;
        case NotReady:
        default:
          break;
      }
    }
  }
  return 0; // Never cancel an operation
}

GarbageCollector::Status GarbageCollector::checkOperation(const smtk::operation::Operation* op)
{
  if (!op)
  {
    return Invalid;
  }

  auto assoc = op->parameters()->associations();
  auto adef = assoc->definition();

  // The minimum count must include (1) the shared pointer we
  // hold during the loop, (2) the resource's shared pointer,
  // and, (3) possibly the operation-association's ReferenceItem
  // (only holdReference is true):
  int minCount = (adef && adef->holdReference() ? 3 : 2);
  bool blank = true;
  for (auto ait = assoc->begin(); ait != assoc->end(); ++ait)
  {
    auto obj = *ait;
    if (obj)
    {
      blank = false;
      int count = obj.use_count();
      if (count > minCount)
      {
        return NotReady;
      }
    }
  }
  auto manager = op->manager();
  if (blank || !manager)
  {
    return Invalid;
  }
  return Ready;
}

} // namespace resource
} // namespace smtk
