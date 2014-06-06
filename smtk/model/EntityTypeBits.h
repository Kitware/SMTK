#ifndef __smtk_model_EntityTypeBits_h
#define __smtk_model_EntityTypeBits_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro
#include "smtk/util/SystemConfig.h"

namespace smtk {
  namespace model {

/// The integer type used to hold bit values describing an entity's type.
typedef unsigned int BitFlags;

/**\brief Different types of entities the model can hold and their inherent properties.
  *
  * This enum is used to specify entity types as well as masks for
  * various operations on entities (e.g., determining suitability
  * for group membership).
  *
  * Entity types are stored as bit vectors (of type smtk::model::BitFlags)
  * whose bits are values from this enumeration.
  *
  * The lower 5 bits are reserved for entity dimensionality (DIMENSION_0 through
  * DIMENSION_4). These do not refer to specific dimensions but rather indicate
  * the number of dimensions required to parameterize the point locus of the
  * entity.
  * Shell entities have 2 dimension bits set: the dimension of their
  * boundary and the dimension of the space they enclose.
  * Groups use these bits to constrain membership.
  * Instances need not have any of these bits set.
  * All other entities should have exactly one of these bits set.
  * (For models, this bit indicates the maximum *parametric* dimension
  * of any entity contained in the model.)
  *
  * The next 8 bits are reserved for entity types (currently only 6 are used).
  * Every entity should have exactly one of these bits set, except group
  * entities which may use the other bits to indicate constraints on membership.
  *
  * The next 8 bits identify immutable properties of entities.
  * COVER and PARTITION may be applied to group entities to indicate that they
  * must form a (not necessarily disjoint) cover or (precisely disjoint) partition
  * of the point-locus their top-level member describes. The COVER and PARTITION
  * bits are mutually exclusive.
  * A group with the COVER bit set may have subgroups that overlap.
  * The union of the subgroups is the space covered.
  * A group with the PARTITION bit set must have subgroups that do not overlap.
  * The union of the subgroups is the space partitioned.
  *
  * The OPEN and CLOSED bits indicate that the point-set of the designated
  * aentities should be considered as OPEN (not containing its closure)
  * or CLOSED (containing its closure). This is largely intended for marking
  * groups to indicate that, for example, if a FaceEntity is in a CLOSED group
  * then its bounding edges and vertices should also be considered members
  * of the group. However, it may be used in the future to accommodate
  * modeling kernels that represent point locii differently than SMTK by
  * marking topological entities themselves (rather than groups).
  *
  * The MODEL_DOMAIN and MODEL_BOUNDARY bits are intended to be
  * set on groups to constrain their membership to items to the
  * dimensionality of the model to which the group belongs.
  *
  * The HOMOGENOUS_GROUP bit indicates that a group should
  * contain entities of the same type.
  *
  * Finally, because groups may have entity-type bits (e.g. CELL_ENTITY)
  * other than GROUP_ENTITY set in order to constrain membership, this
  * bit can be used to indicate whether or not the group may contain
  * other groups.
  *
  * Besides these bits, there are many enumerants that combine bits
  * into meaningful specifications for entities:
  * VERTEX (CELL_0D),    EDGE (CELL_1D),      FACE (CELL_2D),      VOLUME (CELL_3D),
  * VERTEX_USE (USE_0D), EDGE_USE (USE_1D),   FACE_USE (USE_2D),   USE_3D,
  * CHAIN (SHELL_0D),    LOOP (SHELL_0D),     SHELL (SHELL_2D),
  * GROUP_0D,            GROUP_1D,            GROUP_2D,            GROUP_3D,
  * BOUNDARY_GROUP, and  DOMAIN_GROUP.
  *
  * There are also enumerants meant to be used as masks to
  * separate out bits into functional groups:
  * ANY_DIMENSION, ENTITY_MASK, ANY_ENTITY, ANY_GROUP, ANY_SHELL,
  * ANY_CELL, ANY_USE, and GROUP_CONSTRAINT_MASK.
  *
  * Finally, the INVALID enumerant indicates that an entity does not
  * exist or should be considered ill-posed.
  */
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
  HOMOGENOUS_GROUP     = 0x04000000, //!< The group must contain entities of the same type (except that subgroups are allowed).
  NO_SUBGROUPS         = 0x08000000, //!< The group is not allowed to contain other groups if this flag is set.
  // Specific bit-combinations of interest (just combinations of the above):
  ANY_DIMENSION        = 0x000000ff, //!< Mask to extract the dimensionality of an entity.
  ENTITY_MASK          = 0x00003f00, //!< Mask to extract the type of an entity. Exactly one bit should be set for any valid entity.
  ANY_ENTITY           = 0x00003fff, //!< Mask to extract the type and dimension of an entity.
  VERTEX               = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  EDGE                 = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  FACE                 = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  VOLUME               = 0x00000108, //!< A cell of dimension 3 (i.e., a volume)
  CELL_0D              = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  CELL_1D              = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  CELL_2D              = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  CELL_3D              = 0x00000108, //!< A cell of dimension 3 (i.e., a volume)
  ANY_CELL             = 0x000001ff, //!< A cell of any dimension
  VERTEX_USE           = 0x00000201, //!< A cell-use of dimension 0 (i.e., a vertex use)
  EDGE_USE             = 0x00000202, //!< A cell-use of dimension 1 (i.e., an edge use)
  FACE_USE             = 0x00000204, //!< A cell-use of dimension 2 (i.e., a face use)
  USE_0D               = 0x00000201, //!< A cell-use of dimension 0 (i.e., a vertex use)
  USE_1D               = 0x00000202, //!< A cell-use of dimension 1 (i.e., an edge use)
  USE_2D               = 0x00000204, //!< A cell-use of dimension 2 (i.e., a face use)
  USE_3D               = 0x00000208, //!< A cell-use of dimension 3 (i.e., a volume use)
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
  GROUP_CONSTRAINT_MASK= 0x0ff00000, //!< Must a group be homogenous, have no subgroups, be a partition or cover, be open/closed (i.e., include its closure, or be composed only of model bdy/domain entities?
  HALF_OPEN            = 0x00c00000, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  HALF_CLOSED          = 0x00c00000, //!< A bit indicating that the entity should be regarded as half-open (or half-closed)
  INVALID              = 0xffffffff  //!< The entity is invalid
};

inline bool isCellEntity(BitFlags entityFlags) { return (entityFlags & ENTITY_MASK) == CELL_ENTITY; }
inline bool isVertex(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == CELL_0D; }
inline bool isEdge(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == CELL_1D; }
inline bool isFace(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == CELL_2D; }
inline bool isVolume(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == CELL_3D; }

inline bool isUseEntity(BitFlags entityFlags) { return (entityFlags & ENTITY_MASK) == USE_ENTITY; }
inline bool isVertexUse(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == USE_0D; }
inline bool isEdgeUse(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == USE_1D; }
inline bool isFaceUse(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == USE_2D; }
inline bool isVolumeUse(BitFlags entityFlags)   { return (entityFlags & ANY_ENTITY) == USE_3D; }

inline bool isShellEntity(BitFlags entityFlags) { return (entityFlags & ENTITY_MASK) == SHELL_ENTITY; }
inline bool isChain(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == SHELL_0D; }
inline bool isLoop(BitFlags entityFlags)  { return (entityFlags & ANY_ENTITY) == SHELL_1D; }
inline bool isShell(BitFlags entityFlags) { return (entityFlags & ANY_ENTITY) == SHELL_2D; }

inline bool isGroupEntity(BitFlags entityFlags)    { return (entityFlags & ENTITY_MASK) == GROUP_ENTITY; }
inline bool isModelEntity(BitFlags entityFlags)    { return (entityFlags & ENTITY_MASK) == MODEL_ENTITY; }
inline bool isInstanceEntity(BitFlags entityFlags) { return (entityFlags & ENTITY_MASK) == INSTANCE_ENTITY; }

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityTypeBits_h
