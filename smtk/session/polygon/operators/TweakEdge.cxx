//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/TweakEdge.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Face.h"
#include "smtk/model/Vertex.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/operators/TweakEdge_xml.h"

namespace smtk
{
namespace session
{
namespace polygon
{

TweakEdge::Result TweakEdge::operateInternal()
{
  auto edgeItem = this->parameters()->associations();
  auto pointsItem = this->parameters()->findDouble("points");
  auto coordinatesItem = this->parameters()->findInt("coordinates");
  auto promoteItem = this->parameters()->findInt("promote");

  smtk::model::Edge src(edgeItem->valueAs<smtk::model::Entity>());
  if (!src.isValid())
  {
    smtkErrorMacro(this->log(), "Input edge was invalid.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::session::polygon::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::polygon::Resource>(src.component()->resource());
  if (!resource)
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);

  internal::edge::Ptr storage = resource->findStorage<internal::edge>(src.entity());
  internal::pmodel* pmod = storage->parentAs<internal::pmodel>();
  if (!storage || !pmod)
  {
    smtkErrorMacro(
      this->log(), "Input edge was not part of its parent model (or not a polygon-session edge).");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  bool ok = true;
  std::set<int> splits(promoteItem->begin(), promoteItem->end());
  int numCoordsPerPt = coordinatesItem->value(0);
  std::size_t npts = pointsItem->numberOfValues() / numCoordsPerPt; // == #pts / #coordsPerPt
  if (npts < 2)
  {
    smtkErrorMacro(
      this->log(),
      "Not enough points to form an edge (" << pointsItem->numberOfValues() << " coordinates at "
                                            << numCoordsPerPt << " per point => " << npts
                                            << " points)");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  if (!splits.empty())
  {
    std::set<int>::iterator sit;
    std::ostringstream derp;
    bool bad = false;
    derp << "Ignoring invalid split-point indices: ";
    for (sit = splits.begin(); sit != splits.end() && *sit < 0; ++sit)
    {
      derp << " " << *sit;
      std::set<int>::iterator tmp = sit;
      ++sit;
      splits.erase(tmp);
      bad = true;
      --sit;
    }
    std::set<int>::reverse_iterator rsit;
    for (rsit = splits.rbegin(); rsit != splits.rend() && *rsit >= static_cast<int>(npts); ++rsit)
    {
      derp << " " << *rsit;
      std::set<int>::iterator tmp = --rsit.base();
      ++rsit;
      splits.erase(tmp);
      bad = true;
      --rsit;
    }
    if (bad)
    {
      smtkWarningMacro(this->log(), derp.str());
    }
  }

  smtk::model::EntityRefArray modified; // includes edge and perhaps eventually faces.
  smtk::model::Edges edgeCreated;
  smtk::model::Vertices verticesCreated;

  // Done checking input. Perform operation.
  ok &= pmod->tweakEdge(src, numCoordsPerPt, pointsItem->begin(), pointsItem->end(), modified);
  // Split the edge as requested by the user:
  std::vector<internal::vertex::Ptr> promotedVerts;
  std::vector<internal::PointSeq::const_iterator> splitLocs;
  internal::PointSeq& epts(storage->points());
  internal::PointSeq::const_iterator ptit = epts.begin();
  int last = 0;
  for (std::set<int>::iterator promit = splits.begin(); promit != splits.end(); ++promit)
  {
    std::advance(ptit, *promit - last);
    last = *promit;
    if (!!pmod->pointId(*ptit))
    {
      continue; // skip points that are already model vertices (should only happen at start/end)
    }
    smtk::model::Vertex pv = pmod->findOrAddModelVertex(resource, *ptit);
    verticesCreated.push_back(pv);

    promotedVerts.push_back(resource->findStorage<internal::vertex>(pv.entity()));
    splitLocs.push_back(ptit);
    std::cout << "  " << ptit->x() << " " << ptit->y() << "\n";
  }
  smtk::model::EntityRefArray edgesAdded;
  smtk::model::EntityRefArray expunged;
  if (!splitLocs.empty())
  {
    smtkInfoMacro(this->log(), "Splitting tweaked edge at " << splitLocs.size() << " places.");
    if (!pmod->splitModelEdgeAtModelVertices(
          resource, storage, promotedVerts, splitLocs, edgesAdded, m_debugLevel))
    {
      smtkErrorMacro(this->log(), "Could not split edge.");
      ok = false;
    }
    if (!edgesAdded.empty())
    {
      expunged.push_back(src);
    }
    edgeCreated.insert(edgeCreated.end(), edgesAdded.begin(), edgesAdded.end());
  }

  if (m_debugLevel > 0)
  {
    for (smtk::model::Edges::iterator eCrit = edgeCreated.begin(); eCrit != edgeCreated.end();
         ++eCrit)
    {
      smtkOpDebug("Created " << eCrit->name() << ".");
    }
    for (auto vCrit = verticesCreated.begin(); vCrit != verticesCreated.end(); ++vCrit)
    {
      smtkOpDebug("Created " << vCrit->name() << ".");
    }

    for (smtk::model::EntityRefArray::iterator moit = modified.begin(); moit != modified.end();
         ++moit)
    {
      smtkOpDebug("Modified " << moit->name() << ".");
    }

    for (smtk::model::EntityRefArray::iterator epit = expunged.begin(); epit != expunged.end();
         ++epit)
    {
      smtkOpDebug("Expunged " << epit->name() << ".");
    }
  }

  Result opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

    smtk::attribute::ComponentItem::Ptr createdItem = opResult->findComponent("created");
    for (auto& c : verticesCreated)
    {
      createdItem->appendValue(c.component());
    }
    for (auto& c : edgeCreated)
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
  }
  else
  {
    opResult = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return opResult;
}

const char* TweakEdge::xmlDescription() const
{
  return TweakEdge_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
