//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

#include "smtk/graph/operators/Delete.h"
#include "smtk/graph/operators/Delete_xml.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/Paths.h"

#include <sstream>

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace graph
{

Delete::Delete() = default;

void Delete::generateSummary(smtk::operation::Operation::Result& result)
{
  if (!m_suppressOutput)
  {
    smtkInfoMacro(
      this->log(),
      "Deleted " << result->findComponent("expunged")->numberOfValues() << " components");
  }
}

const char* Delete::xmlDescription() const
{
  return Delete_xml;
}

} // namespace graph
} // namespace smtk
