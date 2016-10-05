//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <smtk/io/mesh/Format.h>

namespace smtk {
  namespace io {
namespace mesh {

const Format::IOFlags Format::Import = IOFlags(0x1);
const Format::IOFlags Format::Export = IOFlags(0x2);
const Format::IOFlags Format::Read = IOFlags(0x4);
const Format::IOFlags Format::Write = IOFlags(0x8);

}
}
}
