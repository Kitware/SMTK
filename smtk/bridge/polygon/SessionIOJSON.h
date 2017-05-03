//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_bridge_polygon_SessionIOJSON_h
#define smtk_bridge_polygon_SessionIOJSON_h

#include "smtk/model/SessionIOJSON.h"

#include "smtk/model/Face.h"

#include "smtk/bridge/polygon/Exports.h"
#include "smtk/bridge/polygon/internal/Config.h"
#include "smtk/bridge/polygon/internal/Vertex.h"

struct cJSON;

namespace smtk
{
namespace bridge
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

  virtual ~SessionIOJSON() {}

  virtual int saveJSON(
    cJSON* node, const smtk::model::SessionRef& sref, const smtk::model::Models& models) override;

  virtual int importJSON(smtk::model::ManagerPtr mgr, const smtk::model::SessionPtr& session,
    cJSON* sessionRec, bool loadNativeModels = false);
  virtual int exportJSON(smtk::model::ManagerPtr mgr, const smtk::model::SessionPtr& sessPtr,
    cJSON* sessionRec, bool writeNativeModels = false);
  virtual int exportJSON(smtk::model::ManagerPtr mgr, const smtk::model::SessionPtr& session,
    const common::UUIDs& modelIds, cJSON* sessionRec, bool writeNativeModels = false);

protected:
  cJSON* serializeModel(internal::ModelPtr pmodel, const smtk::model::Model& mod);
  cJSON* serializeFace(const smtk::model::Face& face);
  cJSON* serializeEdge(internal::EdgePtr edge, const smtk::model::Edge& e);
  cJSON* serializeVertex(internal::VertexPtr vert, const smtk::model::Vertex& v);
  cJSON* serializeIncidentEdgeRecord(const internal::vertex::incident_edge_data& rec);
  void serializeEntityBase(cJSON* inout, internal::EntityPtr ent);

  internal::ModelPtr deserializeModel(cJSON* record, const smtk::model::Model& m);
  void deserializeFace(cJSON* record, const smtk::model::Face& f);
  internal::EdgePtr deserializeEdge(cJSON* record, const smtk::model::Edge& e);
  internal::VertexPtr deserializeVertex(cJSON* record, const smtk::model::Vertex& v);
  internal::vertex::incident_edge_data deserializeIncidentEdgeRecord(cJSON* record);
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // smtk_bridge_polygon_SessionIOJSON_h
