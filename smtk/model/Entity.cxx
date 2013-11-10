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
  : m_entityFlags(INVALID), m_dimension(-1)
{
}

/// Construct a link with the given \a dimension with a type specified by \a entityFlags.
Entity::Entity(int entityFlags, int dimension)
  : m_entityFlags(entityFlags), m_dimension(dimension)
{
  if (this->m_dimension >= 0 && this->m_dimension <= 3)
    {
    // Clear the dimension bits:
    this->m_entityFlags &= ~(
      DIMENSION_0 | DIMENSION_1 | DIMENSION_2 | DIMENSION_3 | DIMENSION_4);
    // Now add in the *proper* dimension bit to match m_dimension:
    this->m_entityFlags |= (1 << this->m_dimension);
    }
}

/**\brief Return the bit vector describing the type and attributes of the associated entity.
  *
  * \sa smtk::model::EntityTypeBits
  */
int Entity::entityFlags() const
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
  return this->m_dimension;
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
  unsigned size = arr.size();
  unsigned curr;
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
