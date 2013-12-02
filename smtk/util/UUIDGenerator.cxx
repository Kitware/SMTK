#include "smtk/util/UUIDGenerator.h"

#include <boost/uuid/uuid_generators.hpp>

namespace smtk {
  namespace util {

class UUIDGenerator::Internal
{
public:
  boost::uuids::basic_random_generator<boost::mt19937> m_randomGenerator;
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
  return UUID(this->P->m_randomGenerator());
}

/// Generate a nil UUID.
UUID UUIDGenerator::null()
{
  return UUID(this->P->m_nullGenerator());
}

  } // namespace util
} // namespace smtk
