//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME AttributeWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_AttributeWriter_h
#define __smtk_io_AttributeWriter_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include <string>

namespace smtk
{
namespace attribute
{
class System;
}

namespace io
{
class Logger;
class ResourceSetWriter;
class XmlStringWriter;
class SMTKCORE_EXPORT AttributeWriter
{
  // Make ResourceSetWriter friend so it can call newXmlStringWriter()
  friend class ResourceSetWriter;

public:
  AttributeWriter();

  // Set an explicit file version - for stable builds
  bool setFileVersion(unsigned int version);
  // Set the latest file version - for development
  void setMaxFileVersion();
  unsigned int fileVersion() const;

  // Returns true if there was a problem with writing the file
  bool write(
    const smtk::attribute::System& system, const std::string& filename, smtk::io::Logger& logger);
  bool writeContents(const smtk::attribute::System& system, std::string& filecontents,
    smtk::io::Logger& logger, bool no_declaration = false);
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
#ifndef SHIBOKEN_SKIP
  // Instantiates internal writer
  // Caller is responsible for deleting the instance
  XmlStringWriter* newXmlStringWriter(const smtk::attribute::System& system) const;
#endif
private:
  unsigned int m_fileVersion;
  bool m_includeDefinitions;
  bool m_includeInstances;
  bool m_includeModelInformation;
  bool m_includeViews;
};
}
}

#endif /* __smtk_io_AttributeWriter_h */
