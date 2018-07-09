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

#include "smtk/resource/Set.h"

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
  typedef std::map<std::string, smtk::resource::ResourcePtr> ResourceMapType;

public:
  ResourceSetReader();
  ResourceSetReader(smtk::model::ResourcePtr modelResource);

  bool readFile(std::string filename, smtk::resource::Set& resources, smtk::io::Logger& logger,
    bool loadLinkedFiles = true);
  bool readString(const std::string& content, smtk::resource::Set& resources,
    smtk::io::Logger& logger, bool loadLinkedFiles = true, ResourceMapType* resourceMap = NULL);

protected:
  bool readEmbeddedAttResource(pugi::xml_node& element, smtk::resource::ResourcePtr& resource,
    std::string& linkStartPath, smtk::io::Logger& logger);
  bool readIncludedManager(const pugi::xml_node& element, smtk::resource::ResourcePtr& resource,
    std::string& path, smtk::io::Logger& logger);
  std::string buildIncludePath(const smtk::resource::Set& resources, const std::string link) const;

  smtk::model::ResourcePtr m_modelResource;
};

} // namespace io
} // namespace smtk

#endif // __smtk_io_ResourceSetReader_h
