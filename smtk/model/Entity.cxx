#include "smtk/model/Entity.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;
using namespace smtk::util;

namespace smtk {
  namespace model {

/**\class smtk::model::Entity - Store links between named entities.
  */

/// The default constructor creates an invalid link.
Entity::Entity()
  : m_entityFlags(INVALID)
{
}

/// Construct a link with the given \a dimension with a type specified by \a entityFlags.
Entity::Entity(unsigned int entityFlags, int dimension)
  : m_entityFlags(entityFlags)
{
  // Override the dimension bits if the dimension is specified
  if (dimension >= 0 && dimension <= 4)
    {
    // Clear the dimension bits:
    this->m_entityFlags &= ~(
      DIMENSION_0 | DIMENSION_1 | DIMENSION_2 | DIMENSION_3 | DIMENSION_4);
    // Now add in the *proper* dimension bit to match m_dimension:
    this->m_entityFlags |= (1 << dimension);
    }
}

/**\brief Return the bit vector describing the type and attributes of the associated entity.
  *
  * \sa smtk::model::EntityTypeBits
  */
unsigned int Entity::entityFlags() const
{
  return this->m_entityFlags;
}

/**\brief Return the dimension of the associated entity.
  *
  * When \a entityFlags() includes the CELL bit, this must be in [0,3] for
  * the cell to be considered valid.
  * For the SHELL bit, this must be in [1,3] for the shell to be valid.
  * For the USE bit, this must be in [0,2] for the use to be valid.
  *
  * For the GROUP bit, the integer returned should be treated as a bit
  * vector. Valid values include [0,15].
  */
int Entity::dimension() const
{
  unsigned int dimBits = this->m_entityFlags & ANY_DIMENSION;
  if ((dimBits != 0) & ((dimBits & (dimBits - 1)) == 0))
    { // dimBits is exactly a power of two:
    switch (dimBits)
      {
    case DIMENSION_0: return 0;
    case DIMENSION_1: return 1;
    case DIMENSION_2: return 2;
    case DIMENSION_3: return 3;
    case DIMENSION_4: return 4;
    default: return -2; // A power of two, but not one we know
      }
    }
  // dimBits is NOT a power of two:
  return -1;
}

unsigned int Entity::dimensionBits() const
{
  return this->m_entityFlags & ANY_DIMENSION;
}

UUIDArray& Entity::relations()
{
  return this->m_relations;
}
const UUIDArray& Entity::relations() const
{
  return this->m_relations;
}

Entity& Entity::appendRelation(const UUID& b)
{
  this->m_relations.push_back(b);
  return *this;
}

Entity& Entity::removeRelation(const UUID& b)
{
  UUIDArray& arr(this->m_relations);
  unsigned int size = arr.size();
  unsigned int curr;
  for (curr = 0; curr < size; ++curr)
    {
    if (arr[curr] == b)
      {
      arr.erase(arr.begin() + curr);
      --curr;
      --size;
      }
    }
  return *this;
}

  } // namespace model
} //namespace smtk
