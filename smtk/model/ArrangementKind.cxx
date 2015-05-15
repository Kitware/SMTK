//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/ArrangementKind.h"

namespace smtk {
 namespace model {

const char* ArrangementKindName[KINDS_OF_ARRANGEMENTS + 1] = {
  "inclusion",    // INCLUDES
  "cell",         // HAS_CELL
  "shell",        // HAS_SHELL
  "use",          // HAS_USE
  "embedding",    // EMBEDDED_IN
  "subset",       // SUBSET_OF
  "superset",     // SUPERSET_OF
  "instantiates", // INSTANCE_OF
  "instance",     // INSTANCED_BY
  "invalid"       // KINDS_OF_ARRANGEMENTS
};

const char* ArrangementKindAbbr[KINDS_OF_ARRANGEMENTS + 1] = {
  "i", // INCLUDES
  "c", // HAS_CELL
  "s", // HAS_SHELL
  "u", // HAS_USE
  "e", // EMBEDDED_IN
  "b", // SUBSET_OF (suBset)
  "r", // SUPERSET_OF (supeRset)
  "p", // INSTANCE_OF (Prototype)
  "x", // INSTANCED_BY (eXemplar)
  "?"  // KINDS_OF_ARRANGEMENTS
};

ArrangementKind ArrangementKindFromName(const std::string& name)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (name == ArrangementKindName[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string NameForArrangementKind(ArrangementKind k)
{
  return ArrangementKindName[k];
}

ArrangementKind ArrangementKindFromAbbreviation(const std::string& abbr)
{
  for (int i = 0; i < KINDS_OF_ARRANGEMENTS; ++i)
    {
    if (abbr == ArrangementKindAbbr[i])
      {
      return ArrangementKind(i);
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

std::string AbbreviationForArrangementKind(ArrangementKind k)
{
  return ArrangementKindAbbr[k];
}

// Dual arrangement type when the source of the input arrangement is a cell.
static ArrangementKind ArrangementKindDualCell[KINDS_OF_ARRANGEMENTS] = {
  EMBEDDED_IN,           // dual of INCLUDES
  KINDS_OF_ARRANGEMENTS, // dual of HAS_CELL is invalid; cells don't have other cells... they include them
  HAS_CELL,              // dual of HAS_SHELL; this should probably be invalid
  HAS_CELL,              // dual of HAS_USE
  INCLUDES,              // dual of EMBEDDED_IN
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; only instances may be instance_of
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is a use.
static ArrangementKind ArrangementKindDualUse[KINDS_OF_ARRANGEMENTS] = {
  EMBEDDED_IN,           // dual of INCLUDES
  HAS_USE,               // dual of HAS_CELL
  HAS_USE,               // dual of HAS_SHELL
  KINDS_OF_ARRANGEMENTS, // dual of HAS_USE is invalid; a use may not have another use.
  INCLUDES,              // dual of EMBEDDED_IN
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; only instances may be instance_of
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is a shell.
static ArrangementKind ArrangementKindDualShell[KINDS_OF_ARRANGEMENTS] = {
  EMBEDDED_IN,           // dual of INCLUDES
  HAS_SHELL,             // dual of HAS_CELL should probably be invalid...
  KINDS_OF_ARRANGEMENTS, // dual of HAS_SHELL is invalid; a shell cannot have another shell.
  HAS_SHELL,             // dual of HAS_USE
  INCLUDES,              // dual of EMBEDDED_IN
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; only instances may be instance_of
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is a group.
static ArrangementKind ArrangementKindDualGroup[KINDS_OF_ARRANGEMENTS] = {
  EMBEDDED_IN,           // dual of INCLUDES
  KINDS_OF_ARRANGEMENTS, // dual of HAS_CELL is invalid; a group does not have cells (it supersets them)
  KINDS_OF_ARRANGEMENTS, // dual of HAS_SHELL is invalid; a group does not have uses (it supersets them)
  KINDS_OF_ARRANGEMENTS, // dual of HAS_USE is invalid; a group does not have shells (it supersets them)
  INCLUDES,              // dual of EMBEDDED_IN
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; only instances may be instance_of
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is a model.
static ArrangementKind ArrangementKindDualModel[KINDS_OF_ARRANGEMENTS] = {
  EMBEDDED_IN,           // dual of INCLUDES
  KINDS_OF_ARRANGEMENTS, // dual of HAS_CELL is invalid; a model does not have cells (it includes them)
  KINDS_OF_ARRANGEMENTS, // dual of HAS_SHELL is invalid; a model does not have uses (it includes them)
  KINDS_OF_ARRANGEMENTS, // dual of HAS_USE is invalid; a model does not have shells (it includes them)
  INCLUDES,              // dual of EMBEDDED_IN
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; only instances may be instance_of
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is an instance.
static ArrangementKind ArrangementKindDualInstance[KINDS_OF_ARRANGEMENTS] = {
  KINDS_OF_ARRANGEMENTS, // dual of INCLUDES is invalid; instances may not include things
  KINDS_OF_ARRANGEMENTS, // dual of HAS_CELL is invalid; instances may not include things
  KINDS_OF_ARRANGEMENTS, // dual of HAS_SHELL is invalid; instances may not include things
  KINDS_OF_ARRANGEMENTS, // dual of HAS_USE is invalid; instances may not include things
  KINDS_OF_ARRANGEMENTS, // dual of EMBEDDED_IN is invalid; instances may not include things
  SUPERSET_OF,           // dual of SUBSET_OF
  SUBSET_OF,             // dual of SUPERSET_OF
  INSTANCED_BY,          // dual of INSTANCE_OF
  INSTANCE_OF,           // dual of INSTANCED_BY
};

// Dual arrangement type when the source of the input arrangement is a session.
static ArrangementKind ArrangementKindDualSession[KINDS_OF_ARRANGEMENTS] = {
  KINDS_OF_ARRANGEMENTS, // dual of INCLUDES is invalid; a session does not include entities
  KINDS_OF_ARRANGEMENTS, // dual of HAS_CELL is invalid; a session does not have cells
  KINDS_OF_ARRANGEMENTS, // dual of HAS_SHELL is invalid; a session does not have shells
  KINDS_OF_ARRANGEMENTS, // dual of HAS_USE is invalid; a session does not have uses
  KINDS_OF_ARRANGEMENTS, // dual of EMBEDDED_IN is invalid; a session is never embedded in an entity
  KINDS_OF_ARRANGEMENTS, // dual of SUBSET_OF is invalid; a session is never a subset entry
  SUBSET_OF,             // dual of SUPERSET_OF
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCE_OF is invalid; a session is not an instance of anything
  KINDS_OF_ARRANGEMENTS, // dual of INSTANCED_BY is invalid; a session may not be instanced.
};

ArrangementKind Dual(EntityTypeBits entType, ArrangementKind k)
{
  if (k >= static_cast<ArrangementKind>(0) && k < KINDS_OF_ARRANGEMENTS)
    {
    switch (entType & ENTITY_MASK)
      {
    case CELL_ENTITY    : return ArrangementKindDualCell[k];
    case USE_ENTITY     : return ArrangementKindDualUse[k];
    case SHELL_ENTITY   : return ArrangementKindDualShell[k];
    case GROUP_ENTITY   : return ArrangementKindDualGroup[k];
    case MODEL_ENTITY   : return ArrangementKindDualModel[k];
    case INSTANCE_ENTITY: return ArrangementKindDualInstance[k];
    case SESSION        : return ArrangementKindDualSession[k];
    default:
      // Groups can have membership constraint bits set:
      if (entType & GROUP_ENTITY)
        return ArrangementKindDualGroup[k];
      // Otherwise: Invalid... fall through
      break;
      }
    }
  return KINDS_OF_ARRANGEMENTS;
}

  } //namespace model
} // namespace smtk
