//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKPythonTrace.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"
#include "smtk/resource/Resource.h"
// PV
#include "vtkSMTrace.h"

#include <iostream>
#include <sstream>
#include <string>

namespace
{
std::size_t replace_all(std::string& inout, std::string what, std::string with)
{
  std::size_t count{};
  for (std::string::size_type pos{};
       std::string::npos != (pos = inout.find(what.data(), pos, what.length()));
       pos += with.length(), ++count)
  {
    inout.replace(pos, what.length(), with.data(), with.length());
  }
  return count;
}

// handle names with embeded quote or newlines
std::string quoteName(const std::string& name)
{
  std::string raw = name.find('\\') != std::string::npos ? "r" : "";
  if (name.find('\'') != std::string::npos || name.find('\n') != std::string::npos)
  {
    std::string repl = name;
    replace_all(repl, "'", "\\'");
    return raw + "'''" + repl + "'''";
  }
  return raw + "'" + name + "'";
}

std::string traceAssociations(const smtk::attribute::ReferenceItemPtr& assoc)
{
  std::ostringstream text;
  std::string indent = "    ";
  if (assoc && assoc->isEnabled())
  {
    for (auto entry = assoc->begin(); entry != assoc->end(); ++entry)
    {
      // Each ReferenceItem is retrieved with a resource name and optional component name
      auto* resource = dynamic_cast<smtk::resource::Resource*>((*entry).get());
      if (resource)
      {
        text << indent << "{ 'resource': " << quoteName(resource->name())
             << " }, # id: " << resource->id().toString() << "\n";
      }
      else
      {
        auto* comp = dynamic_cast<smtk::resource::Component*>((*entry).get());
        if (comp)
        {
          resource = dynamic_cast<smtk::resource::Resource*>(comp->resource().get());
          text << indent << "{ 'resource': " << quoteName(resource->name())
               << ", 'component': " << quoteName(comp->name()) << " }, # id "
               << comp->id().toString() << "\n";
        }
      }
    }
  }
  return text.str();
}

// adds indices for group items
std::string getPath(const smtk::attribute::Item* item)
{
  std::string path = item->name();
  auto owningItem = item->owningItem();
  const auto* groupItem = dynamic_cast<const smtk::attribute::GroupItem*>(owningItem.get());

  while (owningItem)
  {
    if (groupItem)
    {
      // parent is a group item, add "#N" to indicate group index
      path = std::to_string(item->subGroupPosition()) + "/" + path;
    }
    path = owningItem->name() + "/" + path;
    owningItem = owningItem->owningItem();
  }
  return path;
}

// FileItem and ValueItem don't share a common base class, so template.
template<class itemT>
std::string traceParams(itemT item, bool quoted)
{
  std::ostringstream text;
  std::string quote = quoted ? "'" : "";
  std::string indent = "    ";
  std::string path = getPath(item.get());
  // Handle recording unset() values by using python None
  auto itemVal = [&](int i) {
    if (!item->isSet(i))
      return std::string("None");
    return quoted ? quoteName(item->valueAsString(i)) : item->valueAsString(i);
  };
  // record "enable" key only if item is optional.
  std::string enabled;
  if (item->isOptional())
  {
    enabled = ", 'enable': " + std::string(item->isEnabled() ? "True" : "False");
  }
  if (item->numberOfValues() == 1 && !item->isExtensible())
  {
    text << indent << "{ 'path': " << quoteName(path) << enabled << ", 'value': " << itemVal(0)
         << " },\n";
  }
  else if ((item->isExtensible() && item->numberOfValues() > 0) || item->numberOfValues() > 1)
  {
    // nothing special for extensible items - record a list
    text << indent << "{ 'path': " << quoteName(path) << enabled << ", 'value': [ " << itemVal(0);
    for (auto i = 1; i < item->numberOfValues(); ++i)
    {
      text << ", " << itemVal(i);
    }
    text << " ] },\n";
  }
  return text.str();
}

std::string traceVoid(const smtk::attribute::Item* item)
{
  std::ostringstream text;
  std::string indent = "    ";
  std::string path = getPath(item);
  // if the user changed the default state, record.
  if (item->isEnabled() != item->definition()->isEnabledByDefault())
  {
    text << indent << "{ 'path': " << quoteName(path)
         << ", 'enable': " << (item->isEnabled() ? "True" : "False") << " },\n";
  }
  return text.str();
}

std::string traceGroup(const smtk::attribute::GroupItem* item)
{
  std::ostringstream text;
  std::string indent = "    ";
  std::string path = getPath(item);
  std::string enabled;
  if (item->isOptional() && item->isEnabled() != item->definition()->isEnabledByDefault())
  {
    enabled = ", 'enable': " + std::string(item->isEnabled() ? "True" : "False");
  }
  text << indent << "{ 'path': " << quoteName(path) << enabled
       << ", 'count': " << item->numberOfGroups() << " },\n";
  return text.str();
}

std::string traceRef(const smtk::attribute::ReferenceItemPtr& item)
{
  std::ostringstream text;
  std::string indent = "    ";
  std::string path = getPath(item.get());
  // record "enable" key only if item is optional.
  std::string enabled;
  if (item->isOptional())
  {
    enabled = ", 'enable': " + std::string(item->isEnabled() ? "True" : "False");
  }
  text << indent << "{ 'path': " << quoteName(path) << enabled << ", 'value': [\n";
  text << traceAssociations(item);
  text << indent << "] },\n";
  return text.str();
}

} // namespace

