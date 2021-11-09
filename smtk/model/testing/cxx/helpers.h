//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_testing_helpers_h
#define smtk_model_testing_helpers_h

#include "smtk/common/UUID.h"
#include "smtk/model/Resource.h"

#include <ostream>

namespace smtk
{
namespace model
{
namespace testing
{

smtk::common::UUIDArray createTet(smtk::model::ResourcePtr sm);

/// Report an integer as a hexadecimal value.
class hexconst
{
public:
  hexconst(long long c) { m_val = c; }
  long long m_val;
};

std::ostream& operator<<(std::ostream& os, const hexconst& x);

/// A timer for benchmarking.
class Timer
{
public:
  Timer();
  ~Timer();

  void mark();
  double elapsed();

protected:
  class Internal;
  Internal* P;
};

} // namespace testing
} // namespace model
} // namespace smtk

#endif // smtk_model_testing_helpers_h
