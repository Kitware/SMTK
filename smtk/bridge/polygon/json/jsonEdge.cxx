//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/json/jsonEdge.h"

#include "smtk/bridge/polygon/internal/Edge.h"

#include "smtk/model/json/jsonTessellation.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{
namespace internal
{
using json = nlohmann::json;
// Define how polygon resources are serialized.
void to_json(json& j, const smtk::bridge::polygon::internal::edge::Ptr& edge)
{
  j["type"] = "edge";

  if (edge->parent())
  {
    j["parent"] = edge->parent()->id().toString();
  }

  std::size_t np = edge->pointsSize();
  const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
  std::vector<long> ptdata(np * stride, 0);
  smtk::bridge::polygon::internal::PointSeq::const_iterator pit = edge->pointsBegin();
  for (std::size_t i = 0; i < np; ++i, ++pit)
  {
    smtk::bridge::polygon::internal::Coord c = pit->x();
    unsigned long long& uc(*reinterpret_cast<unsigned long long*>(&c));
    ptdata[i * stride + 0] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 1] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 2] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 3] = uc & 0xffff;

    c = pit->y();
    ptdata[i * stride + 4] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 5] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 6] = uc & 0xffff;
    uc >>= 16;
    ptdata[i * stride + 7] = uc & 0xffff;
  }
  j["points"] = ptdata;
}

void from_json(const json& j, smtk::bridge::polygon::internal::edge::Ptr& edge)
{
  edge = smtk::bridge::polygon::internal::edge::create();

  try
  {
    std::vector<long> ptdata = j.at("points");

    const int stride = 2 /* coords per pt */ * 4 /* ints per coord */;
    std::size_t np = ptdata.size() / stride;
    std::vector<long>::const_iterator cit = ptdata.begin();
    for (std::size_t i = 0; i < np; ++i)
    {
      smtk::bridge::polygon::internal::Coord xy[2] = { 0, 0 };
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
      edge->points().insert(
        edge->pointsEnd(), smtk::bridge::polygon::internal::Point(xy[0], xy[1]));
    }
  }
  catch (std::exception&)
  {
    std::cerr << "Polygon edge does not have a valid points object" << std::endl;
    return;
  }
}

} // namespace internal
} // namespace polygon
} // namespace bridge
} // namespace smtk