std::string pqSMTKPythonTrace::traceOperation(const smtk::operation::Operation& op)
{
  if (dynamic_cast<const smtk::attribute::Signal*>(&op))
    return "";

  if (vtkSMTrace::GetActiveTracer() == nullptr)
  {
    m_showSetup = true;
  }
  else if (m_showSetup)
  {
    m_showSetup = false;
    // Besides imports, retrieves the managers, and a list of current resources.
    SM_SCOPED_TRACE(TraceText)
      .arg("import smtk.extension.paraview.appcomponents\n"
           "import smtk.attribute\n"
           "import smtk.operation\n"
           "from smtk.operation import configureAttribute\n"
           "behavior = smtk.extension.paraview.appcomponents.pqSMTKBehavior.instance()\n"
           "opMgr = behavior.activeWrapperOperationManager()\n"
           "rsrcMgr = behavior.activeWrapperResourceManager()")
      .arg("comment", "Retrieve managers");
  }

  // construct a string that contains python code to exectute the operation
  // available resources are passed in, so they can be looked up by name.
  std::ostringstream text;
  text << "op = opMgr.createOperation('" << op.typeName() << "')\n";
  text << "configureAttribute(op.parameters(), {\n";
  text << "  'resourceManager': rsrcMgr,\n";
  text << "  'associations': [\n";
  text << traceAssociations(op.parameters()->associations());
  text << "  ],\n";
  text << "  'items': [\n";

  std::vector<smtk::attribute::Item::Ptr> items;
  // Gather all of the items recursively, if they are active.
  auto filter = [](smtk::attribute::Item::Ptr) { return true; };
  op.parameters()->filterItems(items, filter, true);

  // For each item found...
  for (auto& item : items)
  {
    smtk::attribute::ValueItemPtr valueItem =
      std::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
    smtk::attribute::VoidItemPtr voidItem =
      std::dynamic_pointer_cast<smtk::attribute::VoidItem>(item);
    smtk::attribute::ReferenceItemPtr refItem =
      std::dynamic_pointer_cast<smtk::attribute::ReferenceItem>(item);
    smtk::attribute::FileSystemItemPtr fileItem =
      std::dynamic_pointer_cast<smtk::attribute::FileSystemItem>(item);
    smtk::attribute::GroupItemPtr groupItem =
      std::dynamic_pointer_cast<smtk::attribute::GroupItem>(item);
    if (valueItem)
    {
      if (
        !valueItem->isUsingDefault() ||
        (item->isOptional() && item->isEnabled() != item->definition()->isEnabledByDefault()))
      {
        text << traceParams(valueItem, item->type() == smtk::attribute::Item::StringType);
      }
    }
    else if (refItem)
    {
      text << traceRef(refItem);
    }
    else if (fileItem)
    {
      if (
        !fileItem->isUsingDefault() ||
        (item->isOptional() && item->isEnabled() != item->definition()->isEnabledByDefault()))
      {
        text << traceParams(fileItem, true);
      }
    }
    else if (voidItem)
    {
      text << traceVoid(voidItem.get());
    }
    else if (groupItem)
    {
      text << traceGroup(groupItem.get());
    }
    else
    {
      text << "# unhandled operation input: " << item->name() << std::endl;
    }
  }
  text << "  ],\n"
       << "})\n"
       << "res = op.operate()\n"
       // necessary after Import so pipeline source is available to retrieve.
       << "smtk.extension.paraview.appcomponents.pqSMTKBehavior.processEvents()\n";
  SM_SCOPED_TRACE(TraceText)
    .arg(text.str().c_str())
    .arg("comment", "Setup SMTK operation and parameters");

  // return the operation trace to support testing.
  return text.str();
}
