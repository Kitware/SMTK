//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ResourceSetReader.h - Reader for SMTK resource files
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_ResourceSetReader_h
#define __smtk_io_ResourceSetReader_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"

#include "smtk/io/Logger.h"

#include "smtk/common/ResourceSet.h"

#include <iostream>
#include <string>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace io
{

class SMTKCORE_EXPORT ResourceSetReader
{
  typedef std::map<std::string, smtk::common::ResourcePtr> ResourceMapType;

public:
  bool readFile(std::string filename, smtk::common::ResourceSet& resources,
    smtk::io::Logger& logger, bool loadLinkedFiles = true);
  bool readString(const std::string& content, smtk::common::ResourceSet& resources,
    smtk::io::Logger& logger, bool loadLinkedFiles = true, ResourceMapType* resourceMap = NULL);

protected:
  bool readEmbeddedAttSystem(pugi::xml_node& element, smtk::common::ResourcePtr& resource,
    std::string& linkStartPath, smtk::io::Logger& logger);
  bool readIncludedManager(const pugi::xml_node& element, smtk::common::ResourcePtr& resource,
    std::string& path, smtk::io::Logger& logger);
  std::string buildIncludePath(
    const smtk::common::ResourceSet& resources, const std::string link) const;
};

} // namespace io
} // namespace smtk

#endif // __smtk_io_ResourceSetReader_h
