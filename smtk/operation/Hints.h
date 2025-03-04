//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_Hints_h
#define smtk_operation_Hints_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/view/SelectionAction.h"

#include "smtk/project/Project.h"

#include "smtk/task/Task.h"

#include "smtk/common/Visit.h"

#include <sstream>

namespace smtk
{
namespace operation
{

/// Add a hint of the given \a hintType to the \a result.
///
/// This can be used to create a \a hintType attribute of any type
/// that allows objects in \a associations to be associated to it.
/// You can use this function to create most hints which inherit the
/// "association hint" definition.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addHintWithAssociations(
  smtk::operation::Operation::Result result,
  const Container& associations,
  const std::string& hintType)
{
  if (!result)
  {
    return nullptr;
  }
  auto specification = result->attributeResource();
  auto hint = specification->createAttribute(hintType);
  if (!hint)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not create hint of type \"" << hintType << "\".");
    return nullptr;
  }
  auto assocItem = hint->associations();
  if (!assocItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Hint of type \"" << hintType << "\" does not allow associations.");
    return nullptr;
  }
  bool ok = true;
  for (const auto& association : associations)
  {
    ok &= assocItem->appendValue(association);
    if (!ok)
    {
      break;
    }
  }
  ok &= result->findReference("hints")->appendValue(hint);
  if (!ok)
  {
    specification->removeAttribute(hint);
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Hint of type \"" << hintType << "\" does not allow objects.");
    hint = nullptr;
  }
  return hint;
}

/// Create a hint to indicate the application selection should be modified.
///
/// This adds a "selection hint" attribute associated to \a associations
/// to the \a result and prepares it with the \a selectionAction, \a selectionValue,
/// \a bitwise, and \a ephemeral items to indicate how applications should modify
/// their selection based on the \a result.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addSelectionHint(
  smtk::operation::Operation::Result result,
  const Container& associations,
  smtk::view::SelectionAction selectionAction = smtk::view::SelectionAction::DEFAULT,
  int selectionValue = 1,
  bool bitwise = false,
  bool ephemeral = false)
{
  auto hint = addHintWithAssociations(result, associations, "selection hint");
  if (!hint)
  {
    return hint;
  }
  hint->findInt("action")->setValue(static_cast<int>(selectionAction));
  hint->findInt("value")->setValue(selectionValue);
  hint->findVoid("bitwise")->setIsEnabled(bitwise);
  hint->findVoid("ephemeral")->setIsEnabled(ephemeral);
  return hint;
}

/// Create a hint indicating browsers should scroll to show objects in \a associations.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addBrowserScrollHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "browser scroll hint");
  return hint;
}

/// Create a hint indicating browsers should expand trees to show objects in \a associations.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addBrowserExpandHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "browser expand hint");
  return hint;
}

/// Create a hint indicating render-views should zoom to objects in \a associations.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addRenderFocusHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "render focus hint");
  return hint;
}

/// Visit hints in the \a result that inherit "selection hint" and invoke \a functor on them.
template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitSelectionHints(
  smtk::operation::Operation::Result result,
  Functor functor)
{
  auto hintsItem = result->findReference("hints");
  if (!hintsItem)
  {
    return smtk::common::Visited::Empty;
  }
  std::size_t numberOfHints = hintsItem->numberOfValues();
  smtk::common::VisitorFunctor<Functor> ff(functor);
  bool didVisit = false;
  for (std::size_t ii = 0; ii < numberOfHints; ++ii)
  {
    auto hint = hintsItem->valueAs<smtk::attribute::Attribute>(ii);
    if (hint && hint->type() == "selection hint")
    {
      auto action = static_cast<smtk::view::SelectionAction>(hint->findInt("action")->value());
      int value = hint->findInt("value")->value();
      bool bitwise = hint->findVoid("bitwise")->isEnabled();
      bool ephemeral = hint->findVoid("ephemeral")->isEnabled();
      if (ff(hint->associations(), action, value, bitwise, ephemeral) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
      didVisit = true;
    }
  }
  return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
}

/// Visit hints of \a hintType and invoke \a functor on them.
template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitAssociationHintsOfType(
  smtk::operation::Operation::Result result,
  const std::string& hintType,
  Functor functor)
{
  auto hintsItem = result->findReference("hints");
  if (!hintsItem)
  {
    return smtk::common::Visited::Empty;
  }
  std::size_t numberOfHints = hintsItem->numberOfValues();
  smtk::common::VisitorFunctor<Functor> ff(functor);
  bool didVisit = false;
  for (std::size_t ii = 0; ii < numberOfHints; ++ii)
  {
    auto hint = hintsItem->valueAs<smtk::attribute::Attribute>(ii);
    if (hint && hint->type() == hintType)
    {
      if (ff(hint->associations()) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
      didVisit = true;
    }
  }
  return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
}

/// Visit "focus hint" hints of \a hintType (that inherit "focus hint") and invoke \a functor.
template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitFocusHintsOfType(
  smtk::operation::Operation::Result result,
  const std::string& hintType,
  Functor functor)
{
  return visitAssociationHintsOfType(result, hintType, functor);
}

/// Create a hint of type \a hintType that reference tasks in a project.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addHintWithTasks(
  smtk::operation::Operation::Result result,
  const smtk::project::Project::Ptr& project,
  const Container& taskIds,
  const std::string& hintType)
{
  if (!result)
  {
    return nullptr;
  }
  auto specification = result->attributeResource();
  auto hint = specification->createAttribute(hintType);
  if (!hint)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not create hint of type \"" << hintType << "\".");
    return nullptr;
  }
  hint->associations()->setValue(project);
  auto tasksItem = hint->findString("tasks");
  if (!tasksItem)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Hint of type \"" << hintType << "\" does not allow associations.");
    return nullptr;
  }
  bool ok = true;
  ok &= tasksItem->setValues(taskIds.begin(), taskIds.end());
  ok &= result->findReference("hints")->appendValue(hint);
  if (!ok)
  {
    specification->removeAttribute(hint);
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Hint of type \"" << hintType << "\" does not allow objects.");
    hint = nullptr;
  }
  return hint;
}

