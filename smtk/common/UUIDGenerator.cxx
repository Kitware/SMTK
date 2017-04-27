//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/UUIDGenerator.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid_generators.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <ctime>    // for time()
#include <stdlib.h> // for getenv()/_dupenv_s()

namespace
{
// Return true when \a vname exists in the environment (empty or not).
static bool checkenv(const char* vname)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
  return getenv(vname) ? true : false;
#else
  char* buf; //allocated or assigned by _dupenv_s
  const bool valid = (_dupenv_s(&buf, NULL, vname) == 0) && (buf != NULL);
  free(buf); //perfectly valid to free a NULL pointer
  return valid;
#endif
}
}

namespace smtk
{
namespace common
{

class UUIDGenerator::Internal
{
public:
  Internal()
  {
    if (checkenv("SMTK_IN_VALGRIND"))
    {
      // This is a poor technique for seeding or
      // we would initialize this way all the time.
      this->m_mtseed.seed(static_cast<boost::mt19937::result_type>(time(NULL)));
      this->m_randomGenerator =
        new boost::uuids::basic_random_generator<boost::mt19937>(&this->m_mtseed);
    }
    else
    {
      this->m_randomGenerator = new boost::uuids::basic_random_generator<boost::mt19937>;
    }
  }
  ~Internal() { delete this->m_randomGenerator; }

  boost::mt19937 m_mtseed;
  boost::uuids::basic_random_generator<boost::mt19937>* m_randomGenerator;
  boost::uuids::nil_generator m_nullGenerator;
};

UUIDGenerator::UUIDGenerator()
{
  this->P = new Internal;
}

UUIDGenerator::~UUIDGenerator()
{
  delete this->P;
}

UUID UUIDGenerator::random()
{
  return UUID((*this->P->m_randomGenerator)());
}

/// Generate a nil UUID.
UUID UUIDGenerator::null()
{
  return UUID(this->P->m_nullGenerator());
}

} // namespace common
} // namespace smtk
