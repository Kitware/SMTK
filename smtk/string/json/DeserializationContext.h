//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_string_json_DeserializationContext_h
#define smtk_string_json_DeserializationContext_h

#include "smtk/string/Manager.h"

#include <nlohmann/json.hpp>

namespace smtk
{
namespace string
{

/**\brief Manage the lifetime of a string-manager's deserialization translation table.
  *
  * The string manager holds a translation table mapping hash values from those
  * found in a file (which may have been generated on a platform with a different
  * hashing function) into hashes found in the manager.
  * Instances of this class keep the translation table from the file (or stream)
  * alive only as long as this instance is extant.
  *
  * This class deserializes the JSON structure passed to it upon construction,
  * increments the translation-table depth-counter, and possibly adds entries
  * to the manager's translation table.
  * At deletion, the manager's translation-table depth-counter is decremented
  * and, if 0, the table is cleared to free storage.
  * Note that translation tables are thread-local storage, so all
  * deserialization of tokens must be performed on the same thread as this
  * class instance.
  */
class SMTKCORE_EXPORT DeserializationContext
{
public:
  DeserializationContext(std::shared_ptr<smtk::string::Manager> m, const nlohmann::json& j);
  ~DeserializationContext();

protected:
  std::shared_ptr<smtk::string::Manager> m_manager;
};

} // namespace string
} // namespace smtk

#endif // smtk_string_json_DeserializationContext_h
