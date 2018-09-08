//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_bpConfig_h
#define __smtk_session_polygon_internal_bpConfig_h

#include "smtk/SharedPtr.h"
#include "smtk/common/CompilerInformation.h"
#include "smtk/common/UUID.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/polygon/polygon.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <list>
#include <map>
#include <set>

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

class entity;
class vertex;
class edge;
class face;
class pmodel;

typedef smtk::shared_ptr<entity> EntityPtr;
typedef smtk::shared_ptr<vertex> VertexPtr;
typedef smtk::shared_ptr<edge> EdgePtr;
typedef smtk::shared_ptr<face> FacePtr;
typedef smtk::shared_ptr<pmodel> ModelPtr;

typedef long long Coord;
typedef boost::polygon::high_precision_type<Coord>::type HighPrecisionCoord;
typedef smtk::common::UUID Id;
typedef boost::polygon::point_data<Coord> Point;
typedef boost::polygon::point_data<HighPrecisionCoord> HighPrecisionPoint;
typedef boost::polygon::segment_data<Coord> Segment;
typedef boost::polygon::interval_data<Coord> Interval;
typedef boost::polygon::rectangle_data<Coord> Rect;
typedef std::map<Point, Id> PointToVertexId;
typedef std::map<Id, EntityPtr> EntityIdToPtr;
typedef std::list<Point> PointSeq;
typedef std::map<Point, VertexPtr> VertexById;

} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_internal_bpConfig_h
