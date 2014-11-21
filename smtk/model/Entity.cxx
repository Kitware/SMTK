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

#include "smtk/common/StringUtil.h"

#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <cctype>

#include <sstream>

//#include <boost/variant.hpp>

using namespace std;
using namespace smtk::common;

namespace smtk {
  namespace model {

static const char* cellNamesByDimensionSingular[] = {
  "vertex",
  "edge",
  "face",
  "volume",
  "spacetime",
  "mixed-dimension cell"
};

static const char* cellNamesByDimensionPlural[] = {
  "vertices",
  "edges",
  "faces",
  "volumes",
  "spacetimes",
  "mixed-dimension cells"
};

/**\class smtk::model::Entity
  *
  * Store links between named entities.
  */

/// The default constructor creates an invalid link.
Entity::Entity()
  : m_entityFlags(INVALID)
{
}

/// Construct a link with the given \a dimension with a type specified by \a entityFlags.
Entity::Entity(BitFlags entFlags, int dim)
  : m_entityFlags(entFlags)
{
  // Override the dimension bits if the dimension is specified
  if (dim >= 0 && dim <= 4)
    {
    // Clear the dimension bits:
    this->m_entityFlags &= ~(
      DIMENSION_0 | DIMENSION_1 | DIMENSION_2 | DIMENSION_3 | DIMENSION_4);
    // Now add in the *proper* dimension bit to match m_dimension:
    this->m_entityFlags |= (1 << dim);
    }
}

/**\brief Return the bit vector describing the type and attributes of the associated entity.
  *
  * \sa smtk::model::EntityTypeBits
  */
BitFlags Entity::entityFlags() const
{
  return this->m_entityFlags;
}

bool Entity::setEntityFlags(BitFlags flags)
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
  BitFlags dimBits = this->m_entityFlags & ANY_DIMENSION;
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

BitFlags Entity::dimensionBits() const
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

int Entity::appendRelation(const UUID& b)
{
  int reln = static_cast<int>(this->m_relations.size());
  this->m_relations.push_back(b);
  return reln;
}

Entity& Entity::pushRelation(const UUID& b)
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

/**\brief Find the given relation \a r and return its index, inserting it if not present.
  */
int Entity::findOrAppendRelation(const UUID& r)
{
  for (UUIDArray::size_type i = 0; i < this->m_relations.size(); ++i)
    {
    if (this->m_relations[i] == r)
      {
      return static_cast<int>(i);
      }
    }
  int idx = static_cast<int>(this->m_relations.size());
  this->m_relations.push_back(r);
  return idx;
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
    if (flags & (1<<i))
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
      result = "mixed-dimension cell use";
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
    if (flags & MODEL_BOUNDARY) result += "boundary ";
    if (flags & MODEL_DOMAIN) result += "domain ";
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
    case DIMENSION_0: result = "point "; break;
    case DIMENSION_1: result = "curve "; break;
    case DIMENSION_2: result = "surface "; break;
    case DIMENSION_3: result = "volumetric "; break;
    case DIMENSION_4: result = "space-time "; break;
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
  case BRIDGE_SESSION:
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
      if (dimSep) result += "-d";
      }

    bool entSep = false;
    for (BitFlags entType = CELL_ENTITY; entType < ENTITY_MASK; entType <<= 1)
      {
      if ((entType & GROUP_ENTITY) && NO_SUBGROUPS)
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
        case CELL_ENTITY: result += "cells"; break;
        case USE_ENTITY: result += "uses"; break;
        case SHELL_ENTITY: result += "shells"; break;
        case GROUP_ENTITY: result += "groups"; break;
        case MODEL_ENTITY: result += "models"; break;
        case INSTANCE_ENTITY: result += "instances"; break;
        case BRIDGE_SESSION: result += "bridge sessions"; break;
        default: break;
          }
        }
      }
    if (dimSep && !entSep)
      result += " entities";
    if (dimSep || entSep)
      result += ")";

    if (flags & COVER) result += " cover";
    if (flags & PARTITION) result += " partition";
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

/**\brief Given a vector of counters, return a numbered name string and advance counters as required.
  *
  * If you change this method, you may also need to change flagSummmary/flagDescription.
  */
