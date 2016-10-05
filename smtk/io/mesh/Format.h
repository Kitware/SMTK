//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_Formats_h
#define __smtk_io_Formats_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.

#include <bitset>
#include <string>
#include <vector>

/**\brief Mesh IO formats
  *
  */

namespace smtk {
  namespace io {
namespace mesh {

struct SMTKCORE_EXPORT Format
{
  typedef std::bitset<4> IOFlags;

  Format() : Name(""), Extensions(), Flags(IOFlags(0x0)) {}
  Format(const std::string& name, IOFlags flags) :
    Name(name), Extensions(), Flags(flags) {}
  Format(const std::string& name, const std::vector<std::string>& extensions,
         IOFlags flags) :
    Name(name), Extensions(extensions), Flags(flags) {}
  Format(const std::string& name, const std::string& extension, IOFlags flags) :
    Name(name), Extensions(1,extension), Flags(flags) {}

#ifndef SHIBOKEN_SKIP
  static const IOFlags Import;
  static const IOFlags Export;
  static const IOFlags Read;
  static const IOFlags Write;

  bool CanImport() const { return (this->Flags & Import) == Import; }
  bool CanExport() const { return (this->Flags & Export) == Export; }
  bool CanRead()   const { return (this->Flags & Read)   == Read; }
  bool CanWrite()  const { return (this->Flags & Write)  == Write; }
#endif

  std::string Name;
  std::vector<std::string> Extensions;
  IOFlags Flags;
};

}
}
}

#endif
