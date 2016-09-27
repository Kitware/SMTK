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

struct Format
{
  typedef std::bitset<4> IOFlags;
  constexpr static IOFlags Import = IOFlags(0x1);
  constexpr static IOFlags Export = IOFlags(0x2);
  constexpr static IOFlags Read   = IOFlags(0x4);
  constexpr static IOFlags Write  = IOFlags(0x8);

  Format() : Name(""), Extensions(), Flags(IOFlags(0x0)) {}
  Format(const std::string& name, IOFlags flags) :
    Name(name), Extensions(), Flags(flags) {}
  Format(const std::string& name, const std::vector<std::string>& extensions,
         IOFlags flags) :
    Name(name), Extensions(extensions), Flags(flags) {}
  Format(const std::string& name, const std::string& extension, IOFlags flags) :
    Name(name), Extensions(1,extension), Flags(flags) {}

  bool CanImport() const { return (this->Flags & Import) == Import; }
  bool CanExport() const { return (this->Flags & Export) == Export; }
  bool CanRead()   const { return (this->Flags & Read)   == Read; }
  bool CanWrite()  const { return (this->Flags & Write)  == Write; }

  std::string Name;
  std::vector<std::string> Extensions;
  IOFlags Flags;
};

}
}
}

#endif
