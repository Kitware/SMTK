//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionIOJSON_h
#define __smtk_model_SessionIOJSON_h

#include "smtk/model/SessionIO.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace model
{

/**\brief A base class for delegating session I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
class SMTKCORE_EXPORT SessionIOJSON : public SessionIO
{
public:
  using json = nlohmann::json;

  smtkTypeMacro(SessionIOJSON);
  smtkCreateMacro(SessionIOJSON);

  ~SessionIOJSON() override {}

  /**\brief Serialize a resource into a set of JSON records.
    */
  static json saveJSON(const smtk::model::ResourcePtr& rsrc);

  /**\brief Write a set of JSON records to the given location.
    */
  static bool saveModelRecords(const json& j, const std::string& url);

  /**\brief Load JSON from a file and parse it, but do nothing more.
    */
  static json loadJSON(const std::string& filename);

  /**\brief Given JSON data, attempt to deserialize SMTK model records from it.
    *
    * If the JSON object represents model data, it is added to the
    * given resource.
    */
  static bool loadModelRecords(const json& j, smtk::model::ResourcePtr rsrc);
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_SessionIOJSON_h
