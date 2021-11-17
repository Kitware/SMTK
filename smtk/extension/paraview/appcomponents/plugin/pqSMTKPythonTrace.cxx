//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKPythonTrace.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/operators/Signal.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

// PV
#include "vtkSMTrace.h"

#include <iostream>
#include <sstream>

namespace
{
// FileItem and ValueItem don't share a common base class, so template.
template<class itemT>
std::string traceParams(itemT item, bool quoted)
{
  if (!item->isEnabled())
  {
    return "";
  }
  std::ostringstream text;
  std::string quote = quoted ? "'" : "";
  if (item->numberOfValues() == 1)
  {
    text << "op.parameters().find('" << item->name() << "').setValue(" << quote
         << item->valueAsString() << quote << ")\n";
  }
  else if (item->numberOfValues() > 1)
  {
    text << "item = op.parameters().find('" << item->name() << "')\n";
    for (auto i = 0; i < item->numberOfValues(); ++i)
    {
      text << "item.setValue(" << i << ", " << quote << item->valueAsString(i) << quote << ")\n";
    }
  }
  return text.str();
}
} // namespace

void pqSMTKPythonTrace::traceOperation(const smtk::operation::Operation& op)
{
  if (dynamic_cast<const smtk::attribute::Signal*>(&op))
    return;

  if (vtkSMTrace::GetActiveTracer() == nullptr)
  {
    m_showSetup = true;
  }
  else if (m_showSetup)
  {
    m_showSetup = false;
    SM_SCOPED_TRACE(TraceText)
      .arg("import smtk.extension.paraview.appcomponents\n"
           "import smtk.attribute\n"
           "import uuid\n"
           "behavior = smtk.extension.paraview.appcomponents.pqSMTKBehavior.instance()\n"
           "opMgr = behavior.activeWrapperOperationManager()\n"
           "rsrcMgr = behavior.activeWrapperResourceManager()")
      .arg("comment", "Retrieve operation manager");
  }

  // construct a string that contains python code to exectute the operation
  std::ostringstream text;
  text << "op = opMgr.createOperation('" << op.typeName() << "')\n";
  smtk::io::AttributeWriter writer;
  smtk::io::Logger outputLogger;
  outputLogger.setFlushToStdout(true);
  writer.includeDefinitions(false);
  writer.includeInstances(true);
  writer.includeResourceID(false);
  writer.includeResourceAssociations(true);
  writer.includeViews(false);
  writer.includeAttributeAssociations(true);
  auto spec = const_cast<smtk::operation::Operation&>(op).specification();
  writer.setExcludedDefinitions({ smtk::operation::extractResultDefinition(spec, op.typeName()) });
  std::string xmlAttr;
  bool outputErr = writer.writeContents(spec, xmlAttr, outputLogger, false);
  if (outputErr)
  {
    std::cout << "Error writing op parameters\n";
  }
  text << "xmlSpec = \"\"\"\n" << xmlAttr << "\n\"\"\"\n";
  text << "op.restoreTrace(xmlSpec)\n";

  text << "res = op.operate()\n";
  SM_SCOPED_TRACE(TraceText)
    .arg(text.str().c_str())
    .arg("comment", "Setup SMTK operation and parameters");
}
