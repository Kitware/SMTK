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
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"
#include <string>

namespace smtk
{
namespace io
{
class Logger;
class SetWriter;
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
  bool write(const smtk::attribute::ResourcePtr resource, const std::string& filename,
    smtk::io::Logger& logger);
  bool writeContents(const smtk::attribute::ResourcePtr resource, std::string& filecontents,
    smtk::io::Logger& logger, bool no_declaration = false);
  //Control which sections of the attribute resource should be writtern out
  // By Default all sections are processed.  These are advance options!!
  // If val is false then defintions will not be saved
  void includeDefinitions(bool val) { m_includeDefinitions = val; }

  // If val is false then instances will not be saved
  void includeInstances(bool val) { m_includeInstances = val; }

  // If val is false then views will not be saved
  void includeViews(bool val) { m_includeViews = val; }

  // If val is true then when write(...) will use include file sections in the produced XML
  // and will write out included files (based on the resource's Directory Info) as well
  // This is ignored when calling writeContents(...).  The default is false
  void useDirectoryInfo(bool val) { m_useDirectoryInfo = val; }
protected:
  // Instantiates internal writer
  // Caller is responsible for deleting the instance
  XmlStringWriter* newXmlStringWriter(const smtk::attribute::ResourcePtr resource) const;

private:
  unsigned int m_fileVersion;
  bool m_includeDefinitions;
  bool m_includeInstances;
  bool m_includeViews;
  bool m_useDirectoryInfo;
};
}
}

#endif /* __smtk_io_AttributeWriter_h */
