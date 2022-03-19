//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/CleanGeometry.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Model.h"

#include "smtk/session/polygon/Session.txx"
#include "smtk/session/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/session/polygon/CleanGeometry_xml.h"

namespace smtk
{
namespace session
{
namespace polygon
{

typedef std::vector<std::pair<size_t, internal::Segment>> SegmentSplitsT;

template<typename T>
smtk::model::Edge findEdgeFromSegmentId(size_t cur, T& lkup)
{
  typename T::iterator it = lkup.lower_bound(cur);
  if (it == lkup.end())
  {
    return smtk::model::Edge();
  }
  return it->second;
}

template<typename T>
smtk::model::Vertex findOrAddInputModelVertex(
  smtk::model::ResourcePtr resource,
  const internal::Point& splitPt,
  T& endpoints,
  internal::pmodel* mod,
  smtk::model::EntityRefArray& created)
{
  typename T::iterator eit = endpoints.find(splitPt);
  if (eit != endpoints.end())
  {
    typename T::mapped_type::iterator it;
    for (it = eit->second.begin(); it != eit->second.end(); ++it)
    {
      if (it->isVertex())
      {
        return *it;
      }
    }
  }
  // No model vertex at the given location... add one.
  // Do **not** use mod->findOrAddModelVertex, as it may pick
  // a pre-existing model vertex that we have **not** been
  // told to use. We've been told everything we are supposed
  // to clean, so we should only use model vertices provided as inputs.
  smtk::model::Vertex vv = mod->addModelVertex(resource, splitPt, /* isFreeCell */ false);
  created.push_back(vv);
  endpoints[splitPt].insert(vv);
  return vv;
}

template<typename T, typename U, typename V, typename W, typename X>
bool CleanGeometry::splitEdgeAsNeeded(
  const smtk::model::Edge& curEdge,
  internal::edge::Ptr storage,
  T& result,
  U& lkup,
  V& revlkup,
  W& reslkup,
  X& endpoints,
  smtk::model::EntityRefArray& created,
  smtk::model::EntityRefArray& modified,
  smtk::model::EntityRefArray& expunged)
{
  (void)lkup;
  (void)revlkup;

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(curEdge.component()->resource());

  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  const std::pair<size_t, size_t> range = reslkup[curEdge];

  // Find edge split points
  std::vector<internal::PointSeq::const_iterator> splitPts;
  std::vector<internal::vertex::Ptr> splitVerts;
  SegmentSplitsT::iterator sstart = result.begin() + range.first;
  SegmentSplitsT::iterator sstop = result.begin() + range.second;
  internal::PointSeq::iterator ept0 = storage->pointsBegin();
  internal::PointSeq::iterator ept1 = ept0;
  if (curEdge.vertices().empty())
  {
    // We don't know for sure that the start of a periodic edge
    // with no vertices is not also a split point.
    // Add it to endpoints just in case something else shows up
    // at the same location.
    endpoints[*ept0].insert(curEdge);
  }
  internal::PointSeq reshaped; // the reshaped points including splits.
  internal::PointSeq::iterator rit;
  ++ept1; // Advance to start of next segment (i.e., end of first segment)
  reshaped.push_back(*ept0);
  //double x0[3], x1[3], rp[3];
  rit = reshaped.begin();
  //mod->liftPoint(*rit, rp);
  //std::cout << "Start edge " << curEdge.name() << "   reshaped p0 " << rp[0] << " " << rp[1] << "\n";
  for (SegmentSplitsT::iterator it = sstart; it != sstop; ++it, ++ept0, ++ept1)
  {
    /*
    mod->liftPoint(it->second.low(), x0);
    mod->liftPoint(it->second.high(), x1);
    std::cout << "  Top of loop, it = " << it->first << " " << x0[0] << " " << x0[1] << "  " << x1[0] << " " << x1[1] << "\n";
    */
    // Determine whether segment is flipped by location of ept0/1.
    SegmentSplitsT::iterator tmp;
    if (*ept1 == it->second.low())
    { // Oops, segment has been flipped and all its intersection points. Reorder.
      tmp = it;
      size_t nn = 0;
      while (tmp != sstop && tmp->first == it->first)
      {
        internal::Segment tsg(tmp->second.high(), tmp->second.low());
        tmp->second = tsg; // Flip each segment's low/high ordering
        ++nn;
        ++tmp; // Advance to end of segment
      }
      --tmp; // Back up so we point to a segment to be swapped
      SegmentSplitsT::value_type swap;
      nn /=
        2; // This rounds down, just like we want; no need to swap the middle out for itself when nn is odd.
      SegmentSplitsT::iterator tmp2 = it;
      for (size_t ns = 0; ns < nn; ++ns, ++tmp2, --tmp)
      {
        swap = *tmp2;
        tmp2->second = tmp->second;
        tmp->second = swap.second;
      }
    }
    tmp = it;

    /*
    mod->liftPoint(it->second.low(), x0);
    mod->liftPoint(it->second.high(), x1);
    std::cout << "  Oriented, now it =" << it->first << " " << x0[0] << " " << x0[1] << "  " << x1[0] << " " << x1[1] << "\n";
    */
    // Now we know ept0 == it->low and we can advance until it->high == ept1
    while (tmp != sstop && tmp->first == it->first)
    {
      /*
      mod->liftPoint(tmp->second.low(), x0);
      mod->liftPoint(tmp->second.high(), x1);
      std::cout << "    tmp =           " << tmp->first << " " << x0[0] << " " << x0[1] << "  " << x1[0] << " " << x1[1] << "\n";
      */

      internal::Point splitPt = tmp->second.high();
      reshaped.push_back(splitPt);
      if (tmp != it)
      {
        splitPts.emplace_back(rit);
        /*
        mod->liftPoint(*rit, rp);
        std::cout << "      split at " << rp[0] << " " << rp[1] << "\n";
        */
        // Find or add a vertex here. If we've been told to use a particular pre-existing
        // model vertex, use it. Otherwise, do **not** use a pre-existing model vertex
        // (instead, create a new one).
        smtk::model::Vertex splitVert =
          findOrAddInputModelVertex(resource, *rit, endpoints, mod, created);
        splitVerts.push_back(resource->findStorage<internal::vertex>(splitVert.entity()));
      }
      ++rit;
      ++tmp;
    }
    --tmp;
    it = tmp;
  }

  /*
  for (rit = reshaped.begin(); rit != reshaped.end(); ++rit)
    {
    mod->liftPoint(*rit, rp);
    std::cout << "    " << rp[0] << " " << rp[1] << "\n";
    }
    */

  // Tweak the edge so that its point sequence includes all the intersection points
  if (!mod->tweakEdge(curEdge, reshaped, modified))
  {
    smtkOpDebug("Could not reshape " << curEdge.name() << " to include intersections.");
    return false;
  }

  // Now split the edge
  std::size_t precre = created.size();
  if (!splitPts.empty())
  {
    expunged.push_back(
      curEdge); // The source edge is going away, to be replaced by multiple pieces:

    if (!mod->splitModelEdgeAtModelVertices(
          resource, storage, splitVerts, splitPts, created, m_debugLevel))
    {
      smtkOpDebug(
        "Could not split " << curEdge.name() << " at " << splitPts.size()
                           << " intersection points.");
      return false;
    }
    for (smtk::model::EntityRefArray::iterator cit = created.begin() + precre; cit != created.end();
         ++cit)
    {
      smtk::model::Vertices verts = cit->as<smtk::model::Edge>().vertices();
      if (!verts.empty())
      {
        internal::vertex::Ptr vv;
        for (int vi = 0; vi < 2; ++vi)
        {
          vv = resource->findStorage<internal::vertex>(
            vi == 0 ? verts.begin()->entity() : verts.rbegin()->entity());
          internal::Point vpt = vv->point();
          endpoints[vpt].insert(*verts.begin());
          endpoints[vpt].insert(*cit);
        }
      }
    }
  }
  return true;
}

template<typename T>
void CleanGeometry::addVertex(const smtk::model::Vertex& vertex, T& pointMap)
{
  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(vertex.component()->resource());
  internal::vertex::Ptr storage = resource->findStorage<internal::vertex>(vertex.entity());
  if (!storage)
  {
    smtkErrorMacro(this->log(), "Input vertex " << vertex.name() << " has no storage.");
    return;
  }
  pointMap[storage->point()].insert(vertex);
}

template<typename T>
void CleanGeometry::addEdgePoints(const smtk::model::Edge& edge, T& pointMap)
{
  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(edge.component()->resource());
  internal::edge::Ptr storage = resource->findStorage<internal::edge>(edge.entity());
  if (!storage)
  {
    smtkErrorMacro(this->log(), "Input edge " << edge.name() << " has no storage.");
    return;
  }
  internal::PointSeq::const_iterator pit;
  for (pit = storage->pointsBegin(); pit != storage->pointsEnd(); ++pit)
  {
    pointMap[*pit].insert(edge);
  }
}

template<typename T>
void CleanGeometry::addEdgeEndpoints(const smtk::model::Edge& edge, T& pointMap)
{
  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(edge.component()->resource());
  smtk::model::Vertices verts = edge.vertices();
  for (auto vit = verts.begin(); vit != verts.end(); ++vit)
  {
    internal::vertex::Ptr storage = resource->findStorage<internal::vertex>(vit->entity());
    if (!storage)
    {
      smtkErrorMacro(
        this->log(), "Vertex " << vit->name() << " bounding " << edge.name() << " has no storage.");
      continue;
    }
    pointMap[storage->point()].insert(edge);
  }
}

bool needEdgeSplit(
  const smtk::model::EntityRefs& prev,
  const smtk::model::EntityRefs& next,
  bool& splitPrev,
  bool& splitNext)
{
  splitPrev = splitNext = false;
  smtk::model::EntityRefs diff;
  std::set_symmetric_difference(
    prev.begin(), prev.end(), next.begin(), next.end(), std::inserter(diff, diff.begin()));
  for (smtk::model::EntityRefs::const_iterator it = diff.begin(); it != diff.end(); ++it)
  {
    // Entities entering @ next require split there, entities @prev require a split there
    if (next.find(*it) != next.end())
    {
      splitNext = true;
    }
    else
    {
      splitPrev = true;
    }
  }
  return splitPrev || splitNext;
}

template<typename T, typename U, typename V>
void CleanGeometry::addDeferredSplits(
  const smtk::model::Edge& edge,
  T& actions,
  U& allpoints,
  U& endpoints,
  V& created)
{
  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(edge.component()->resource());
  internal::edge::Ptr storage = resource->findStorage<internal::edge>(edge.entity());
  if (!storage)
  {
    smtkErrorMacro(this->log(), "Input edge " << edge.name() << " has no storage.");
    return;
  }
  if (storage->pointsSize() < 2)
  {
    smtkWarningMacro(this->log(), "Input edge " << edge.name() << " has too few points.");
    return;
  }
  smtk::model::Vertices verts = edge.vertices();
  internal::PointSeq::const_iterator pit = storage->pointsBegin();
  smtk::model::EntityRefs eset(allpoints[*pit]);
  smtk::model::Vertex vins;
  typename T::value_type action;
  action.m_edge = edge;
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  bool alreadySplit = false;
  internal::PointSeq::const_iterator prevIt = pit;

  // Process edge interior, looking for changes to eset
  for (++pit; pit != storage->pointsEnd(); ++pit)
  {
    smtk::model::EntityRefs& curset(allpoints[*pit]);
    bool splitPrev, splitNext;
    if (needEdgeSplit(eset, curset, splitPrev, splitNext))
    {
      if (splitPrev && !alreadySplit)
      {
        vins = findOrAddInputModelVertex(resource, *prevIt, endpoints, mod, created);
        action.m_splitPts.push_back(prevIt);
        action.m_splitVerts.push_back(resource->findStorage<internal::vertex>(vins.entity()));
        allpoints[*prevIt].insert(vins);
      }
      if (splitNext)
      {
        vins = findOrAddInputModelVertex(resource, *pit, endpoints, mod, created);
        action.m_splitPts.push_back(pit);
        action.m_splitVerts.push_back(resource->findStorage<internal::vertex>(vins.entity()));
        allpoints[*pit].insert(vins);
      }
    }

    alreadySplit = splitNext;
    eset = curset;
    prevIt = pit;
  }
  if (!action.m_splitPts.empty())
  {
    //std::cout << "Split " << action.m_edge.name() << " at " << action.m_splitPts.size() << " points\n";
    actions.push_back(action);
  }
}

template<typename T, typename U, typename V, typename W>
bool CleanGeometry::applyDeferredSplit(T resource, U& action, V& allpoints, W& created)
{
  internal::edge::Ptr storage =
    resource->template findStorage<internal::edge>(action.m_edge.entity());
  if (!storage)
  {
    smtkErrorMacro(this->log(), "Input edge " << action.m_edge.name() << " has no storage.");
    return false;
  }
  internal::pmodel* mod = storage->parentAs<internal::pmodel>();
  internal::PointSeq::const_iterator pit;
  // Remove pre-existing model edge from allpts
  for (pit = storage->pointsBegin(); pit != storage->pointsEnd(); ++pit)
  {
    allpoints[*pit].erase(action.m_edge);
  }
  // Split the edge
  std::size_t crepre = created.size();
  if (!mod->splitModelEdgeAtModelVertices(
        resource, storage, action.m_splitVerts, action.m_splitPts, created, m_debugLevel))
  {
    smtkOpDebug(
      "Could not split " << action.m_edge.name() << " at " << action.m_splitPts.size()
                         << " intersection points.");
    return false;
  }
  for (typename W::const_iterator ncre = created.begin() + crepre; ncre != created.end(); ++ncre)
  {
    if (ncre->isEdge())
    {
      this->addEdgePoints(*ncre, allpoints);
    }
  }
  return true;
}

template<typename T, typename U>
bool checkOneDirection(T& p0, T& q0, const U& p1, const U& q1)
{
  T t0;
  U t1;
  for (t0 = p0, t1 = p1; t0 != q0 && t1 != q1; ++t0, ++t1)
  {
    if (*t0 != *t1)
    {
      return false;
    }
  }
  return !(t0 != q0 || t1 != q1);
  // point sequences are not of same length: false
  // point sequences match at every point: true
}

template<typename T>
bool checkBothDirections3(T& p0, T& pt, T& q0, T& p1, T& q1, bool& matchDir)
{
  T t0;
  T t1;
  bool match;

  // Check forward direction:
  match = true;
  for (t0 = pt, t1 = p1; match && t0 != q0 && t1 != q1; ++t0, ++t1)
  {
    match = (*t0 == *t1);
  }
  if (match && ((t1 != q1) == (t0 != q0)))
  {
    for (t0 = p0; match && t0 != pt && t1 != q1; ++t0, ++t1)
    {
      match = (*t0 == *t1);
    }
    if (t0 == pt)
    {
      matchDir = true;
      return true;
    }
  }

  // Check backward direction:
  match = true;
  int mm = 0;
  for (t0 = pt, t1 = p1; match && t0 != p0 && t1 != q1; --t0, ++t1, ++mm)
  {
    //std::cout << "  " << mm << " " << t0->x() << " " << t1->x() << "  " << t0->y() << " " << t1->y() << "\n";
    match = (*t0 == *t1);
  }
  //std::cout << "  wrap around to q0\n";
  if (match && t1 != q1 && t0 == p0 && *t0 == *t1)
  { // Keep going
    t0 = q0;
    for (--t0; match && t0 != pt && t1 != q1; --t0, ++t1, ++mm)
    {
      //std::cout << "  " << mm << " " << t0->x() << " " << t1->x() << "  " << t0->y() << " " << t1->y() << "\n";
      match = (*t0 == *t1);
    }
    if (t1 != q1)
      ++t1; // Since we are periodic.
    if (t0 == pt && t1 == q1)
    {
      matchDir = false;
      return true;
    }
  }

  return false;
}

/// Given a std::pair<Edge, Edge>, merge properties and then delete the second if they are duplicates. Returns true if deleted.
template<typename T, typename U>
bool CleanGeometry::deleteIfDuplicates(T& edgePair, U& modified, U& expunged)
{
  if (!edgePair.first.isValid() || !edgePair.second.isValid())
  {
    return false;
  }

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(
      edgePair.first.component()->resource());

  smtk::model::Vertices verts;

  verts = edgePair.first.vertices();
  bool e0periodic = verts.size() < 2;
  //bool e0noEndpts = verts.empty();

  verts = edgePair.second.vertices();
  bool e1periodic = verts.size() < 2;
  //bool e1noEndpts = verts.empty();

  if (e0periodic != e1periodic)
  {
    return false;
  }

  internal::edge::Ptr e0 = resource->findStorage<internal::edge>(edgePair.first.entity());
  if (!e0)
  {
    return false;
  }

  internal::edge::Ptr e1 = resource->findStorage<internal::edge>(edgePair.second.entity());
  if (!e1)
  {
    return false;
  }

  if (e0->pointsSize() != e1->pointsSize())
  {
    return false;
  }

  //std::cout << "Comparing " << edgePair.first.name() << " and " << edgePair.second.name() << "\n";
  // If edges are periodic with no vertices, then we must find the
  // common point and then look for the proper direction.
  // If edges are not periodic, then we only need to check the
  // endpoints to find the match.
  internal::PointSeq::const_iterator p0 = e0->pointsBegin();
  internal::PointSeq::const_iterator p1 = e1->pointsBegin();
  internal::PointSeq::const_iterator q0 = e0->pointsEnd();
  internal::PointSeq::const_iterator q1 = e1->pointsEnd();
  internal::PointSeq::const_iterator pt;
  smtk::model::EdgeSet delset;
  if (!e0periodic)
  { // Neither edge is periodic, since we checked above.
    /*
    double xp0[3], xp1[3], xq0[3], xq1[3];
    internal::pmodel* mod = e0->parentAs<internal::pmodel>();
    mod->liftPoint(*p0, xp0);
    --q0; mod->liftPoint(*q0, xq0); ++q0;
    mod->liftPoint(*p1, xp1);
    --q1; mod->liftPoint(*q1, xq1); ++q1;
    std::cout
      << "    e0 " << xp0[0] << " " << xp0[1] << " -- " << xq0[0] << " " << xq0[1]
      << "    e1 " << xp1[0] << " " << xp1[1] << " -- " << xq1[0] << " " << xq1[1]
      << "\n";
      */

    if (*p0 == *p1)
    {
      if (checkOneDirection(p0, q0, p1, q1))
      {
        //std::cout << "Delete " << edgePair.second.name() << " (fwd)\n";
        delset.insert(edgePair.second);
      }
      else
      {
        //std::cout << "Do **NOT** delete " << edgePair.second.name() << " (fwd)\n";
      }
    }
    else if (*p0 == *(--q1))
    {
      if (checkOneDirection(p0, q0, e1->pointsRBegin(), e1->pointsREnd()))
      {
        //std::cout << "Delete " << edgePair.second.name() << " (bck)\n";
        delset.insert(edgePair.second);
      }
      else
      {
        //std::cout << "Do **NOT** delete " << edgePair.second.name() << " (bck)\n";
      }
    }
    // else the endpoints are model vertices and p0 of e0 had no match on e1.
  }
  else
  {
    // if (e0noEndpts && e1noEndpts)
    for (pt = p0; pt != q0; ++pt)
    {
      if (*pt == *p1)
      {
        bool matchDir;
        if (checkBothDirections3(p0, pt, q0, p1, q1, matchDir))
        {
          //std::cout << "Delete " << edgePair.second.name() << " (both)\n";
          delset.insert(edgePair.second);
          break;
        }
        else
        {
          //std::cout << "Do **NOT** delete " << edgePair.second.name() << " (both)\n";
        }
      }
    }
    // else: couldn't find a match in e1 to p0 of e0.
  }
  if (!delset.empty())
  {
    smtk::model::EntityRefs mergeset(delset.begin(), delset.end());
    smtk::model::EntityRef preserve(
      delset.find(edgePair.first) == delset.end() ? edgePair.first : edgePair.second);
    resource->polygonSession()->mergeProperties(mergeset, preserve);
    std::cout << "Deleting " << delset.begin()->name() << "...\n"
              << "  preserving " << preserve.name() << " pedigree ";
    model::IntegerList ped(preserve.integerProperty("pedigree id"));
    for (auto iit = ped.begin(); iit != ped.end(); ++iit)
    {
      std::cout << " " << *iit;
    }
    std::cout << "\n";
    // FIXME: Handle face attached to duplicate edge...
    resource->polygonSession()->consistentInternalDelete(
      delset, modified, expunged, m_debugLevel > 0);
    return true;
  }
  return false;
}

/**\brief Clean model geometry.
  *
  * ## Algorithm
  *
  * + Create segments for all edges, plus a map of offsets per pre-existing model edge.
  * + Call boost poly's intersect_segments, then "split edge" for all segments reporting multiple outputs.
  *      + For each (input or split) edge, add to map from endpoint *location* (not vert) to edge (both endpoints, if any).
  *      + At end, loop over map:
  *          + Remove duplicate edges between each pair of locations
  *            (afterwards, no coincident segments should exist).
  *              + If duplicate edges exist and faces are attached, error out for now
  *                (eventually migrate to remaining edge if possible)
  *          + Merge duplicate model vertices. Fail if pre-existing faces overlap.
  *              + This must fix any vertex uses/vertices referred to by edges,
  *                delete old vertices, and mark edges/uses modified.
  */
CleanGeometry::Result CleanGeometry::operateInternal()
{
  auto cleanItems = this->parameters()->associations();
  auto inputs = cleanItems->as<smtk::model::CellSet>([](smtk::resource::PersistentObjectPtr obj) {
    return smtk::model::CellEntity(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(
      cleanItems->valueAs<smtk::model::Entity>()->resource());
  if (!resource)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  smtk::model::CellSet::iterator iit;
  smtk::model::CellSet::iterator tmp;
  // Remove faces
  for (iit = inputs.begin(); iit != inputs.end(); /* do nothing */)
  {
    tmp = iit;
    ++tmp;
    if (iit->isFace())
    {
      inputs.erase(iit); // Invalidates iit.
      smtkWarningMacro(this->log(), "Faces are not allowed yet. Skipping " << iit->name());
    }
    iit = tmp;
  }

  if (inputs.empty())
  {
    smtkErrorMacro(this->log(), "No valid input cells.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::map<internal::Point, std::set<smtk::model::EntityRef>> endpoints;
  std::list<internal::Segment> segs;
  internal::pmodel* pp = nullptr;
  internal::pmodel* mod = nullptr;
  std::map<size_t, smtk::model::Edge> lkup;
  std::map<smtk::model::Edge, std::pair<size_t, size_t>> revlkup;
  // I. Prepare to intersect
  // Prepare lookup and reverse-lookup tables for the inputs.
  // Prepare an array of segments to intersect for all model edges in the input.
  // Start populating the endpoints table with model vertices in the input.
  // Validate the inputs are all polygon-session entities from the same model.
  for (iit = inputs.begin(); iit != inputs.end(); ++iit)
  {
    if (iit->isEdge())
    {
      internal::edge::Ptr storage = resource->findStorage<internal::edge>(iit->entity());
      if (!storage)
      {
        smtkErrorMacro(this->log(), "Input edge " << iit->name() << " has no storage.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      mod = storage->parentAs<internal::pmodel>();
      if (!mod || (pp && pp != mod))
      {
        smtkErrorMacro(
          this->log(),
          "Input edge " << iit->name() << " has no parent model (" << mod
                        << ") or a different parent (" << pp << ") from other inputs.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      else if (!pp)
      {
        pp = mod;
      }
      internal::PointSeq& epts(storage->points());
      internal::PointSeq::iterator epit = epts.begin();
      internal::Point p0 = *epit;
      internal::Point p1;
      size_t sstart = static_cast<size_t>(segs.size());
      for (++epit; epit != epts.end(); ++epit, p0 = p1)
      {
        p1 = *epit;
        segs.emplace_back(p0, p1);
      }
      size_t sstop = static_cast<size_t>(segs.size());
      revlkup[*iit] = std::pair<size_t, size_t>(sstart, sstop);
      lkup[sstop - 1] = *iit;
    }
    else if (iit->isVertex())
    {
      // Add model vertices to our map of endpoints.
      // This way if there are several coincident model vertices we can
      // suggest one we would prefer to be used for edges being split
      // (as well as which vertices should be merged).
      internal::vertex::Ptr vv = resource->findStorage<internal::vertex>(iit->entity());
      if (!vv)
      {
        smtkErrorMacro(this->log(), "Input vertex " << iit->name() << " has no storage.");
        return this->createResult(smtk::operation::Operation::Outcome::FAILED);
      }
      else
      {
        if (pp && ((mod = vv->parentAs<internal::pmodel>()) != pp))
        {
          smtkErrorMacro(
            this->log(),
            "Input vertex " << iit->name() << " has no parent model (" << mod
                            << ") or a different parent (" << pp << ") from other inputs.");
          return this->createResult(smtk::operation::Operation::Outcome::FAILED);
        }
        else if (!pp)
        {
          pp = mod;
        }
        endpoints[vv->point()].insert(*iit);
      }
    }
  }

  std::map<internal::Point, std::set<smtk::model::EntityRef>> allpoints;
  smtk::model::EntityRefArray created;
  smtk::model::EntityRefArray modified;
  smtk::model::EntityRefArray expunged;
  { // This block is here to limit the scope of "result"
    // II. Intersect all the segments.
    SegmentSplitsT result;
    intersect_segments(result, segs.begin(), segs.end());

    // III. Prepare a lookup table for the results as well
    std::map<smtk::model::Edge, std::pair<size_t, size_t>> reslkup;
    std::map<smtk::model::Edge, std::pair<size_t, size_t>>::iterator rlit = revlkup.begin();
    std::set<smtk::model::Edge> edgesToSplit;
    size_t last = 0;
    size_t cur = 0;
    for (SegmentSplitsT::iterator rit = result.begin(); rit != result.end(); last = cur, ++rlit)
    {
      size_t seglast = -1;
      bool needSplit = false;
      for (; rit != result.end() && rit->first < rlit->second.second; ++rit)
      {
        needSplit |= (rit->first == seglast); // 2+ segments with same source ID
        ++cur;
        seglast = rit->first;
      }
      reslkup[rlit->first] = std::pair<size_t, size_t>(last, cur);
      if (needSplit)
      {
        edgesToSplit.insert(rlit->first);
      }
      else
      {
        this->addEdgePoints(rlit->first, allpoints);
        this->addEdgeEndpoints(rlit->first, endpoints);
      }
    }

    // IV. Split edges
    smtkDebugMacro(
      this->log(), segs.size() << " segments in, " << result.size() << " segments out\n");
    for (std::set<smtk::model::Edge>::iterator esit = edgesToSplit.begin();
         esit != edgesToSplit.end();
         ++esit)
    {
      smtk::model::Edge curEdge = *esit;
      internal::edge::Ptr storage = resource->findStorage<internal::edge>(curEdge.entity());
      if (storage)
      {
        this->splitEdgeAsNeeded(
          curEdge, storage, result, lkup, revlkup, reslkup, endpoints, created, modified, expunged);
      }
    }
    for (auto citer = created.begin(); citer != created.end(); ++citer)
    {
      if (citer->isEdge())
      {
        this->addEdgePoints(*citer, allpoints);
        this->addEdgeEndpoints(*citer, endpoints);
      }
      else if (citer->isVertex())
      {
        this->addVertex(*citer, endpoints);
      }
    }
  }

  // V. Split edges as required to break partial overlaps.
  std::set<smtk::model::Edge> processed; // Keep track of edges we've already checked.
  struct DeferredSplit
  {
    smtk::model::Edge m_edge;
    std::vector<internal::PointSeq::const_iterator> m_splitPts;
    std::vector<internal::vertex::Ptr> m_splitVerts;
  };
  std::vector<DeferredSplit> deferredActions;
  for (auto piter = allpoints.begin(); piter != allpoints.end(); ++piter)
  {
    if (piter->second.size() > 1)
    { // multiple model entities exist at this point; we may need to split/delete edges or merge model vertices
      std::set<smtk::model::Vertex> coincidentVerts;
      for (auto eiter = piter->second.begin(); eiter != piter->second.end(); ++eiter)
      {
        if (eiter->isEdge() && processed.find(eiter->as<smtk::model::Edge>()) == processed.end())
        {
          this->addDeferredSplits(*eiter, deferredActions, allpoints, endpoints, created);
          processed.insert(*eiter);
        }
        else if (eiter->isVertex())
        {
          coincidentVerts.insert(*eiter);
        }
      }
      if (coincidentVerts.size() > 1)
      {
        // FIXME: Merge vertices
      }
    }
  }
  processed.clear();

  for (auto action : deferredActions)
  {
    if (this->applyDeferredSplit(resource, action, allpoints, created))
    {
      expunged.push_back(action.m_edge);
    }
  }

  // VI. Remove overlapping model edges
  //     We do this by pairwise comparison of model edges that share endpoints.
  //     No model edges should overlap
  std::set<std::pair<smtk::model::Edge, smtk::model::Edge>> processedPairs;
  for (auto piter = allpoints.begin(); piter != allpoints.end(); ++piter)
  {
    if (piter->second.size() > 1)
    { // multiple model entities exist at this point; we may need to split/delete edges or merge model vertices
      for (auto e1iter = piter->second.begin(); e1iter != piter->second.end(); ++e1iter)
      {
        if (!e1iter->isEdge())
        {
          continue;
        }
        auto e2iter = e1iter;
        for (++e2iter; e2iter != piter->second.end(); ++e2iter)
        {
          if (!e2iter->isEdge())
          {
            continue;
          }
          std::pair<smtk::model::Edge, smtk::model::Edge> epr;
          if (*e1iter < *e2iter)
          {
            epr = std::pair<smtk::model::Edge, smtk::model::Edge>(*e1iter, *e2iter);
          }
          else
          {
            epr = std::pair<smtk::model::Edge, smtk::model::Edge>(*e2iter, *e1iter);
          }
          if (processedPairs.find(epr) != processedPairs.end())
          {
            continue;
          }
          //std::cout << "Consider " << epr.first.name() << " and " << epr.second.name() << "\n";
          processedPairs.insert(epr);
          if (this->deleteIfDuplicates(epr, modified, expunged))
          {
            expunged.push_back(epr.second);
            //this->removeEdgePoints(epr.second, allpoints); // we are iterating over allpoints... really change it now?
          }
        }
      }
    }
  }

  Result opResult;
  opResult = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  smtk::attribute::ComponentItem::Ptr createdItem = opResult->findComponent("created");
  for (auto& c : created)
  {
    createdItem->appendValue(c.component());
  }

  smtk::attribute::ComponentItem::Ptr modifiedItem = opResult->findComponent("modified");
  for (auto& m : modified)
  {
    modifiedItem->appendValue(m.component());
  }

  smtk::attribute::ComponentItem::Ptr expungedItem = opResult->findComponent("expunged");
  for (auto& e : expunged)
  {
    expungedItem->appendValue(e.component());
  }
  operation::MarkGeometry(resource).markResult(opResult);

  return opResult;
}

const char* CleanGeometry::xmlDescription() const
{
  return CleanGeometry_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
