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
int SessionIOJSON::importJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              cJSON* sessionRec,
                              bool loadNativeModels)
{
  (void)modelMgr;
  (void)session;
  (void)sessionRec;
  (void)loadNativeModels;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
// ++ 1 ++
int SessionIOJSON::exportJSON(ManagerPtr modelMgr,
                              const SessionPtr& session,
                              cJSON* sessionRec,
                              bool writeNativeModels)
{
  (void)modelMgr;
  (void)session;
  (void)sessionRec;
  (void)writeNativeModels;
  return 1;
}
// -- 1 --

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  * This variant should export only information for the given models.
  */
// ++ 3 ++
int SessionIOJSON::exportJSON(ManagerPtr modelMgr, const SessionPtr& session,
                         const common::UUIDs &modelIds, cJSON* sessionRec,
                         bool writeNativeModels)
{
  (void)modelMgr;
  (void)session;
  (void)modelIds;
  (void)sessionRec;
  (void)writeNativeModels;
  return 1;
}
// -- 3 --

  } // namespace model
} // namespace smtk
