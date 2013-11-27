#ifndef __smtk_model_EntityTypeBits_h
#define __smtk_model_EntityTypeBits_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro
#include "smtk/util/SystemConfig.h"

namespace smtk {
  namespace model {

/// The integer type used to hold bit values describing an entity's type.
typedef unsigned int BitFlags;

/// Different types of entities the model can hold and their inherent properties.
enum EntityTypeBits
{
  // Dimensionality bits:
  DIMENSION_0          = 0x00000001, //!< The entity may include 0-dimensional components
  DIMENSION_1          = 0x00000002, //!< The entity may include 1-dimensional components
  DIMENSION_2          = 0x00000004, //!< The entity may include 2-dimensional components
  DIMENSION_3          = 0x00000008, //!< The entity may include 3-dimensional components
  DIMENSION_4          = 0x00000010, //!< The entity may include 4-dimensional components (where time is one dimension)
  // Entity type bits:
  CELL_ENTITY          = 0x00000100, //!< A bit indicating the entity is a cell
  USE_ENTITY           = 0x00000200, //!< A bit indicating the entity is a shell (a GroupingEntity in CGM parlance)
  SHELL_ENTITY         = 0x00000400, //!< A bit indicating the entity is a use (a SenseEntity in CGM parlance)
  GROUP_ENTITY         = 0x00000800, //!< A bit indicating a group; UUIDs may only occur 0 or 1 times. A separate flag describes constraints on group members.
  MODEL_ENTITY         = 0x00001000, //!< A bit indicating a (sub)model.
  INSTANCE_ENTITY      = 0x00002000, //!< A bit indicating an instance of model.
  // Inherent property bits (arguably inappropriate as they could be hard to maintain):
  COVER                = 0x00100000, //!< The entity must have a relation indicating which cover(s) it participates in
  PARTITION            = 0x00200000, //!< The entity must have a relation indicating which partition(s) it participates in
  OPEN                 = 0x00400000, //!< A bit indicating that the entity should be regarded as an open set (esp. groups)
  CLOSED               = 0x00800000, //!< A bit indicating that the entity should be regarded as a closed set (esp. groups)
  MODEL_BOUNDARY       = 0x01000000, //!< The entity is part of a boundary (esp. partition groups, indicating boundary conditions)
  MODEL_DOMAIN         = 0x02000000, //!< The entity is part of the model domain (esp. groups)
  // Specific bit-combinations of interest (just combinations of the above):
  ANY_DIMENSION        = 0x000000ff, //!< Mask to extract the dimensionality of an entity.
  ENTITY_MASK          = 0x00003f00, //!< Mask to extract the type of an entity. Exactly one bit should be set for any valid entity.
  ANY_ENTITY           = 0x00003fff, //!< Mask to extract the type and dimension of an entity.
  VERTEX               = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  EDGE                 = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  FACE                 = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  REGION               = 0x00000108, //!< A cell of dimension 3 (i.e., a region)
  CELL_0D              = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  CELL_1D              = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  CELL_2D              = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  CELL_3D              = 0x00000108, //!< A cell of dimension 3 (i.e., a region)
  ANY_CELL             = 0x000001ff, //!< A cell of any dimension
  VERTEX_USE           = 0x00000201, //!< A cell-use of dimension 0 (i.e., a vertex use)
  EDGE_USE             = 0x00000202, //!< A cell-use of dimension 1 (i.e., an edge use)
  FACE_USE             = 0x00000204, //!< A cell-use of dimension 2 (i.e., a face use)
  USE_0D               = 0x00000201, //!< A cell-use of dimension 0 (i.e., a vertex use)
  USE_1D               = 0x00000202, //!< A cell-use of dimension 1 (i.e., an edge use)
  USE_2D               = 0x00000204, //!< A cell-use of dimension 2 (i.e., a face use)
  ANY_USE              = 0x000002ff, //!< A cell-use of any dimension
  CHAIN                = 0x00000403, //!< A shell of dimension 0+1 (i.e., a vertex chain)
  LOOP                 = 0x00000406, //!< A shell of dimension 1+2 (i.e., an edge loop)
  SHELL                = 0x0000040c, //!< A shell of dimension 2+3 (i.e., a face shell)
  SHELL_0D             = 0x00000403, //!< A shell of dimension 0+1 (i.e., a vertex chain)
  SHELL_1D             = 0x00000406, //!< A shell of dimension 1+2 (i.e., an edge lop)
  SHELL_2D             = 0x0000040c, //!< A shell of dimension 2+3 (i.e., a face shell)
  ANY_SHELL            = 0x000004ff, //!< A shell of any dimension
  GROUP_0D             = 0x00000801, //!< A group of cells of dimension 0 (which may include other groups of dimension 0)
  GROUP_1D             = 0x00000802, //!< A group of cells of dimension 1 (which may include other groups of dimension 1)
  GROUP_2D             = 0x00000804, //!< A group of cells of dimension 2 (which may include other groups of dimension 2)
  GROUP_3D             = 0x00000808, //!< A group of cells of dimension 3 (which may include other groups of dimension 3)
  ANY_GROUP            = 0x000008ff, //!< A group of cells of any dimension (which may include other groups of any dimension)
  BOUNDARY_GROUP       = 0x01600800, //!< A boundary condition group (no dimension specified)
  DOMAIN_GROUP         = 0x02600800, //!< A model domain group (no dimension specified)
  HALF_OPEN            = 0x00c00000, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  HALF_CLOSED          = 0x00c00000, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  INVALID              = 0xffffffff  //!< The entity is invalid
};

inline bool isVertex(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == CELL_0D; }
inline bool isEdge(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == CELL_1D; }
inline bool isFace(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == CELL_2D; }
inline bool isRegion(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == CELL_3D; }

inline bool isVertexUse(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == USE_0D; }
inline bool isEdgeUse(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == USE_1D; }
inline bool isFaceUse(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == USE_2D; }

inline bool isChain(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == SHELL_0D; }
inline bool isLoop(BitFlags entityFlags)  { return (entityFlags & ANY_ENTITY) == SHELL_1D; }
inline bool isShell(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == SHELL_2D; }

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityTypeBits_h
