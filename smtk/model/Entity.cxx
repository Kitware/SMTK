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

static const char* cellNamesByDimension[] = {
  "vertex",
  "edge",
  "face",
  "volume",
  "spacetime",
  "mixed-dimension cell"
};

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

bool Entity::setEntityFlags(unsigned int flags)
{
  bool allowed = false;
  if (this->m_entityFlags == INVALID)
    {
    this->m_entityFlags = flags;
    allowed = true;
    }
  else
    {
    // Only allow changes to properties, not the entity type
    // after it has been set to something valid.
    // Otherwise, craziness may ensue.
    if ((flags & ANY_ENTITY) == (this->m_entityFlags & ANY_ENTITY))
      {
      this->m_entityFlags = flags;
      allowed = true;
      }
    }
  return allowed;
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
  UUIDArray::size_type size = arr.size();
  UUIDArray::size_type curr;
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

// If you change this, you may need to change flagSummmary/flagDescription/defaultNameFromCounters
std::string Entity::flagSummaryHelper(unsigned int flags)
{
  std::string result;
  switch (flags & ENTITY_MASK)
    {
  case CELL_ENTITY:
    switch (flags & ANY_DIMENSION)
      {
    case DIMENSION_0:
      result = cellNamesByDimension[0];
      break;
    case DIMENSION_1:
      result = cellNamesByDimension[1];
      break;
    case DIMENSION_2:
      result = cellNamesByDimension[2];
      break;
    case DIMENSION_3:
      result = cellNamesByDimension[3];
      break;
    case DIMENSION_4:
      result = cellNamesByDimension[4];
      break;
    default:
      result = cellNamesByDimension[5];
      }
    break;
  case USE_ENTITY:
    switch (flags & ANY_DIMENSION)
      {
    case DIMENSION_0:
      result = "vertex use";
      break;
    case DIMENSION_1:
      result = "edge use";
      break;
    case DIMENSION_2:
      result = "face use";
      break;
    case DIMENSION_3:
      result = "region use";
      break;
    case DIMENSION_4:
      result = "spacetime region use";
      break;
    default:
      result = "mixed-dimension cell use";
      }
    break;
  case SHELL_ENTITY:
    switch (flags & ANY_DIMENSION)
      {
    case DIMENSION_0 | DIMENSION_1:
      result = "chain";
      break;
    case DIMENSION_1 | DIMENSION_2:
      result = "loop";
      break;
    case DIMENSION_2 | DIMENSION_3:
      result = "shell";
      break;
    case DIMENSION_3 | DIMENSION_4:
      // a spacetime region with no volumetric boundary
      result = "timeloop";
      break;
    case 0:
      result = "dimensionless shell";
      break;
    default:
      result = "unknown-dimensionality shell";
      }
    break;
  case GROUP_ENTITY:
    if (flags & MODEL_BOUNDARY) result += "boundary ";
    if (flags & MODEL_DOMAIN) result += "domain ";
    result += "group";
    break;
  case MODEL_ENTITY:
    result = "model";
    break;
  case INSTANCE_ENTITY:
    result = "instance";
    break;
  default:
    result = "invalid";
    break;
    }
  return result;
}

// If you change this, you may also need to change flagDescription/defaultNameForCount below.
std::string Entity::flagSummary(unsigned int flags)
{
  std::string result = flagSummaryHelper(flags);
  // Add some extra information about groups.
  if ((flags & ENTITY_MASK) == GROUP_ENTITY)
    {
    if (flags & ANY_DIMENSION)
      {
      result += " (";
      bool comma = false;
      for (int i = 0; i <= 4; ++i)
        {
        int dim = 1 << i;
        if (flags & dim)
          {
          if (comma)
            {
            result += ",";
            }
          result += cellNamesByDimension[i];
          comma = true;
          }
        }
      switch (flags & ENTITY_MASK)
        {
      case CELL_ENTITY:
        result += " cells)";
        break;
      case USE_ENTITY:
        result += " uses)";
        break;
      case SHELL_ENTITY:
        result += " shells)";
        break;
      default:
        result += " entities)";
        break;
        }
      }
    if (flags & COVER) result += " cover";
    if (flags & PARTITION) result += " partition";
    }
  return result;
}

std::string Entity::flagDescription(unsigned int flags)
{
  // TODO: Eventually this should return a markdown-formatted
  // description documenting the entity type.
  // Example (for CELL_ENTITY | DIMENSION_0):
  //
  // A **vertex** is a cell of dimension 0 that corresponds to
  // a single point in space in the embedding dimension of its
  // containing model. It has no parametric coordinates.
  // Vertices are used as boundary endpoints of vertex chains
  // which define edges.
  return Entity::flagSummary(flags);
}

// If you change this, you may need to change flagSummmary/flagDescription above
std::string Entity::defaultNameFromCounters(unsigned int flags, IntegerList& counters)
{
  std::ostringstream name;
  name << Entity::flagSummaryHelper(flags) << " ";
  switch (flags & ENTITY_MASK)
    {
  case CELL_ENTITY:
  case USE_ENTITY:
    switch (flags & ANY_DIMENSION)
      {
    case DIMENSION_0:
      name << counters[0]++;
      break;
    case DIMENSION_1:
      name << counters[1]++;
      break;
    case DIMENSION_2:
      name << counters[2]++;
      break;
    case DIMENSION_3:
      name << counters[3]++;
      break;
    case DIMENSION_4:
      name << counters[4]++;
      break;
    default:
      name << counters[5]++;
      break;
      }
    break;
  case SHELL_ENTITY:
    switch (flags & ANY_DIMENSION)
      {
    case DIMENSION_0 | DIMENSION_1:
      name << counters[0]++;
      break;
    case DIMENSION_1 | DIMENSION_2:
      name << counters[1]++;
      break;
    case DIMENSION_2 | DIMENSION_3:
      name << counters[2]++;
      break;
    case DIMENSION_3 | DIMENSION_4:
      name << counters[3]++;
      break;
    case 0:
      name << counters[4]++;
      break;
    default:
      name << counters[5]++;
      }
    break;
  case GROUP_ENTITY:
    if (
      (((flags & MODEL_BOUNDARY) != 0) xor
       ((flags & MODEL_DOMAIN) != 0)) == 0)
      {
      name << counters[0]++;
      }
    else if (flags & MODEL_DOMAIN)
      {
      name << counters[1]++;
      }
    else // if (flags & MODEL_BOUNDARY)
      {
      name << counters[2]++;
      }
    break;
  case MODEL_ENTITY:
    name << counters[0]++;
    break;
  case INSTANCE_ENTITY:
  default:
    name << counters[0]++;
    break;
    }
  return name.str();
}

  } // namespace model
} //namespace smtk
