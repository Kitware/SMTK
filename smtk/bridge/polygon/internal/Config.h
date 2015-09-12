#ifndef __smtk_bridge_polygon_internal_bpConfig_h
#define __smtk_bridge_polygon_internal_bpConfig_h

#include "smtk/SharedPtr.h"
#include "smtk/common/UUID.h"

#include "boost/polygon.hpp"

#include <list>
#include <map>
#include <set>

namespace smtk {
  namespace bridge {
    namespace polygon {

      class Entity;
      class Vertex;
      class Edge;
      class Face;

      typedef smtk::shared_ptr<Entity> EntityPtr;
      typedef smtk::shared_ptr<Vertex> VertexPtr;
      typedef smtk::shared_ptr<Edge> EdgePtr;
      typedef smtk::shared_ptr<Face> FacePtr;

      typedef long long Coord;
      typedef smtk::common::UUID Id;
      typedef std::pair<Coord> Point;
      typedef std::map<Point,Id> PointToVertexId;
      typedef std::map<Id,EntityPtr> EntityIdToPtr;
      typedef std::list<Point> PointSeq;
      typedef std::map<Point,VertexPtr> VertexById;

    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_bpConfig_h
