//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionIOJSON.h"

#include "smtk/model/Manager.h"

#include "cJSON.h"

namespace smtk {
  namespace model {

/**\brief Decode information from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::importJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  (void)modelIds;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelId of the \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int SessionIOJSON::exportJSON(ManagerPtr modelMgr, const smtk::common::UUIDs& modelIds, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)modelIds;
  (void)sessionRec;
  return 1;
}

  } // namespace model
} // namespace smtk