std::string Entity::defaultNameFromCounters(BitFlags flags, IntegerList& counters)
{
  std::ostringstream name;
  name << Entity::flagSummaryHelper(flags, /*singular*/ 0) << " ";
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
      (((flags & MODEL_BOUNDARY) != 0) ^
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
  case BRIDGE_SESSION:
  default:
    name << counters[0]++;
    break;
    }
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
    if (val == smtk::model::INVALID)     spec << "invalid";

    else if (val == smtk::model::VERTEX) spec << "vertex";
    else if (val == smtk::model::EDGE)   spec << "edge";
    else if (val == smtk::model::FACE)   spec << "face";
    else if (val == smtk::model::VOLUME) spec << "volume";

    else if (val == smtk::model::VERTEX_USE) spec << "vertex_use";
    else if (val == smtk::model::EDGE_USE)   spec << "edge_use";
    else if (val == smtk::model::FACE_USE)   spec << "face_use";
    else if (val == smtk::model::VOLUME_USE) spec << "volume_use";

    else if (val == smtk::model::CHAIN) spec << "chain";
    else if (val == smtk::model::LOOP)  spec << "loop";
    else if (val == smtk::model::SHELL) spec << "shell2";

    // Not a specially-named bit value; generate a name:
    else
      {
      // Put entity type bits first and ensure they are all non-numeric.
      bool haveType = false;
      if (isCellEntity(val))     {                                 haveType = true; spec << "cell"; }
      if (isUseEntity(val))      { if (haveType) spec << "|"; else haveType = true; spec << "use"; }
      if (isShellEntity(val))    { if (haveType) spec << "|"; else haveType = true; spec << "shell"; }
      if (isGroupEntity(val))    { if (haveType) spec << "|"; else haveType = true; spec << "group"; }
      if (isModelEntity(val))    { if (haveType) spec << "|"; else haveType = true; spec << "model"; }
      if (isInstanceEntity(val)) { if (haveType) spec << "|"; else haveType = true; spec << "instance"; }
      if (isBridgeSession(val))  { if (haveType) spec << "|"; else haveType = true; spec << "bridge"; }

      if (!haveType)             { spec << "none"; }

      if (hasBit<smtk::model::MODEL_DOMAIN>(val))     spec << "|domain";
      if (hasBit<smtk::model::MODEL_BOUNDARY>(val))   spec << "|bdy";

      if (hasBit<smtk::model::COVER>(val))            spec << "|cover";
      if (hasBit<smtk::model::PARTITION>(val))        spec << "|partition";

      if (hasBit<smtk::model::OPEN>(val))             spec << "|open";
      if (hasBit<smtk::model::CLOSED>(val))           spec << "|closed";

      if (hasBit<smtk::model::HOMOGENOUS_GROUP>(val)) spec << "|homg";

      if (hasBit<smtk::model::NO_SUBGROUPS>(val))     spec << "|flat";

      if (hasBit<smtk::model::ANY_DIMENSION>(val))    spec << "|anydim";
      else if (!(val & smtk::model::ANY_DIMENSION))   spec << "|nodim";
      else
        {
        spec << "|";
        // Note that explicit dimension bits are not separated; they are only 1 character.
        if (hasBit<smtk::model::DIMENSION_0>(val))    spec << "0";
        if (hasBit<smtk::model::DIMENSION_1>(val))    spec << "1";
        if (hasBit<smtk::model::DIMENSION_2>(val))    spec << "2";
        if (hasBit<smtk::model::DIMENSION_3>(val))    spec << "3";
        if (hasBit<smtk::model::DIMENSION_4>(val))    spec << "4";
        }
      }
    }
  else
    {
    spec << val;
    }
  return spec.str();
}

