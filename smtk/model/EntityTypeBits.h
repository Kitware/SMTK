#ifndef __smtk_model_EntityTypeBits_h
#define __smtk_model_EntityTypeBits_h

#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT macro

namespace smtk {
  namespace model {

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
  SHELL_ENTITY         = 0x00000200, //!< A bit indicating the entity is a shell (a GroupingEntity in CGM parlance)
  USE_ENTITY           = 0x00000400, //!< A bit indicating the entity is a use (a SenseEntity in CGM parlance)
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
  ANY_DIMENSION        = 0x000000ff, //!< An entity of any dimension
  VERTEX               = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  EDGE                 = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  FACE                 = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  REGION               = 0x00000108, //!< A cell of dimension 3 (i.e., a region)
  CELL_0D              = 0x00000101, //!< A cell of dimension 0 (i.e., a vertex)
  CELL_1D              = 0x00000102, //!< A cell of dimension 1 (i.e., an edge)
  CELL_2D              = 0x00000104, //!< A cell of dimension 2 (i.e., a face)
  CELL_3D              = 0x00000108, //!< A cell of dimension 3 (i.e., a region)
  ANY_CELL             = 0x000001ff, //!< A cell of any dimension
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

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EntityTypeBits_h
