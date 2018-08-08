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

#include "smtk/attribute/Resource.h"
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
  XmlStringWriter(smtk::attribute::ResourcePtr resource)
    : m_resource(resource)
    , m_includeDefinitions(true)
    , m_includeInstances(true)
    , m_includeViews(true)
  {
  }

  virtual ~XmlStringWriter() {}

  // Subclass methods
  virtual std::string className() const = 0;
  virtual std::string rootNodeName() const = 0;
  virtual unsigned int fileVersion() const = 0;

  virtual std::string convertToString(smtk::io::Logger& logger, bool no_declaration = false) = 0;
  // If using the resource's DirectoryInfo - return the ith string (NOTE - make use you have called
  // convertToString first!)
  virtual std::string getString(std::size_t ith, bool no_declaration = false) = 0;

  virtual void generateXml(smtk::io::Logger& logger) = 0;

  //Control which sections of the attribute resource should be writtern out
  // By Default all sections are processed.  These are advance options!!
  // If val is false then defintions will not be saved (default is true)
  void includeDefinitions(bool val) { m_includeDefinitions = val; }

  // If val is false then instances will not be saved (default is true)
  void includeInstances(bool val) { m_includeInstances = val; }

  // If val is false then views will not be saved (default is true)
  void includeViews(bool val) { m_includeViews = val; }

  // If val is true then the resource's DirectoryInfo is used to construct several
  // XML representations
  void useDirectoryInfo(bool val) { m_useDirectoryInfo = val; }

protected:
  smtk::attribute::ResourcePtr m_resource;
  bool m_includeDefinitions;
  bool m_includeInstances;
  bool m_includeViews;
  bool m_useDirectoryInfo;

  smtk::io::Logger m_logger;
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlStringWriter_h
