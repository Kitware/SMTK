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
class Logger;
class SMTKCORE_EXPORT AttributeReader
{
public:
  AttributeReader()
    : m_reportAsError(true)
  {
  }

  // Returns true if there was a problem with reading the file
  bool read(smtk::attribute::CollectionPtr collection, const std::string& filename,
    bool includePath, smtk::io::Logger& logger);
  bool read(smtk::attribute::CollectionPtr collection, const std::string& filename,
    smtk::io::Logger& logger)
  {
    return this->read(collection, filename, false, logger);
  }

  bool readContents(smtk::attribute::CollectionPtr collection, const std::string& filecontents,
    smtk::io::Logger& logger);

  bool readContents(smtk::attribute::CollectionPtr collection, const char* contents,
    std::size_t length, smtk::io::Logger& logger);

  bool readContents(
    smtk::attribute::CollectionPtr collection, pugi::xml_node& rootNode, smtk::io::Logger& logger);

  void setSearchPaths(const std::vector<std::string>& paths) { this->m_searchPaths = paths; }

  void setReportDuplicateDefinitionsAsErrors(bool mode) { this->m_reportAsError = mode; }

protected:
private:
  bool m_reportAsError;
  std::vector<std::string> m_searchPaths;
};
}
}

#endif /* __smtk_io_AttributeReader_h */
