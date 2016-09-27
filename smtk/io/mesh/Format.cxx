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

constexpr Format::IOFlags Format::Import;
constexpr Format::IOFlags Format::Export;
constexpr Format::IOFlags Format::Read;
constexpr Format::IOFlags Format::Write;

}
}
}
