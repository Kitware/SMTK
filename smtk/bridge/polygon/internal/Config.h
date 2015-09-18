#ifndef __smtk_bridge_polygon_internal_bpConfig_h
#define __smtk_bridge_polygon_internal_bpConfig_h
#ifndef SHIBOKEN_SKIP

#include "smtk/SharedPtr.h"
#include "smtk/common/UUID.h"

#include "boost/polygon/polygon.hpp"

#include <list>
#include <map>
#include <set>

namespace smtk {
  namespace bridge {
    namespace polygon {
      namespace internal {

        class entity;
        class vertex;
        class edge;
        class face;
        class model;

        typedef smtk::shared_ptr<entity> EntityPtr;
        typedef smtk::shared_ptr<vertex> VertexPtr;
        typedef smtk::shared_ptr<edge> EdgePtr;
        typedef smtk::shared_ptr<face> FacePtr;

        typedef long long Coord;
        typedef smtk::common::UUID Id;
        typedef boost::polygon::point_data<Coord> Point;
        typedef boost::polygon::segment_data<Coord> Segment;
        typedef std::map<Point,Id> PointToVertexId;
        typedef std::map<Id,EntityPtr> EntityIdToPtr;
        typedef std::list<Point> PointSeq;
        typedef std::map<Point,VertexPtr> VertexById;

      } // namespace internal
    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_bridge_polygon_internal_bpConfig_h
