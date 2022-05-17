//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Entity.h"
#include "smtk/model/FilterGrammar.h"
#include "smtk/model/Group.h"
#include "smtk/model/LimitingClause.h"
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/PropertyType.h"
#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/StringUtil.h"
#include "smtk/common/UUIDGenerator.h"

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::regex_search;
using std::sregex_token_iterator;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
using boost::regex_replace;
using boost::regex_search;
using boost::sregex_token_iterator;
#endif

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <set>
#include <vector>

#include <sstream>

using namespace smtk::common;
using namespace smtk::resource;

// The code in the namespace below is all written in the style of PEGTL
// so that the grammar and actions are consistent and thus easier to read.

namespace smtk
{
namespace model
{

static const char* cellNamesByDimensionSingular[] = { "vertex", "edge",      "face",
                                                      "volume", "spacetime", "cell" };

static const char* cellNamesByDimensionPlural[] = { "vertices", "edges",      "faces",
                                                    "volumes",  "spacetimes", "cells" };

static const char* auxGeomNamesByDimensionSingular[] = {
  "auxiliary point",  "auxiliary curve",     "auxiliary surface",
  "auxiliary volume", "auxiliary spacetime", "auxiliary geometry"
};

static const char* auxGeomNamesByDimensionPlural[] = {
  "auxiliary points",  "auxiliary curves",     "auxiliary surfaces",
  "auxiliary volumes", "auxiliary spacetimes", "auxiliary geometries"
};

static const char* entityTypeNames[] = { "cell",     "use",     "shell",    "group",  "model",
                                         "instance", "session", "aux_geom", "concept" };

/**\class smtk::model::Entity
  *
  * Store links between named entities.
  */

/// The default constructor creates an invalid link.
Entity::Entity()
  : m_id(smtk::common::UUIDGenerator::instance().random())
{
}

Entity::~Entity() = default;

/// Create and set up an entity object in a single call. This version sets the Entity's UUID.
EntityPtr Entity::create(const UUID& uid, BitFlags entityFlags, ResourcePtr resource)
{
  EntityPtr result = Entity::create();
  int dim = Entity::dimensionBitsToDimension(entityFlags & EntityTypeBits::ANY_DIMENSION);
  result->setId(uid);
  result->setup(entityFlags, dim, resource);
  return result;
}

/// Create and set up an entity object in a single call. This version does not set the UUID.
EntityPtr Entity::create(BitFlags entityFlags, int dimension, ResourcePtr resource)
{
  EntityPtr result = Entity::create();
  result->setup(entityFlags, dimension, resource);
  return result;
}

/**\brief Populate an object with the given \a dimension with a type specified by \a entityFlags.
  *
  * By default, this does resets the array of relations,
  * since changing the type or dimension of an object would usually
  * invalidate relations; if it does not, you should
  * pass \a resetRelations = false.
  */
EntityPtr Entity::setup(BitFlags entFlags, int dim, Resource::Ptr resource, bool resetRelations)
{
  m_entityFlags = entFlags;
  m_resource = resource;
  // Override the dimension bits if the dimension is specified
  if (dim >= 0 && dim <= 4)
  {
    // Clear the dimension bits:
    m_entityFlags &= ~(DIMENSION_0 | DIMENSION_1 | DIMENSION_2 | DIMENSION_3 | DIMENSION_4);
    // Now add in the *proper* dimension bit to match m_dimension:
    m_entityFlags |= (1 << dim);
  }
  if (resetRelations)
  {
    m_firstInvalid = -1;
    m_relations.clear();
  }
  return shared_from_this();
}

const smtk::resource::ResourcePtr Entity::resource() const
{
  return std::dynamic_pointer_cast<smtk::resource::Resource>(m_resource.lock());
}

ResourcePtr Entity::modelResource() const
{
  return std::dynamic_pointer_cast<smtk::model::Resource>(m_resource.lock());
}

bool Entity::reparent(ResourcePtr newParent)
{
  ResourcePtr oldParent = m_resource.lock();
  if (oldParent == newParent)
  {
    return false;
  }
  m_resource = newParent;
  return true;
}

/**\brief Return the bit vector describing the type and attributes of the associated entity.
  *
  * \sa smtk::model::EntityTypeBits
  */
BitFlags Entity::entityFlags() const
{
  return m_entityFlags;
}

bool Entity::setEntityFlags(BitFlags flags)
{
  bool allowed = false;
  if (m_entityFlags == INVALID)
  {
    m_entityFlags = flags;
    allowed = true;
  }
  else
  {
    // Only allow changes to properties, not the entity type
    // after it has been set to something valid.
    // Otherwise, craziness may ensue.
    if ((flags & ANY_ENTITY) == (m_entityFlags & ANY_ENTITY))
    {
      m_entityFlags = flags;
      allowed = true;
    }
  }
  return allowed;
}

smtk::model::Resource* Entity::rawModelResource() const
{
  if (m_resource.expired())
  {
    m_rawResource = nullptr;
  }
  else if (m_rawResource == nullptr)
  {
    m_rawResource = m_resource.lock().get();
  }
  return m_rawResource;
}

std::string Entity::name() const
{
  auto* mr = this->rawModelResource();
  if (mr)
  {
    return mr->name(m_id);
  }
  return std::string();
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
  BitFlags dimBits = m_entityFlags & ANY_DIMENSION;
  if ((dimBits != 0) & ((dimBits & (dimBits - 1)) == 0))
  { // dimBits is exactly a power of two:
    switch (dimBits)
    {
      case DIMENSION_0:
        return 0;
      case DIMENSION_1:
        return 1;
      case DIMENSION_2:
        return 2;
      case DIMENSION_3:
        return 3;
      case DIMENSION_4:
        return 4;
      default:
        return -2; // A power of two, but not one we know
    }
  }
  // dimBits is NOT a power of two:
  return -1;
}

BitFlags Entity::dimensionBits() const
{
  return m_entityFlags & ANY_DIMENSION;
}

UUIDArray& Entity::relations()
{
  return m_relations;
}
const UUIDArray& Entity::relations() const
{
  return m_relations;
}

/**\brief Append a relation \a b to this entity's list of relations.
  *
  * This returns the index of \a b in the entity's list of relations.
  *
  * If \a useHoles is true, then a slower algorithm is used that
  * replaces the first invalid relation with \a b (or appends \a b if
  * there are no invalid relations). Once the hole is used, a linear-time
  * search is used to find the next hole. If you will be performing
  * many intermixed insertions and invalidations, you may wish to
  * set \a useHoles to false. Also, if you are restoring an Entity
  * from an archive, you should pass false.
  */
int Entity::appendRelation(const UUID& b, bool useHoles)
{
  int idx;
  if (useHoles)
  {
    if ((idx = this->consumeInvalidIndex(b)) >= 0)
      return idx;
  }
  idx = static_cast<int>(m_relations.size());
  m_relations.push_back(b);
  return idx;
}

EntityPtr Entity::pushRelation(const UUID& b)
{
  m_relations.push_back(b);
  return shared_from_this();
}

/**\brief Remove the relation from the entity. Do not use this unless you know what you are doing.
  *
  * Instead of this method, you should probably use invalidateRelation().
  *
  * This removes the first array entry holding \a b.
  * When the relation \a b is not at the end of the entity's array of relations,
  * the index of all following relations is reduced by one (because the array is
  * compacted).
  * Since arrangements store indices into this list of relations, you should only
  * call removeRelation when you also update all of the Entity's arrangements.
  */
EntityPtr Entity::removeRelation(const UUID& b)
{
  UUIDArray& arr(m_relations);
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
  return shared_from_this();
}

/**\brief Completely clear the entire list of related entities.
  *
  * \warning This invalidates all Arrangement information for the entity.
  *          Do not invoke this unless you know there are no Arrangements.
  */
void Entity::resetRelations()
{
  m_relations.clear();
  m_firstInvalid = -1;
}

/**\brief Find the given relation \a r and return its index, inserting it if not present.
  */
int Entity::findOrAppendRelation(const UUID& r)
{
  for (UUIDArray::size_type i = 0; i < m_relations.size(); ++i)
  {
    if (m_relations[i] == r)
    {
      return static_cast<int>(i);
    }
  }
  int idx;
  if (m_firstInvalid >= 0)
  {
    m_relations[m_firstInvalid] = r;
    idx = m_firstInvalid;
    UUIDArray::size_type i = m_firstInvalid;
    for (++i; i < m_relations.size(); ++i)
    {
      if (m_relations[i].isNull())
      {
        m_firstInvalid = static_cast<int>(i);
        break;
      }
    }
    if (i >= m_relations.size())
    {
      m_firstInvalid = -1;
    }
    return idx;
  }

  idx = static_cast<int>(m_relations.size());
  m_relations.push_back(r);
  return idx;
}

/**\brief Find the given relation \a r and invalidate its index.
  *
  * This returns the invalidated index if the relation was found
  * and -1 if \a r was not present.
  */
int Entity::invalidateRelation(const UUID& r)
{
  for (UUIDArray::size_type i = 0; i < m_relations.size(); ++i)
  {
    if (m_relations[i] == r)
    {
      return this->invalidateRelationByIndex(static_cast<int>(i));
    }
  }
  return -1;
}

/**\brief Given a valid index, invalidate the UUID at that index.
  *
  * This returns the index upon success or -1 upon failure.
  */
int Entity::invalidateRelationByIndex(int relIdx)
{
  if (relIdx < 0 || relIdx >= static_cast<int>(m_relations.size()))
    return -1;

  m_relations[relIdx] = smtk::common::UUID::null();
  if (m_firstInvalid < 0 || (m_firstInvalid >= 0 && relIdx < m_firstInvalid))
    m_firstInvalid = relIdx;
  return relIdx;
}

/**\brief Return a list the dimension bits set in \a entityFlags.
  *
  * The \a plural reference is set to true if \a flags has
  * more than 1 dimension bit set. Otherwise, it returns false.
  */
std::string Entity::flagDimensionList(BitFlags flags, bool& plural)
{
  std::ostringstream result;
  bool needSeparator = false;
  plural = false;
  for (int i = 0; i <= 4; ++i)
  {
    if (flags & (1 << i))
    {
      if (needSeparator)
      {
        result << ", ";
        plural = true;
      }
      else
      {
        needSeparator = true;
      }
      result << i;
    }
  }
  return result.str();
}

// If you change this, you may need to change flagSummmary/flagDescription/defaultNameFromCounters
std::string Entity::flagSummaryHelper(BitFlags flags, int form)
{
  std::string result;
  if (flags & GROUP_ENTITY)
    flags &= (~ENTITY_MASK) | GROUP_ENTITY; // omit all other entity types for switch statement;

  switch (flags & ENTITY_MASK)
  {
    case CELL_ENTITY:
      switch (flags & ANY_DIMENSION)
      {
        case DIMENSION_0:
          result = form ? cellNamesByDimensionPlural[0] : cellNamesByDimensionSingular[0];
          break;
        case DIMENSION_1:
          result = form ? cellNamesByDimensionPlural[1] : cellNamesByDimensionSingular[1];
          break;
        case DIMENSION_2:
          result = form ? cellNamesByDimensionPlural[2] : cellNamesByDimensionSingular[2];
          break;
        case DIMENSION_3:
          result = form ? cellNamesByDimensionPlural[3] : cellNamesByDimensionSingular[3];
          break;
        case DIMENSION_4:
          result = form ? cellNamesByDimensionPlural[4] : cellNamesByDimensionSingular[4];
          break;
        default:
          result = form ? cellNamesByDimensionPlural[5] : cellNamesByDimensionSingular[5];
      }
      break;
    case AUX_GEOM_ENTITY:
      switch (flags & ANY_DIMENSION)
      {
        case DIMENSION_0:
          result = form ? auxGeomNamesByDimensionPlural[0] : auxGeomNamesByDimensionSingular[0];
          break;
        case DIMENSION_1:
          result = form ? auxGeomNamesByDimensionPlural[1] : auxGeomNamesByDimensionSingular[1];
          break;
        case DIMENSION_2:
          result = form ? auxGeomNamesByDimensionPlural[2] : auxGeomNamesByDimensionSingular[2];
          break;
        case DIMENSION_3:
          result = form ? auxGeomNamesByDimensionPlural[3] : auxGeomNamesByDimensionSingular[3];
          break;
        case DIMENSION_4:
          result = form ? auxGeomNamesByDimensionPlural[4] : auxGeomNamesByDimensionSingular[4];
          break;
        default:
          result = form ? auxGeomNamesByDimensionPlural[5] : auxGeomNamesByDimensionSingular[5];
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
          result = "volume use";
          break;
        case DIMENSION_4:
          result = "spacetime volume use";
          break;
        default:
          result = "cell use";
      }
      if (form)
      {
        result += "s"; // plural is easy in this case.
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
          // a spacetime volume with no volumetric boundary
          result = "timeloop";
          break;
        case 0:
          result = "dimensionless shell";
          break;
        default:
          result = "unknown-dimensionality shell";
      }
      if (form)
      {
        result += "s"; // plural is easy in this case.
      }
      break;
    case GROUP_ENTITY:
      if (flags & MODEL_BOUNDARY)
        result += "boundary ";
      if (flags & MODEL_DOMAIN)
        result += "domain ";
      result += "group";
      if (form)
      {
        result += "s"; // plural is easy in this case.
      }
      break;
    case MODEL_ENTITY:
      // If dimension bits are set, use them.
      switch (flags & ANY_DIMENSION)
      {
        case DIMENSION_0:
          result = "point ";
          break;
        case DIMENSION_1:
          result = "curve ";
          break;
        case DIMENSION_2:
          result = "surface ";
          break;
        case DIMENSION_3:
          result = "volumetric ";
          break;
        case DIMENSION_4:
          result = "space-time ";
          break;
      }
      result += "model";
      if (form)
      {
        result += "s"; // plural is easy in this case.
      }
      break;
    case INSTANCE_ENTITY:
      result = "instance";
      if (form)
      {
        result += "s"; // plural is easy in this case.
      }
      break;
    case SESSION:
      result = "session";
      if (form)
      {
        result += "s"; // plural is easy in this case.
      }
      break;
    default:
      result = "invalid";
      break;
  }
  return result;
}

// If you change this, you may also need to change flagDescription/defaultNameForCount below.
/**\brief Return a string summarizing the type of the entity given its bit-vector \a flags.
  */
std::string Entity::flagSummary(BitFlags flags, int form)
{
  std::string result = flagSummaryHelper(flags, form);
  // Add some extra information about groups.
  if ((flags & GROUP_ENTITY) == GROUP_ENTITY)
  {
    bool dimSep = false;
    if (flags & (ANY_ENTITY & ~GROUP_ENTITY))
      result += " (";
    if (flags & ANY_DIMENSION)
    {
      for (int i = 0; i <= 4; ++i)
      {
        int dim = 1 << i;
        if (flags & dim)
        {
          if (dimSep)
            result += ",";
          result += ('0' + i);
          dimSep = true;
        }
      }
      if (dimSep)
        result += "-d";
    }

    bool entSep = false;
    for (BitFlags entType = CELL_ENTITY; entType < ENTITY_MASK; entType <<= 1)
    {
      if (entType & (GROUP_ENTITY | NO_SUBGROUPS))
        continue;
      if (entType & flags)
      {
        if (entSep)
          result += ", ";
        else if (dimSep)
          result += " ";
        entSep = true;
        switch (entType)
        {
          case CELL_ENTITY:
            result += "cells";
            break;
          case USE_ENTITY:
            result += "uses";
            break;
          case SHELL_ENTITY:
            result += "shells";
            break;
          case GROUP_ENTITY:
            result += "groups";
            break;
          case MODEL_ENTITY:
            result += "models";
            break;
          case INSTANCE_ENTITY:
            result += "instances";
            break;
          case SESSION:
            result += "sessions";
            break;
          case AUX_GEOM_ENTITY:
            result += "auxiliary geometries";
            break;
          default:
            break;
        }
      }
    }
    if (dimSep && !entSep)
      result += " entities";
    if (dimSep || entSep)
      result += ")";

    if (flags & COVER)
      result += " cover";
    if (flags & PARTITION)
      result += " partition";
  }
  return result;
}

std::string Entity::flagDescription(BitFlags flags, int form)
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
  return Entity::flagSummary(flags, form);
}

/**\brief Return the next pedigree ID for an entity of the given type when given a vector of counters appropriate to the context.
  *
  * This does not increment the counter by default (so that it can be used to assign names),
  * however if you pass \a incr = true, the counter will be incremented.
  */
int Entity::countForType(BitFlags flags, IntegerList& counters, bool incr)
{
  if (counters.empty())
  {
    return -1;
  }
  IntegerList::value_type* ptr = nullptr;
  switch (flags & ENTITY_MASK)
  {
    case CELL_ENTITY:
    case USE_ENTITY:
      switch (flags & ANY_DIMENSION)
      {
        case DIMENSION_0:
          ptr = !counters.empty() ? &counters[0] : nullptr;
          break;
        case DIMENSION_1:
          ptr = counters.size() > 1 ? &counters[1] : nullptr;
          break;
        case DIMENSION_2:
          ptr = counters.size() > 2 ? &counters[2] : nullptr;
          break;
        case DIMENSION_3:
          ptr = counters.size() > 3 ? &counters[3] : nullptr;
          break;
        case DIMENSION_4:
          ptr = counters.size() > 4 ? &counters[4] : nullptr;
          break;
        default:
          ptr = counters.size() > 5 ? &counters[5] : nullptr;
          break;
      }
      break;
    case SHELL_ENTITY:
      switch (flags & ANY_DIMENSION)
      {
        case DIMENSION_0 | DIMENSION_1:
          ptr = !counters.empty() ? &counters[0] : nullptr;
          break;
        case DIMENSION_1 | DIMENSION_2:
          ptr = counters.size() > 1 ? &counters[1] : nullptr;
          break;
        case DIMENSION_2 | DIMENSION_3:
          ptr = counters.size() > 2 ? &counters[2] : nullptr;
          break;
        case DIMENSION_3 | DIMENSION_4:
          ptr = counters.size() > 3 ? &counters[3] : nullptr;
          break;
        case 0:
          ptr = counters.size() > 4 ? &counters[4] : nullptr;
          break;
        default:
          ptr = counters.size() > 5 ? &counters[5] : nullptr;
      }
      break;
    case GROUP_ENTITY:
      if ((((flags & MODEL_BOUNDARY) != 0) ^ ((flags & MODEL_DOMAIN) != 0)) == 0)
      {
        ptr = !counters.empty() ? &counters[0] : nullptr;
      }
      else if (flags & MODEL_DOMAIN)
      {
        ptr = counters.size() > 1 ? &counters[1] : nullptr;
      }
      else // if (flags & MODEL_BOUNDARY)
      {
        ptr = counters.size() > 2 ? &counters[2] : nullptr;
      }
      break;
    case MODEL_ENTITY:
      ptr = !counters.empty() ? &counters[0] : nullptr;
      break;
    case INSTANCE_ENTITY:
    case SESSION:
    case AUX_GEOM_ENTITY:
    default:
      ptr = !counters.empty() ? &counters[0] : nullptr;
      break;
  }
  if (!ptr)
  {
    return -1;
  }
  int count = *ptr;
  if (incr)
  {
    ++(*ptr);
  }
  return count;
}

/**\brief Given a vector of counters, return a numbered name string and advance counters as required.
  *
  * If you change this method, you may also need to change flagSummmary/flagDescription.
  *
  * By default, this increments the counter as the name is assigned but the \a incr argument
  * can be provided to override this behavior.
  */
std::string Entity::defaultNameFromCounters(BitFlags flags, IntegerList& counters, bool incr)
{
  std::ostringstream name;
  name << Entity::flagSummaryHelper(flags, /*singular*/ 0) << " "
       << Entity::countForType(flags, counters, incr);
  return name.str();
}

template<EntityTypeBits B>
bool hasBit(BitFlags val)
{
  return ((val & B) == B);
}

/**\brief Given a bitmask or entity flag, return a string specifier.
  *
  * The second argument, \a textual, indicates whether or not the value
  * will be converted to a human-oriented textual description of the bits
  * set or simply a string containing the decimal value of the bit vector.
  * The default is to generate human-oriented text since it is also
  * robust to version changes.
  *
  * The textual version should never begin with a number so that the
  * inverse of this method (specifierStringToFlag) need not expend effort
  * inferring the format.
  */
std::string Entity::flagToSpecifierString(BitFlags val, bool textual)
{
  std::ostringstream spec;
  if (textual)
  {
    // Check for specially-named bit values (convenience names)
    if (val == smtk::model::INVALID)
      spec << "invalid";

    else if (val == smtk::model::VERTEX)
      spec << "vertex";
    else if (val == smtk::model::EDGE)
      spec << "edge";
    else if (val == smtk::model::FACE)
      spec << "face";
    else if (val == smtk::model::VOLUME)
      spec << "volume";

    else if (val == smtk::model::VERTEX_USE)
      spec << "vertex_use";
    else if (val == smtk::model::EDGE_USE)
      spec << "edge_use";
    else if (val == smtk::model::FACE_USE)
      spec << "face_use";
    else if (val == smtk::model::VOLUME_USE)
      spec << "volume_use";

    else if (val == smtk::model::CHAIN)
      spec << "chain";
    else if (val == smtk::model::LOOP)
      spec << "loop";
    else if (val == smtk::model::SHELL)
      spec << "shell2";

    else if (val == smtk::model::ANY_ENTITY)
      spec << "any";

    // Not a specially-named bit value; generate a name:
    else
    {
      // Put entity type bits first and ensure they are all non-numeric.
      bool haveType = false;
      BitFlags tmp = val & smtk::model::ENTITY_MASK;
      if ((tmp != 0) && !(tmp & (tmp - 1)))
      {
        // If exactly one entity-type bit is set (this is what test on tmp does above), add it:
        if (smtk::model::isCellEntity(val))
        {
          haveType = true;
          spec << "cell";
        }
        if (smtk::model::isUseEntity(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "use";
        }
        if (smtk::model::isShellEntity(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "shell";
        }
        if (smtk::model::isGroup(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "group";
        }
        if (smtk::model::isModel(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "model";
        }
        if (smtk::model::isInstance(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "instance";
        }
        if (smtk::model::isSessionRef(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "session";
        }
        if (smtk::model::isAuxiliaryGeometry(val))
        {
          if (haveType)
            spec << "|";
          else
            haveType = true;
          spec << "aux_geom";
        }
      }

      // It is possible to get here with a non-zero value but no text in spec yet.
      // This happens with group/attribute-association membership masks. Check for
      // combinations of entity type bits.
      if (!haveType && spec.str().empty() && (val | smtk::model::ENTITY_MASK))
      {
        int eti = 0;
        for (BitFlags ebit = CELL_ENTITY; ebit < smtk::model::ENTITY_MASK; ebit <<= 1, ++eti)
        {
          if (ebit & val)
          {
            if (haveType)
            {
              spec << "|";
            }
            spec << entityTypeNames[eti];
            haveType = true;
          }
        }
      }

      if (!haveType)
      {
        spec << "none";
      }

      if (hasBit<smtk::model::MODEL_DOMAIN>(val))
        spec << "|domain";
      if (hasBit<smtk::model::MODEL_BOUNDARY>(val))
        spec << "|bdy";

      if (hasBit<smtk::model::COVER>(val))
        spec << "|cover";
      if (hasBit<smtk::model::PARTITION>(val))
        spec << "|partition";

      if (hasBit<smtk::model::OPEN>(val))
        spec << "|open";
      if (hasBit<smtk::model::CLOSED>(val))
        spec << "|closed";

      if (hasBit<smtk::model::HOMOGENOUS_GROUP>(val))
        spec << "|homg";

      if (hasBit<smtk::model::NO_SUBGROUPS>(val))
        spec << "|flat";

      if (hasBit<smtk::model::ANY_DIMENSION>(val))
        spec << "|anydim";
      else if (!(val & smtk::model::ANY_DIMENSION))
        spec << "|nodim";
      else
      {
        spec << "|";
        // Note that explicit dimension bits are not separated; they are only 1 character.
        if (hasBit<smtk::model::DIMENSION_0>(val))
          spec << "0";
        if (hasBit<smtk::model::DIMENSION_1>(val))
          spec << "1";
        if (hasBit<smtk::model::DIMENSION_2>(val))
          spec << "2";
        if (hasBit<smtk::model::DIMENSION_3>(val))
          spec << "3";
        if (hasBit<smtk::model::DIMENSION_4>(val))
          spec << "4";
      }
    }
  }
  else
  {
    spec << val;
  }
  return spec.str();
}

// WARNING!!! This array must be kept sorted (according to
// std::string's less-than operator) so that bisection works
// for searching by keyword.
//
// WARNING!!!! No strings in this array may include parentheses,
// square brackets, or angle brackets. Those are reserved for
// searches.
static struct
{
  std::string name;
  BitFlags value;
} orderedValues[] = {
  { "*", smtk::model::ANY_ENTITY },
  { "0", smtk::model::DIMENSION_0 },
  { "1", smtk::model::DIMENSION_1 },
  { "2", smtk::model::DIMENSION_2 },
  { "3", smtk::model::DIMENSION_3 },
  { "4", smtk::model::DIMENSION_4 },
  { "any", smtk::model::ANY_ENTITY },
  { "anydim", smtk::model::ANY_DIMENSION },
  { "aux_geom", smtk::model::AUX_GEOM_ENTITY },
  { "b", smtk::model::SESSION },
  { "bdy", smtk::model::MODEL_BOUNDARY },
  { "cell", smtk::model::CELL_ENTITY },
  { "chain", smtk::model::CHAIN },
  { "closed", smtk::model::CLOSED },
  { "cover", smtk::model::COVER },
  { "domain", smtk::model::MODEL_DOMAIN },
  { "e", smtk::model::EDGE }, // Backwards-compatibility
  { "edge", smtk::model::EDGE },
  { "edge_use", smtk::model::EDGE_USE },
  { "ef", smtk::model::EDGE | smtk::model::FACE },                        // Backwards-compatibility
  { "efr", smtk::model::EDGE | smtk::model::FACE | smtk::model::VOLUME }, // Backwards-compatibility
  { "ev",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_1 |
      smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "f", smtk::model::FACE },     // Backwards-compatibility
  { "face", smtk::model::FACE },
  { "face_use", smtk::model::FACE_USE },
  { "fe",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 |
      smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "fev",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_1 |
      smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "flat", smtk::model::NO_SUBGROUPS },
  { "fr", smtk::model::FACE | smtk::model::VOLUME }, // Backwards-compatibility
  { "fv",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 |
      smtk::model::DIMENSION_0 },     // Backwards compatibility
  { "g", smtk::model::GROUP_ENTITY }, // Backwards compatibility
  { "gmrfev",
    smtk::model::GROUP_ENTITY | smtk::model::MODEL_ENTITY | smtk::model::CELL_ENTITY |
      smtk::model::ANY_DIMENSION }, // Backwards compatibility
  { "group", smtk::model::GROUP_ENTITY },
  { "homg", smtk::model::HOMOGENOUS_GROUP },
  { "instance", smtk::model::INSTANCE_ENTITY },
  { "invalid", smtk::model::INVALID },
  { "loop", smtk::model::LOOP },
  { "m", smtk::model::MODEL_ENTITY }, // Backwards compatibility
  { "model", smtk::model::MODEL_ENTITY },
  { "mrfev",
    smtk::model::MODEL_ENTITY | smtk::model::CELL_ENTITY |
      smtk::model::ANY_DIMENSION }, // Backwards compatibility
  { "nodim", 0 },
  { "none", 0 },
  { "open", smtk::model::OPEN },
  { "partition", smtk::model::PARTITION },
  { "r", smtk::model::VOLUME }, // Backwards compatibility
  { "re",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 |
      smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "region", smtk::model::VOLUME },
  { "rev",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_1 |
      smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rf",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 |
      smtk::model::DIMENSION_2 }, // Backwards compatibility
  { "rfe",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 |
      smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "rfev",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 |
      smtk::model::DIMENSION_1 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rfv",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 |
      smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rv",
    smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 |
      smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "session", smtk::model::SESSION },
  { "shell", smtk::model::SHELL_ENTITY },
  { "shell2", smtk::model::SHELL },
  { "use", smtk::model::USE_ENTITY },
  { "v", smtk::model::VERTEX },                                           // Backwards-compatibility
  { "ve", smtk::model::VERTEX | smtk::model::EDGE },                      // Backwards-compatibility
  { "vef", smtk::model::VERTEX | smtk::model::EDGE | smtk::model::FACE }, // Backwards-compatibility
  { "vefr",
    smtk::model::VERTEX | smtk::model::EDGE | smtk::model::FACE |
      smtk::model::VOLUME }, // Backwards-compatibility
  { "vertex", smtk::model::VERTEX },
  { "vertex_use", smtk::model::VERTEX_USE },
  { "volume", smtk::model::VOLUME },
  { "volume_use", smtk::model::VOLUME_USE },
};

static int numOrderedValues = sizeof(orderedValues) / sizeof(orderedValues[0]);

BitFlags keywordToBitFlags(const std::string& keyword)
{
  if (keyword.empty())
    return 0;

  // If a keyword starts with a number, it is a sequence of dimension bits:
  if (keyword.size() > 1 && (keyword[0] >= '0' && keyword[0] <= '4'))
  {
    BitFlags dims = 0;
    for (std::size_t i = 0; i < keyword.size(); ++i)
      dims |= keywordToBitFlags(keyword.substr(i, 1));
    return dims;
  }

  std::string proper = keyword;            // make a mutable copy.
  smtk::common::StringUtil::lower(proper); // downcase it.
  // Must be an actual keyword. Try looking it up.
  int lo = 0;
  int hi = numOrderedValues;
  while (lo < hi)
  {
    int mid = (lo & hi) + ((lo ^ hi) >> 1);
    if (orderedValues[mid].name < proper)
      lo = mid + 1;
    else
      hi = mid;
  }
  if (orderedValues[hi].name == proper)
    return orderedValues[hi].value;

  return 0;
}

BitFlags Entity::specifierStringToFlag(const std::string& spec)
{
  std::string val(spec);
  StringUtil::trim(val);
  BitFlags result = 0;
  if (val.empty())
    return result;

  // If the string starts with a digit, it must be a direct numeric value.
  if (isdigit(val[0]))
  {
    std::istringstream strval(val);
    strval >> result;
    return result;
  }
  // OK, we have textual values which we will bitwise-OR together.
  // First, split the values at "|"
  StringList vals = smtk::common::StringUtil::split(val, "|", /*omit:*/ true, /*trim:*/ true);
  for (StringList::iterator it = vals.begin(); it != vals.end(); ++it)
  {
    // Now find keywords in our carefully-arranged lookup table
    result |= keywordToBitFlags(smtk::common::StringUtil::lower(*it));
  }
  return result;
}

/**\brief Given a dimension number (0, 1, 2, 3, 4), return the proper bitcode.
  *
  * This does bounds checking and will return 0 for out-of-bound dimensions.
  */
BitFlags Entity::dimensionToDimensionBits(int dim)
{
  switch (dim)
  {
    case 0:
      return DIMENSION_0;
    case 1:
      return DIMENSION_1;
    case 2:
      return DIMENSION_2;
    case 3:
      return DIMENSION_3;
    case 4:
      return DIMENSION_4;
    default:
      break;
  }
  return 0;
}

int Entity::dimensionBitsToDimension(BitFlags dimBits)
{
  switch (dimBits)
  {
    case DIMENSION_0:
      return 0;
    case DIMENSION_1:
      return 1;
    case DIMENSION_2:
      return 2;
    case DIMENSION_3:
      return 3;
    case DIMENSION_4:
      return 4;
    default:
      break;
  }
  return -1;
}

namespace
{
/// Given an entity and a mask, determine if the entity is accepted by the mask.
bool IsValueValid(const smtk::resource::Component& comp, smtk::model::BitFlags mask)
{
  const auto* modelEnt = dynamic_cast<const smtk::model::Entity*>(&comp);
  if (modelEnt)
  {
    smtk::model::EntityRef c = modelEnt->referenceAs<smtk::model::EntityRef>();

    if (!mask)
    {
      return false; // Nothing can possibly match.
    }
    if (mask == smtk::model::ANY_ENTITY)
    {
      return true; // Fast-track the trivial case.
    }

    smtk::model::BitFlags itemType = modelEnt->entityFlags();
    // The m_membershipMask must match the entity type, the dimension, and (if the
    // item is a group) group constraint flags separately;
    // In other words, we require the entity type, the dimension, and the
    // group constraints to be acceptable independently.
    if (
      ((mask & smtk::model::ENTITY_MASK) && !(itemType & mask & smtk::model::ENTITY_MASK) &&
       (itemType & smtk::model::ENTITY_MASK) != smtk::model::GROUP_ENTITY) ||
      ((mask & smtk::model::ANY_DIMENSION) && !(itemType & mask & smtk::model::ANY_DIMENSION)) ||
      ((itemType & smtk::model::GROUP_ENTITY) && (mask & smtk::model::GROUP_CONSTRAINT_MASK) &&
       !(itemType & mask & smtk::model::GROUP_CONSTRAINT_MASK)))
      return false;
    if (
      itemType != mask && itemType & smtk::model::GROUP_ENTITY &&
      // if the mask is only defined as "group", don't have to check further for members
      mask != smtk::model::GROUP_ENTITY)
    {
      // If the mask does not explicitly include groups and if the component is an empty
      // group, reject the component.
      if (!(mask & smtk::model::GROUP_ENTITY))
      {
        EntityRef preExistingMember = c.as<model::Group>().findFirstNonGroupMember();
        if (!preExistingMember.isValid())
        {
          return false;
        }
      }

      // If the the membershipMask is the same as itemType, we don't need to check, else
      // if the item is a group: recursively check that its members
      // all match the criteria. Also, if the HOMOGENOUS_GROUP bit is set,
      // require all entries to have the same entity type flag as the first.
      smtk::model::BitFlags typeMask = mask;
      bool mustBeHomogenous = (typeMask & smtk::model::HOMOGENOUS_GROUP) != 0;
      if (!(typeMask & smtk::model::NO_SUBGROUPS) && !(typeMask & smtk::model::GROUP_ENTITY))
      {
        typeMask |= smtk::model::GROUP_ENTITY; // if groups aren't banned, allow them.
      }
      if (!c.as<model::Group>().meetsMembershipConstraints(c, typeMask, mustBeHomogenous))
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool CheckPropStringValues(const StringList& propValues, const LimitingClause& clause)
{
  if (clause.m_propStringValues.empty())
  {
    return true;
  }
  if (propValues.size() != clause.m_propStringValues.size())
  {
    return false;
  }
  auto sit = clause.m_propStringValues.begin();
  auto rit = clause.m_propStringIsRegex.begin();
  for (auto vit = propValues.begin();

       sit != clause.m_propStringValues.end();

       ++sit, ++rit, ++vit)
  {
    if (*rit)
    {
      // This is a regex, test it.
      regex match(*sit);
      if (!std::regex_search(*vit, match))
      {
        return false;
      }
    }
    else
    {
      if (*sit != *vit)
      {
        return false;
      }
    }
  }
  return true;
}
} // namespace

Entity::QueryFunctor limitedQueryFunctor(
  smtk::model::BitFlags bitFlags,
  LimitingClause& limitClause)
{
  LimitingClause clause(limitClause);
  return [bitFlags, clause](const smtk::resource::Component& comp) -> bool {
    // See if the component matches the bitFlags:
    if (!IsValueValid(comp, bitFlags))
    {
      return false;
    }
    auto modelRsrc = std::dynamic_pointer_cast<smtk::model::Resource>(comp.resource());
    if (!modelRsrc)
    {
      return false;
    }
    // The component matches; see if the requested properties match.
    switch (clause.m_propType)
    {
      case smtk::resource::PropertyType::FLOAT_PROPERTY:
      {
        const smtk::resource::ConstPropertiesOfType<std::vector<double>> floatProperties =
          comp.properties().get<std::vector<double>>();
        if (floatProperties.empty())
        {
          return false;
        }
        if (clause.m_propName.empty())
        {
          return true;
        }
        if (!floatProperties.contains(clause.m_propName))
        {
          return false;
        }
        if (clause.m_propFloatValues.empty())
        {
          return true;
        }
        if (clause.m_propFloatValues != floatProperties.at(clause.m_propName))
        {
          return false;
        }
      }
      break;

      case smtk::resource::PropertyType::STRING_PROPERTY:
      {
        const smtk::resource::ConstPropertiesOfType<std::vector<std::string>> stringProperties =
          comp.properties().get<std::vector<std::string>>();
        if (stringProperties.empty())
        {
          return false;
        }
        if (clause.m_propName.empty())
        {
          return true;
        }
        StringData::const_iterator pit;
        if (!clause.m_propNameIsRegex)
        {
          if (!stringProperties.contains(clause.m_propName))
          {
            return false;
          }
          return CheckPropStringValues(stringProperties.at(clause.m_propName), clause);
        }
        else
        {
          regex re(clause.m_propName);
          std::set<std::string> keys = stringProperties.keys();
          return std::any_of(
            keys.begin(), keys.end(), [&re, &stringProperties, &clause](const std::string& key) {
              // A matching property name with matching values
              return regex_search(key, re) &&
                CheckPropStringValues(stringProperties.at(key), clause);
            });
        }
      }
      break;

      case smtk::resource::PropertyType::INTEGER_PROPERTY:
      {
        const smtk::resource::ConstPropertiesOfType<std::vector<long>> intProperties =
          comp.properties().get<std::vector<long>>();
        if (intProperties.empty())
        {
          return false;
        }
        if (clause.m_propName.empty())
        {
          return true;
        }
        if (!intProperties.contains(clause.m_propName))
        {
          return false;
        }
        if (clause.m_propIntValues.empty())
        {
          return true;
        }
        if (clause.m_propIntValues != intProperties.at(clause.m_propName))
        {
          return false;
        }
      }
      break;

      case smtk::resource::PropertyType::INVALID_PROPERTY: // fall through
      default:
        break;
    }
    return true;
  };
}

Entity::QueryFunctor Entity::filterStringToQueryFunctor(const std::string& filter)
{
  // If we can turn the filter string into a simple bit-vector comparison,
  // do that since it will be much faster to evaluate:
  std::size_t pos;
  if ((pos = filter.find('[')) == std::string::npos)
  {
    smtk::model::BitFlags bitflags =
      filter.empty() ? smtk::model::ANY_ENTITY : smtk::model::Entity::specifierStringToFlag(filter);
    return std::bind(IsValueValid, std::placeholders::_1, bitflags);
  }
  // Now we know the filter string has a limiting clause of some sort,
  // evaluate it.
  std::string entityStr = filter.substr(0, pos);
  std::string limitStr = filter.substr(pos);
  smtk::model::BitFlags bitflags = smtk::model::Entity::specifierStringToFlag(entityStr);

  LimitingClause limitClause;
  tao::pegtl::string_input<> in(limitStr, "filterStringToQueryFunctor");
  try
  {
    tao::pegtl::parse<FilterGrammar, FilterAction>(in, limitClause);
  }
  catch (tao::pegtl::parse_error& err)
  {
    const auto p = err.positions.front();
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Entity::filterStringToQueryFunctor: " << err.what() << "\n"
                                             << in.line_as_string(p) << "\n"
                                             << std::string(p.byte_in_line, ' ') << "^\n");
    return std::bind(IsValueValid, std::placeholders::_1, bitflags);
  }
  if (limitClause.m_propType == smtk::resource::PropertyType::INVALID_PROPERTY)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Entity::filterStringToQueryFunctor: could not parse limit clause. Skipping.");
    return std::bind(IsValueValid, std::placeholders::_1, bitflags);
  }

  return limitedQueryFunctor(bitflags, limitClause);
}

int Entity::arrange(ArrangementKind kind, const Arrangement& arr, int index)
{
  KindsToArrangements::iterator kit = m_arrangements.find(kind);
  if (kit == m_arrangements.end())
  {
    if (index >= 0)
    { // failure: can't replace information that doesn't exist.
      return -1;
    }
    Arrangements blank;
    kit = m_arrangements.insert(std::pair<ArrangementKind, Arrangements>(kind, blank)).first;
  }

  if (index >= 0)
  {
    if (index >= static_cast<int>(kit->second.size()))
    { // failure: can't replace information that doesn't exist.
      return -1;
    }
    kit->second[index] = arr;
  }
  else
  {
    index = static_cast<int>(kit->second.size());
    kit->second.push_back(arr);
  }
  return index;
}

int Entity::unarrange(ArrangementKind kind, int index, bool removeIfLast)
{
  int result = 0;
  if (index < 0 || m_arrangements.empty())
  {
    return result;
  }
  ArrangementKindWithArrangements ak = m_arrangements.find(kind);
  if (ak == m_arrangements.end() || index >= static_cast<int>(ak->second.size()))
  {
    return result;
  }

  ArrangementReferences duals;
  bool hasDuals = this->modelResource()->findDualArrangements(this->id(), kind, index, duals);

  // TODO: notify someone of imminent removal? Rewrite relations?
  std::vector<int> relIdxs;

  if ((ak->second.begin() + index)->relationIndices(relIdxs, shared_from_this(), kind))
  {
    for (std::vector<int>::iterator rit = relIdxs.begin(); rit != relIdxs.end(); ++rit)
    {
      this->invalidateRelationByIndex(*rit);
    }
  }
  ak->second.erase(ak->second.begin() + index);
  ++result;

  // Now, if we removed the last arrangement of this kind, kill the kind-dictionary entry
  if (ak->second.empty())
  {
    m_arrangements.erase(ak);
  }

  // Now find and remove the dual arrangement (if one exists)
  // This branch should not be taken if we are inside the inner unarrangeEntity call below.
  if (hasDuals)
  {
    // Unarrange every dual to this arrangement.
    bool canIncrement = false;
    for (ArrangementReferences::iterator dit = duals.begin(); dit != duals.end(); ++dit)
    {
      if (
        this->modelResource()->unarrangeEntity(
          dit->entityId, dit->kind, dit->index, removeIfLast) == 2)
      {
        // Only increment result when dualEntity is removed, not the dual arrangement.
        canIncrement = true;
      }
    }
    // Only increment once if other entities were removed; we do not indicate how many were removed.
    if (canIncrement)
    {
      ++result;
    }
  }

  return result;
}

bool Entity::clearArrangements()
{
  bool didRemove = !m_arrangements.empty();
  if (didRemove)
    m_arrangements.clear();
  return didRemove;
}

const Arrangements* Entity::hasArrangementsOfKind(ArrangementKind kind) const
{
  auto ait = m_arrangements.find(kind);
  if (ait != m_arrangements.end())
  {
    return &ait->second;
  }
  return nullptr;
}

Arrangements* Entity::hasArrangementsOfKind(ArrangementKind kind)
{
  ArrangementKindWithArrangements ait = m_arrangements.find(kind);
  if (ait != m_arrangements.end())
  {
    return &ait->second;
  }
  return nullptr;
}

Arrangements& Entity::arrangementsOfKind(ArrangementKind kind)
{
  return m_arrangements[kind];
}

const Arrangement* Entity::findArrangement(ArrangementKind kind, int index) const
{
  if (index < 0)
  {
    return nullptr;
  }

  auto kit = m_arrangements.find(kind);
  if (kit == m_arrangements.end())
  {
    return nullptr;
  }

  if (index >= static_cast<int>(kit->second.size()))
  { // failure: can't replace information that doesn't exist.
    return nullptr;
  }

  return &kit->second[index];
}

Arrangement* Entity::findArrangement(ArrangementKind kind, int index)
{
  if (index < 0)
  {
    return nullptr;
  }

  KindsToArrangements::iterator kit = m_arrangements.find(kind);
  if (kit == m_arrangements.end())
  {
    return nullptr;
  }

  if (index >= static_cast<int>(kit->second.size()))
  { // failure: can't replace information that doesn't exist.
    return nullptr;
  }

  return &kit->second[index];
}

int Entity::findArrangementInvolvingEntity(ArrangementKind kind, const smtk::common::UUID& involved)
  const
{
  const Arrangements* arr = this->hasArrangementsOfKind(kind);
  if (!arr)
    return -1;

  Arrangements::const_iterator it;
  int idx = 0;
  UUIDArray rels;
  for (it = arr->begin(); it != arr->end(); ++it, ++idx, rels.clear())
  {
    if (it->relations(rels, const_cast<Entity*>(this)->shared_from_this(), kind))
    {
      for (UUIDArray::iterator rit = rels.begin(); rit != rels.end(); ++rit)
      {
        if (*rit == involved)
        {
          return idx;
        }
      }
    }
  }

  return -1;
}

bool Entity::findDualArrangements(ArrangementKind kind, int index, ArrangementReferences& duals)
  const
{
  const Arrangements* arr = this->hasArrangementsOfKind(kind);
  if (!arr || index < 0 || index >= static_cast<int>(arr->size()))
  {
    return false;
  }

  int relationIdx;
  int sense;
  Orientation orient;

  UUID dualEntityId;
  ArrangementKind dualKind;
  int dualIndex;
  int relStart, relEnd;

  switch (kind)
  {
    case HAS_USE:
      switch (this->entityFlags() & ENTITY_MASK)
      {
        case CELL_ENTITY:
          if ((*arr)[index].IndexSenseAndOrientationFromCellHasUse(relationIdx, sense, orient))
          { // OK, find use's reference to this cell.
            if (relationIdx < 0 || static_cast<int>(this->relations().size()) <= relationIdx)
              return false;
            dualEntityId = m_relations[relationIdx];
            dualKind = HAS_CELL;
            if (
              (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
                 dualEntityId, dualKind, this->id())) >= 0)
            {
              duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
              return true;
            }
          }
          break;
        case SHELL_ENTITY:
          if ((*arr)[index].IndexRangeFromShellHasUse(relStart, relEnd))
          { // Find the use's reference to this shell.
            dualKind = HAS_SHELL;
            for (; relStart != relEnd; ++relStart)
            {
              dualEntityId = m_relations[relStart];
              if (
                (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
                   dualEntityId, dualKind, this->id())) >= 0)
                duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
            }
            if (!duals.empty())
              return true;
          }
          break;
        /*
      bool IndexFromCellEmbeddedInEntity(int& relationIdx) const;
      bool IndexFromCellIncludesEntity(int& relationIdx) const;
      bool IndexFromCellHasShell(int& relationIdx) const;
      bool IndexAndSenseFromUseHasCell(int& relationIdx, int& sense) const;
      bool IndexFromUseHasShell(int& relationIdx) const;
      bool IndexFromUseOrShellIncludesShell(int& relationIdx) const;
      bool IndexFromShellHasCell(int& relationIdx) const;
      bool IndexRangeFromShellHasUse(int& relationBegin, int& relationEnd) const;
      bool IndexFromShellEmbeddedInUseOrShell(int& relationIdx) const;
      bool IndexFromInstanceInstanceOf(int& relationIdx) const;
      bool IndexFromEntityInstancedBy(int& relationIdx) const;
      */
        case USE_ENTITY:
        case GROUP_ENTITY:
        case MODEL_ENTITY:
        case INSTANCE_ENTITY:
          break;
      }
      break;
    case INCLUDES: // INCLUDES/EMBEDDED_IN are always simple and duals; entity type doesn't matter.
    case EMBEDDED_IN:
      if ((*arr)[index].IndexFromSimple(relationIdx))
      { // OK, find use's reference to this cell.
        if (relationIdx < 0 || static_cast<int>(m_relations.size()) <= relationIdx)
          return false;
        dualEntityId = m_relations[relationIdx];
        dualKind = kind == INCLUDES ? EMBEDDED_IN : INCLUDES;
        if (
          (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
             dualEntityId, dualKind, this->id())) >= 0)
        {
          duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
          return true;
        }
      }
      break;
    case SUPERSET_OF:
    case SUBSET_OF:
      if ((*arr)[index].IndexFromSimple(relationIdx))
      { // OK, find the related entity's reference to this one.
        if (relationIdx < 0 || static_cast<int>(m_relations.size()) <= relationIdx)
        {
          return false;
        }
        dualEntityId = m_relations[relationIdx];
        dualKind = (kind == SUPERSET_OF ? SUBSET_OF : SUPERSET_OF);
        if (
          (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
             dualEntityId, dualKind, this->id())) >= 0)
        {
          duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
          return true;
        }
      }
      break;
    case INSTANCE_OF:
    case INSTANCED_BY:
      if ((*arr)[index].IndexFromSimple(relationIdx))
      { // OK, find the related entity's reference to this one.
        if (relationIdx < 0 || static_cast<int>(m_relations.size()) <= relationIdx)
        {
          return false;
        }
        dualEntityId = m_relations[relationIdx];
        dualKind = (kind == INSTANCED_BY ? INSTANCE_OF : INSTANCED_BY);
        if (
          (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
             dualEntityId, dualKind, this->id())) >= 0)
        {
          duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
          return true;
        }
      }
      break;
    case HAS_CELL:
    case HAS_SHELL:
      // These (HAS_CELL, HAS_SHELL) are not duals of each other.
      // Instead, they are the easier half of HAS_USE relations,
      // and is only defined when the source is a use-record.
      if ((this->entityFlags() & ENTITY_MASK) == USE_ENTITY)
      {
        if ((*arr)[index].IndexFromSimple(relationIdx))
        { // OK, find the related entity's reference to this one.
          if (relationIdx < 0 || static_cast<int>(m_relations.size()) <= relationIdx)
          {
            return false;
          }
          dualEntityId = m_relations[relationIdx];
          dualKind = HAS_USE;
          if (
            (dualIndex = this->modelResource()->findArrangementInvolvingEntity(
               dualEntityId, dualKind, this->id())) >= 0)
          {
            duals.push_back(ArrangementReference(dualEntityId, dualKind, dualIndex));
            return true;
          }
        }
      }
      break;
    default:
      smtkErrorMacro(
        this->modelResource()->log(),
        "Asked to find dual of unknown kind of arrangement: " << kind << ".");
      break;
  }
  return false;
}

int Entity::consumeInvalidIndex(const smtk::common::UUID& uid)
{
  int result = m_firstInvalid;
  if (result < 0)
    return result; // no hole to consume
  m_relations[result] = uid;

  // Now update m_firstInvalid:
  // I. Are we already at the end of the relations? If so, we're done.
  if (m_firstInvalid == static_cast<int>(m_relations.size()) - 1)
  {
    m_firstInvalid = -1;
    return result;
  }

  // II. Keep looking past the entry we just consumed for the next invalid one.
  ++m_firstInvalid;
  UUIDArray::iterator it = m_relations.begin() + m_firstInvalid;
  for (; it != m_relations.end(); ++it, ++m_firstInvalid)
    if (!*it)
    {
      return result;
    }
  m_firstInvalid = -1;
  return result;
}

/**\brief Return the attributes associated with the entity
that are of type (or derived type) def.
  */
smtk::attribute::Attributes Entity::attributes(smtk::attribute::ConstDefinitionPtr def) const
{
  smtk::attribute::Attributes result;
  // If there was no definition return empty list
  if (def == nullptr)
  {
    return result;
  }
  auto attResource = def->resource();
  // If there is no resource then return an empty list
  if (attResource == nullptr)
  {
    return result;
  }

  auto atts = def->attributes(this->shared_from_this());
  result.insert(result.end(), atts.begin(), atts.end());
  return result;
}

EntityPtr Entity::owningModel() const
{
  auto myResource = this->modelResource();
  UUID modelId = myResource->modelOwningEntity(m_id);
  return myResource->findEntity(modelId);
}

} // namespace model
} //namespace smtk
