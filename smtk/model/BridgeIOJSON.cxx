#include "smtk/model/BridgeIOJSON.h"

#include "smtk/model/Manager.h"

#include "cJSON.h"

namespace smtk {
  namespace model {

/**\brief Decode information from \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int BridgeIOJSON::importJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  return 1;
}

/**\brief Encode information into \a sessionRec for the given \a modelMgr.
  *
  * Subclasses should return 1 on success and 0 on failure.
  */
int BridgeIOJSON::exportJSON(ManagerPtr modelMgr, cJSON* sessionRec)
{
  (void)modelMgr;
  (void)sessionRec;
  return 1;
}

  } // namespace model
} // namespace smtk
