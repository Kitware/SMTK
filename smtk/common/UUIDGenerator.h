//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_UUIDGenerator_h
#define smtk_common_UUIDGenerator_h

#include "smtk/common/UUID.h"

namespace smtk
{
namespace common
{

class SMTKCORE_EXPORT UUIDGenerator
{
public:
  /// Returns a static instance of a UUID Generator
  static UUIDGenerator& instance();

  UUIDGenerator();
  virtual ~UUIDGenerator();
  UUIDGenerator(const UUIDGenerator&) = delete;
  UUIDGenerator& operator=(const UUIDGenerator&) = delete;

  UUID random();
  UUID null();

protected:
  class Internal;
  Internal* P;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_UUIDGenerator_h
