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

#include "smtk/common/Visit.h"

namespace smtk
{
namespace operation
{

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
  hint->findInt("value")->setValue(static_cast<int>(selectionValue));
  hint->findVoid("bitwise")->setIsEnabled(bitwise);
  hint->findVoid("ephemeral")->setIsEnabled(ephemeral);
  return hint;
}

template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addBrowserScrollHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "browser scroll hint");
  return hint;
}

template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addBrowserExpandHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "browser expand hint");
  return hint;
}

template<typename Container>
SMTK_ALWAYS_EXPORT inline smtk::attribute::Attribute::Ptr addRenderFocusHint(
  smtk::operation::Operation::Result result,
  const Container& associations)
{
  auto hint = addHintWithAssociations(result, associations, "render focus hint");
  return hint;
}

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

template<typename Functor>
SMTK_ALWAYS_EXPORT inline smtk::common::Visited visitFocusHintsOfType(
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

} // namespace operation
} // namespace smtk

#endif // smtk_operation_Hints_h
