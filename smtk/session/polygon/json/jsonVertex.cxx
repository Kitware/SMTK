//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/json/jsonVertex.h"

#include "smtk/session/polygon/internal/Vertex.h"

#include "smtk/model/json/jsonTessellation.h"

#include <exception>
#include <iostream>
// Define how polygon resources are serialized.
using json = nlohmann::json;
namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{
void to_json(json& j, const smtk::session::polygon::internal::vertex::Ptr& vertex)
{
  j["type"] = "vertex";

  if (vertex->parent())
  {
    j["parent"] = vertex->parent()->id().toString();
  }

  // Store exact integer point coordinates safely:
  std::vector<long> ptdata(8, 0);
  smtk::session::polygon::internal::Coord c = vertex->point().x();
  ptdata[0] = c & 0xffff;
  c >>= 16;
  ptdata[1] = c & 0xffff;
  c >>= 16;
  ptdata[2] = c & 0xffff;
  c >>= 16;
  ptdata[3] = c & 0xffff;
  c = vertex->point().y();
  ptdata[4] = c & 0xffff;
  c >>= 16;
  ptdata[5] = c & 0xffff;
  c >>= 16;
  ptdata[6] = c & 0xffff;
  c >>= 16;
  ptdata[7] = c & 0xffff;
  j["point"] = ptdata;

  json iearr = json::array();

  // Store CCW-ordered list of incident-edge records:
  smtk::session::polygon::internal::vertex::incident_edges::const_iterator ieit;
  for (ieit = vertex->edgesBegin(); ieit != vertex->edgesEnd(); ++ieit)
  {
    json edgeRec = json::object();
    if (ieit->edgeId())
    {
      edgeRec["edge"] = ieit->edgeId().toString();
      edgeRec["edgeout"] = ieit->isEdgeOutgoing();
    }
    if (ieit->clockwiseFaceId())
    {
      edgeRec["cwface"] = ieit->clockwiseFaceId().toString();
    }
    iearr.push_back(edgeRec);
  }
  j["edges"] = iearr;
}

void from_json(const json& j, smtk::session::polygon::internal::vertex::Ptr& vertex)
{
  vertex = smtk::session::polygon::internal::vertex::create();

  try
  {
    std::vector<long> ptdata = j.at("point");
    // Set the integer point coordinates
    const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
    std::size_t np = ptdata.size() / stride;
    std::vector<long>::const_iterator cit = ptdata.begin();
    if (np > 0)
    {
      smtk::session::polygon::internal::Coord xy[2] = { 0, 0 };
      for (int jj = 0; jj < 2; ++jj)
      {
        unsigned long long& uc(*reinterpret_cast<unsigned long long*>(&xy[jj]));
        uc = *cit;
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 16);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 32);
        ++cit;
        uc |= (static_cast<unsigned long long>(*cit) << 48);
        ++cit;
      }
      vertex->point() = smtk::session::polygon::internal::Point(xy[0], xy[1]);
    }
  }
  catch (std::exception& /*e*/)
  {
    std::cerr << "A polygon vertex has an invalid Point json object!" << std::endl;
  }

  json iearr;
  try
  {
    iearr = j.at("edges");
  }
  catch (std::exception& /*e*/)
  {
    std::cerr << "A polygon vertex has an invalid Edges json object!" << std::endl;
    return;
  }
  // Deserialize the CCW-ordered list of incident edges:
  for (json::const_iterator er = iearr.begin(); er != iearr.end(); ++er)
  {
    bool haveEdge = false;
    bool haveFace = false;
    smtk::session::polygon::internal::Id edgeId;
    smtk::session::polygon::internal::Id faceId;
    bool edgeOut = false;
    try
    {
      std::string tmp = er->at("edge");
      edgeId = smtk::common::UUID(tmp);
      edgeOut = er->at("edgeout");
      haveEdge = true;
    }
    catch (std::exception&)
    {
    }
    try
    {
      std::string tmp = er->at("cwface");
      faceId = smtk::common::UUID(tmp);
      haveFace = true;
    }
    catch (std::exception&)
    {
    }
    vertex->dangerousAppendEdge(
      (haveEdge && haveFace
         ? smtk::session::polygon::internal::vertex::incident_edge_data(edgeId, edgeOut, faceId)
         : (haveEdge
              ? smtk::session::polygon::internal::vertex::incident_edge_data(edgeId, edgeOut)
              : (haveFace ? smtk::session::polygon::internal::vertex::incident_edge_data(faceId)
                          : (smtk::session::polygon::internal::vertex::incident_edge_data())))));
  }
}
} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk
