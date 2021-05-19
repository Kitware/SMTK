//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME AttributeReader.h -
// .SECTION Description Used to Read an Attribute Manager from a string or file
// .SECTION See Also

#ifndef __smtk_io_AttributeReader_h
#define __smtk_io_AttributeReader_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"
#include <string>
#include <vector>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace io
{
class AttributeReaderInternals;
class Logger;

class SMTKCORE_EXPORT AttributeReader
{
public:
  AttributeReader();
  ~AttributeReader();

  // Returns true if there was a problem with reading the file
  bool read(
    smtk::attribute::ResourcePtr resource,
    const std::string& filename,
    bool includePath,
    smtk::io::Logger& logger);
  bool
  read(smtk::attribute::ResourcePtr resource, const std::string& filename, smtk::io::Logger& logger)
  {
    return this->read(resource, filename, true, logger);
  }

  bool readContents(
    smtk::attribute::ResourcePtr resource,
    const std::string& filecontents,
    smtk::io::Logger& logger);

  bool readContents(
    smtk::attribute::ResourcePtr resource,
    const char* contents,
    std::size_t length,
    smtk::io::Logger& logger);

  bool readContents(
    smtk::attribute::ResourcePtr resource,
    pugi::xml_node& rootNode,
    smtk::io::Logger& logger);

  void setSearchPaths(const std::vector<std::string>& paths) { m_searchPaths = paths; }

  void setReportDuplicateDefinitionsAsErrors(bool mode) { m_reportAsError = mode; }

protected:
private:
  bool m_reportAsError{ true };
  std::vector<std::string> m_searchPaths;
  AttributeReaderInternals* m_internals;
};
} // namespace io
} // namespace smtk

#endif /* __smtk_io_AttributeReader_h */
