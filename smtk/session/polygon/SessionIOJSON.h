//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_SessionIOJSON_h
#define smtk_session_polygon_SessionIOJSON_h

#include "smtk/model/SessionIOJSON.h"

#include "smtk/session/polygon/Exports.h"
#include "smtk/session/polygon/Resource.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief A base class for delegating session I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
class SMTKPOLYGONSESSION_EXPORT SessionIOJSON : public smtk::model::SessionIOJSON
{
public:
  smtkTypeMacro(SessionIOJSON);
  smtkCreateMacro(SessionIOJSON);

  ~SessionIOJSON() override = default;

  /**\brief Serialize a resource into a set of JSON records.
    */
  static json saveJSON(const smtk::session::polygon::Resource::Ptr& rsrc);

  /**\brief Load JSON from a file and parse it, but do nothing more.
    */
  static json loadJSON(const std::string& filename);

  /**\brief Given JSON data, attempt to deserialize SMTK model records from it.
    *
    * If the JSON object represents model data, it is added to the
    * given resource.
    */
  static bool loadModelRecords(const json& j, smtk::session::polygon::Resource::Ptr& rsrc);
};

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_SessionIOJSON_h
