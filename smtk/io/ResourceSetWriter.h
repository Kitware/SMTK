//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ResourceSetWriter.h - Writer for SMTK resource files
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_io_ResourceSetWriter_h
#define __smtk_io_ResourceSetWriter_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SystemConfig.h"

#include "smtk/io/Logger.h"

#include "smtk/common/ResourceSet.h"

#include <string>

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT ResourceSetWriter
{
 public:
  bool writeFile(std::string filename, const smtk::common::ResourceSet& resources,
                 smtk::io::Logger& logger, bool writeLinkedFiles = true);
  bool writeString(std::string& content, const smtk::common::ResourceSet& resources,
                   smtk::io::Logger& logger, bool writeLinkedFiles = true);

 protected:
};

  }  // namespace io
}  // namespace smtk

#endif // __smtk_io_ResourceSetWriter_h