/// Add a hint to the \a result indicating the given \a task should become active.
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addActivateTaskHint(
  smtk::operation::Operation::Result result,
  const smtk::project::Project::Ptr& project,
  smtk::task::Task* task)
{
  std::set<std::string> taskIds;
  // Serialize the string token by converting the integer hash into a string;
  // do not assume the string manager can provide string content for the token
  // as we do not want the string to be relied upon.
  std::ostringstream taskId;
  taskId << task->id();
  // Add the string to the set of task IDs.
  taskIds.insert(taskId.str());
  auto hint = addHintWithTasks(result, project, taskIds, "activate task hint");
  return hint;
}

/// Visit "task hint" hints of type \a hintType and invoke \a functor on them.
template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitTaskHintsOfType(
  smtk::operation::Operation::Result result,
  const std::string& hintType,
  Functor functor)
{
  auto hintsItem = result->findReference("hints");
  if (!hintsItem)
  {
    return smtk::common::Visited::Empty;
  }
  std::size_t numberOfHints = hintsItem->numberOfValues();
  smtk::common::VisitorFunctor<Functor> ff(functor);
  bool didVisit = false;
  for (std::size_t ii = 0; ii < numberOfHints; ++ii)
  {
    auto hint = hintsItem->valueAs<smtk::attribute::Attribute>(ii);
    if (hint && hint->type() == hintType)
    {
      // Convert the associated objects into a set of project pointers.
      std::set<smtk::project::Project::Ptr> projects;
      std::size_t na = hint->associations()->numberOfValues();
      for (std::size_t ii = 0; ii < na; ++ii)
      {
        if (auto project = hint->associations()->valueAs<smtk::project::Project>(ii))
        {
          projects.insert(project);
        }
      }
      // Convert the strings into UUIDs (task IDs) and then
      // invoke the functor on the set.
      auto tasksItem = hint->findString("tasks");
      if (!tasksItem)
      {
        continue;
      }

      std::set<smtk::common::UUID> taskIds;
      for (const auto& value : *tasksItem)
      {
        smtk::common::UUID taskId;
        std::istringstream valueStream(value);
        valueStream >> taskId;
        taskIds.insert(smtk::common::UUID(taskId));
      }
      if (ff(projects, taskIds) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
      didVisit = true;
    }
  }
  return didVisit ? smtk::common::Visited::All : smtk::common::Visited::Empty;
}

/// Show or hide the objects in \a associations based on \a show.
///
/// This adds a "render visibility hint" to \a result associated to all
/// the resources/components in \a associations indicating that applications
/// should show (when \a show is true) or hide (otherwise) the associated
/// objects.
template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addRenderVisibilityHint(
  smtk::operation::Operation::Result result,
  const Container& associations,
  bool show = false)
{
  auto hint = addHintWithAssociations(result, associations, "render visibility hint");
  hint->findVoid("show")->setIsEnabled(show);
  return hint;
}

} // namespace operation
} // namespace smtk

#endif // smtk_operation_Hints_h