static struct {
  std::string name;
  BitFlags value;
} orderedValues[] = {
  { "0",          smtk::model::DIMENSION_0 },
  { "1",          smtk::model::DIMENSION_1 },
  { "2",          smtk::model::DIMENSION_2 },
  { "3",          smtk::model::DIMENSION_3 },
  { "4",          smtk::model::DIMENSION_4 },
  { "anydim",     smtk::model::ANY_DIMENSION },
  { "b",          smtk::model::BRIDGE_SESSION },
  { "bdy",        smtk::model::MODEL_BOUNDARY },
  { "bridge",     smtk::model::BRIDGE_SESSION },
  { "cell",       smtk::model::CELL_ENTITY },
  { "chain",      smtk::model::CHAIN },
  { "closed",     smtk::model::CLOSED },
  { "cover",      smtk::model::COVER },
  { "domain",     smtk::model::MODEL_DOMAIN },
  { "e",          smtk::model::EDGE }, // Backwards-compatibility
  { "edge",       smtk::model::EDGE },
  { "edge_use",   smtk::model::EDGE_USE },
  { "ef",         smtk::model::EDGE | smtk::model::FACE }, // Backwards-compatibility
  { "efr",        smtk::model::EDGE | smtk::model::FACE | smtk::model::VOLUME }, // Backwards-compatibility
  { "ev",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "f",          smtk::model::FACE }, // Backwards-compatibility
  { "face",       smtk::model::FACE },
  { "face_use",   smtk::model::FACE_USE },
  { "fe",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "fev",        smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "flat",       smtk::model::NO_SUBGROUPS },
  { "fr",         smtk::model::FACE | smtk::model::VOLUME }, // Backwards-compatibility
  { "fv",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "g",          smtk::model::GROUP_ENTITY }, // Backwards compatibility
  { "gmrfev",     smtk::model::GROUP_ENTITY | smtk::model::MODEL_ENTITY | smtk::model::CELL_ENTITY | smtk::model::ANY_DIMENSION }, // Backwards compatibility
  { "group",      smtk::model::GROUP_ENTITY },
  { "homg",       smtk::model::HOMOGENOUS_GROUP },
  { "instance",   smtk::model::INSTANCE_ENTITY },
  { "invalid",    smtk::model::INVALID },
  { "loop",       smtk::model::LOOP },
  { "m",          smtk::model::MODEL_ENTITY }, // Backwards compatibility
  { "model",      smtk::model::MODEL_ENTITY },
  { "mrfev",      smtk::model::MODEL_ENTITY | smtk::model::CELL_ENTITY | smtk::model::ANY_DIMENSION }, // Backwards compatibility
  { "nodim",      0 },
  { "none",       0 },
  { "open",       smtk::model::OPEN },
  { "partition",  smtk::model::PARTITION },
  { "r",          smtk::model::VOLUME }, // Backwards compatibility
  { "re",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "region",     smtk::model::VOLUME },
  { "rev",        smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rf",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 }, // Backwards compatibility
  { "rfe",        smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_1 }, // Backwards compatibility
  { "rfev",       smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rfv",        smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_2 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "rv",         smtk::model::CELL_ENTITY | smtk::model::DIMENSION_3 | smtk::model::DIMENSION_0 }, // Backwards compatibility
  { "shell",      smtk::model::SHELL_ENTITY },
  { "shell2",     smtk::model::SHELL },
  { "use",        smtk::model::USE_ENTITY },
  { "v",          smtk::model::VERTEX }, // Backwards-compatibility
  { "ve",         smtk::model::VERTEX | smtk::model::EDGE }, // Backwards-compatibility
  { "vef",        smtk::model::VERTEX | smtk::model::EDGE | smtk::model::FACE }, // Backwards-compatibility
  { "vefr",       smtk::model::VERTEX | smtk::model::EDGE | smtk::model::FACE | smtk::model::VOLUME }, // Backwards-compatibility
  { "vertex",     smtk::model::VERTEX },
  { "vertex_use", smtk::model::VERTEX_USE },
  { "volume",     smtk::model::VOLUME },
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

  std::string proper = keyword; // make a mutable copy.
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
  case 0: return DIMENSION_0;
  case 1: return DIMENSION_1;
  case 2: return DIMENSION_2;
  case 3: return DIMENSION_3;
  case 4: return DIMENSION_4;
  default: break;
    }
  return 0;
}

  } // namespace model
} //namespace smtk
