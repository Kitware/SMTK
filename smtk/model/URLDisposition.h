//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_URLDisposition_h
#define smtk_model_URLDisposition_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.

namespace smtk
{
namespace model
{

// Ways to resolve differences between contents of memory and a file.
enum ModificationResolution
{
  OVERWRITE, //!< Favor the in-memory contents over the file/URL.
  IGNORE,    //!< Favor the file/URL contents over what's in memory.
  ABORT,     //!< Cancel the operation.
  MOVE, //!< Write a new file (to a different location) and modify references to the URL to match.
  ASK,  //!< Ask the user what to do.
};

struct SMTKCORE_EXPORT URLDisposition
{
  /// Identify what the disposition applies to:
  smtk::model::EntityRef entity;

  /// Identify original and updated properties (the updates are temporary if originals are present).
  smtk::model::StringData originalStringProperties;
  smtk::model::StringData updatedStringProperties;

  /** Indicate what should happen when the _entity_ is newer than
    * what is stored in the file.
    */
  ModificationResolution resolution;
};

} // namespace model
} // namespace smtk

#endif // smtk_model_URLDisposition_h
