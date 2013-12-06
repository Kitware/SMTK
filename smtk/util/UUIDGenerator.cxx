#include "smtk/util/UUIDGenerator.h"

#include <boost/uuid/uuid_generators.hpp>

#include <ctime> // for time()
#include <stdlib.h> // for getenv()

namespace smtk {
  namespace util {

class UUIDGenerator::Internal
{
public:
  Internal()
  {
    if (getenv("SMTK_IN_VALGRIND"))
      {
      // This is a poor technique for seeding or
      // we would initialize this way all the time.
        this->m_mtseed.seed(static_cast<boost::uin32_t>(time(NULL)));
      this->m_randomGenerator =
        new boost::uuids::basic_random_generator<boost::mt19937>(&this->m_mtseed);
      }
    else
      {
      this->m_randomGenerator =
        new boost::uuids::basic_random_generator<boost::mt19937>;
      }
  }
  ~Internal()
  {
    delete this->m_randomGenerator;
  }

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

  } // namespace util
} // namespace smtk
