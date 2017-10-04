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

#include "smtk/resource/Set.h"

#include <string>

namespace smtk
{
namespace io
{

class SMTKCORE_EXPORT ResourceSetWriter
{
public:
  enum LinkedFilesOption
  {
    SKIP_LINKED_FILES = 0,
    EXPAND_LINKED_FILES,
    WRITE_LINKED_FILES
  };

  bool writeFile(std::string filename, const smtk::resource::Set& resources,
    smtk::io::Logger& logger, LinkedFilesOption option = WRITE_LINKED_FILES);
  bool writeString(std::string& content, const smtk::resource::Set& resources,
    smtk::io::Logger& logger, LinkedFilesOption option = WRITE_LINKED_FILES);

protected:
};

} // namespace io
} // namespace smtk

#endif // __smtk_io_ResourceSetWriter_h
