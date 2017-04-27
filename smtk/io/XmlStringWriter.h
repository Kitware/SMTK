//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlStringWriter.h - Abstract base class for internal attribute writers
// .SECTION Description - Header file only
// .SECTION See Also

#ifndef __smtk_io_XmlStringWriter_h
#define __smtk_io_XmlStringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/System.h"
#include "smtk/io/Logger.h"

#include <string>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlStringWriter
{
public:
  XmlStringWriter(const smtk::attribute::System& system)
    : m_system(system)
    , m_includeDefinitions(true)
    , m_includeInstances(true)
    , m_includeModelInformation(true)
    , m_includeViews(true)
  {
  }

  virtual ~XmlStringWriter() {}

  // Subclass methods
  virtual std::string className() const = 0;
  virtual unsigned int fileVersion() const = 0;

  virtual std::string convertToString(smtk::io::Logger& logger, bool no_declaration = false) = 0;

  virtual void generateXml(
    pugi::xml_node& parent_node, smtk::io::Logger& logger, bool createRoot = true) = 0;

  //Control which sections of the attribute system should be writtern out
  // By Default all sections are processed.  These are advance options!!
  // If val is false then defintions will not be saved
  void includeDefinitions(bool val) { this->m_includeDefinitions = val; }

  // If val is false then instances will not be saved
  void includeInstances(bool val) { this->m_includeInstances = val; }

  // If val is false then model information will not be saved
  void includeModelInformation(bool val) { this->m_includeModelInformation = val; }

  // If val is false then views will not be saved
  void includeViews(bool val) { this->m_includeViews = val; }

protected:
  const smtk::attribute::System& m_system;
  bool m_includeDefinitions;
  bool m_includeInstances;
  bool m_includeModelInformation;
  bool m_includeViews;

  smtk::io::Logger m_logger;
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlStringWriter_h
