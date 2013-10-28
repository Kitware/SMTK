#ifndef __smtk_model_Link_h
#define __smtk_model_Link_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

typedef std::set<smtk::util::UUID> UUIDs;
typedef std::vector<smtk::util::UUID> UUIDArray;
typedef std::vector<UUIDArray> UUIDArrays;

/// Different types of entities the model can hold
enum EntityTypeBits
  {
  // Dimensionality bits:
  DIMENSION_0          = 0x0001,
  DIMENSION_1          = 0x0002,
  DIMENSION_2          = 0x0004,
  DIMENSION_3          = 0x0008,
  // It might be nice to reserve a bit for time in case we have to deal with space-time finite elements.
  // Entity type bits:
  CELL_ENTITY          = 0x0010, //!< A bit indicating the entity is a cell
  SHELL_ENTITY         = 0x0020, //!< A bit indicating the entity is a shell (a GroupingEntity in CGM parlance)
  USE_ENTITY           = 0x0040, //!< A bit indicating the entity is a use (a SenseEntity in CGM parlance)
  GROUP_ENTITY         = 0x0080, //!< A bit indicating a group; combine with CELL bits to specify dimension
  // Inherent property bits (arguably inappropriate as they could be hard to maintain):
  COVER                = 0x0100, //!< The entity must have a relation indicating which cover(s) it participates in
  PARTITION            = 0x0200, //!< The entity must have a relation indicating which partition(s) it participates in
  OPEN                 = 0x0400, //!< A bit indicating that the entity should be regarded as an open set (esp. groups)
  CLOSED               = 0x0800, //!< A bit indicating that the entity should be regarded as a closed set (esp. groups)
  MODEL_BOUNDARY       = 0x1000, //!< The entity is part of a boundary (esp. partition groups, indicating boundary conditions)
  MODEL_DOMAIN         = 0x2000, //!< The entity is part of the model domain (esp. groups)
  // Specific bit-combinations of interest (just combinations of the above):
  ANY_DIMENSION        = 0x000f, //!< An entity of any dimension
  VERTEX               = 0x0011, //!< A cell of dimension 0 (i.e., a vertex)
  EDGE                 = 0x0012, //!< A cell of dimension 1 (i.e., an edge)
  FACE                 = 0x0014, //!< A cell of dimension 2 (i.e., a face)
  REGION               = 0x0018, //!< A cell of dimension 3 (i.e., a region)
  CELL_0D              = 0x0011, //!< A cell of dimension 0 (i.e., a vertex)
  CELL_1D              = 0x0012, //!< A cell of dimension 1 (i.e., an edge)
  CELL_2D              = 0x0014, //!< A cell of dimension 2 (i.e., a face)
  CELL_3D              = 0x0018, //!< A cell of dimension 3 (i.e., a region)
  ANY_CELL             = 0x001f, //!< A cell of any dimension
  GROUP_0D             = 0x0081, //!< A group of cells of dimension 0 (which may include other groups of dimension 0)
  GROUP_1D             = 0x0082, //!< A group of cells of dimension 1 (which may include other groups of dimension 1)
  GROUP_2D             = 0x0084, //!< A group of cells of dimension 2 (which may include other groups of dimension 2)
  GROUP_3D             = 0x0088, //!< A group of cells of dimension 3 (which may include other groups of dimension 3)
  ANY_GROUP            = 0x008f, //!< A group of cells of any dimension (which may include other groups of any dimension)
  BOUNDARY_CONDITION   = 0x1280, //!< A boundary condition group (no dimension specified)
  DOMAIN_SET           = 0x2280, //!< A model domain group (no dimension specified)
  HALF_OPEN            = 0x0c00, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  HALF_CLOSED          = 0x0c00, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  INVALID              = 0xffff  //!< The entity is invalid
  };

class Link
{
public:
  Link();
  Link(int dimension, int entityFlags);

  int dimension() const;
  int entityFlags() const;

  UUIDArray& relations();
  const UUIDArray& relations() const;

  Link& appendRelation(const smtk::util::UUID& b);
  Link& removeRelation(const smtk::util::UUID& b);

protected:
  int EntityFlags;
  int Dimension;
  UUIDArray Relations;
private:
};

typedef std::pair<smtk::util::UUID,Link> UUIDLinkPair;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Link_h
