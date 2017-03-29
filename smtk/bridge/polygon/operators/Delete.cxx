//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/Delete.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/Session.txx"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/ShellEntity.txx"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Manager.txx"

#include "smtk/bridge/polygon/Delete_xml.h"

#define MAX_WARNINGS 50

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Either add boundings cells or log messages.
  *
  * If deleteBoundingCells is true, then for each cell in \a entities, add
  * any dependents to \a verts, \a edges, or \a auxiliary geometry.
  * If deleteBoundingCells is false, add a log message for the first
  * umpteen entries in \a entities that have dependents.
  * After umpteen, just print a count.
  *
  * Returns true when \a deleteBoundingCells is true and false
  * when both \a deleteBoundingCells is false **and** any cell
  * in \a entities had a bounding cell.
  */
template<typename U, typename V, typename W, typename X>
bool Delete::checkAndAddBoundingCells(const smtk::model::EntityRef& ent, bool deleteBoundingCells, U& verts, V& edges, W& faces, X& other)
{
  bool ok = true;
  if (ent.isVertex())
  {
    smtk::model::Vertex vv = ent.as<smtk::model::Vertex>();
    // Add edges
    smtk::model::Edges ev = vv.edges();
    if (!ev.empty())
    {
      ++this->m_numInUse;
      smtk::model::Edges::iterator it;
      if (deleteBoundingCells)
      {
        verts.insert(vv);
        for (it = ev.begin(); it != ev.end(); ++it)
        {
          this->checkAndAddBoundingCells(*it, deleteBoundingCells, verts, edges, faces, other);
        }
      }
      else
      {
        // Detect when edges in ev (*and* their dependents) are explicitly
        // scheduled for deletion
        for (const auto &edge:ev)
        {
          if (edges.find(edge) == edges.end())
          {
          ok = false;
          if (this->m_numWarnings < MAX_WARNINGS)
          {
            std::ostringstream msg;
            msg << "Vertex " << vv.name() << " is used by";
            if (ev.size() > 5)
            {
              msg << " " << ev.size() << " cells.";
            }
            else
            {
              msg << ":\n";
              for (it = ev.begin(); it != ev.end(); ++it)
              {
                msg << "  " << it->name() << "\n";
              }
            }
            smtkErrorMacro(this->log(), msg.str());
          }
          ++this->m_numWarnings;
            break;
          }
        }
        if (ok)
          {verts.insert(vv);}
      }
    }
    else
    {
      verts.insert(vv);
    }
  }
  else if (ent.isEdge())
  {
    smtk::model::Edge ee = ent.as<smtk::model::Edge>();
    // Add faces
    smtk::model::Faces fe = ee.faces();
    if (!fe.empty())
    {
      ++this->m_numInUse;
      smtk::model::Faces::iterator it;
      if (deleteBoundingCells)
      {
        edges.insert(ee);
        for (it = fe.begin(); it != fe.end(); ++it)
        {
          this->checkAndAddBoundingCells(*it, deleteBoundingCells, verts, edges, faces, other);
        }
      }
      else
      {
        // Detect when faces in fe (*and* their dependents) are explicitly
        // scheduled for deletion
        for (const auto& face:fe)
        {
          if(faces.find(face) == faces.end())
          {
            ok = false;
            if (this->m_numWarnings < MAX_WARNINGS)
            {
              std::ostringstream msg;
              msg << "Edge " << ee.name() << " is used by ";
              if (fe.size() > 5)
              {
                msg << fe.size() << " cells.";
              }
              else
              {
                for (it = fe.begin(); it != fe.end(); ++it)
                {
                  msg << "  " << it->name() << "\n";
                }
              }
              smtkErrorMacro(this->log(), msg.str());
            }
            ++this->m_numWarnings;
          }
        }
        if (ok)
          {edges.insert(ee);}
      }
    }
    else
    {
      edges.insert(ee);
    }
  }
  else if (ent.isFace())
  {
    smtk::model::Face ff = ent.as<smtk::model::Face>();
    faces.insert(ff);
    // No volumes in polygonal models... we're done.
  }
  else if (ent.isAuxiliaryGeometry())
  {
    smtk::model::AuxiliaryGeometries children =
        ent.as<smtk::model::AuxiliaryGeometry>().embeddedEntities<smtk::model::AuxiliaryGeometries>();
    if (!children.empty())
    {
      ++this->m_numInUse;
      smtk::model::AuxiliaryGeometries::iterator it;
      if (deleteBoundingCells)
      {
        other.insert(ent);
        for (it = children.begin(); it != children.end(); ++it)
        {
          this->checkAndAddBoundingCells(*it, deleteBoundingCells, verts, edges, faces, other);
        }
      }
      else
      {
        ok = false;
        if (this->m_numWarnings < MAX_WARNINGS)
        {
          std::ostringstream msg;
          msg << "Auxiliary geometry " << ent.name() << " is used by ";
          if (children.size() > 5)
          {
            msg << children.size() << " cells.";
          }
          else
          {
            for (it = children.begin(); it != children.end(); ++it)
            {
              msg << "  " << it->name() << "\n";
            }
          }
          smtkErrorMacro(this->log(), msg.str());
        }
        ++this->m_numWarnings;
      }
    }
    else
    {
      other.insert(ent);
    }
  }
  else
  {
    smtkWarningMacro(this->log(), "Cannot delete non-cell entity " << ent.name() << ". Skipping.");
  }
  return ok;
}

/**\brief given an entity, add corresponding boundary cells
 * For each given entity, if its lower-dimensional entity is not associated by
 * any remaining entities, then this lower-dimensional entity would be deleted
 */
  template<typename U, typename V, typename W, typename X>
void Delete::addBoundaryCells(const smtk::model::EntityRef& ent, U& verts, V&
                              edges, W& faces, X& /*other*/)
{
  if (ent.isFace())
  {
    smtk::model::Face ff = ent.as<smtk::model::Face>();
    smtk::model::Edges ef = ff.edges();
    for (auto&& edgeEf:ef)
    {
      // loop over current edge's faces. If the edge is used by a face that
      // would not be deleted, do nothing. Else add edge to edges
      smtk::model::Faces facesEef = edgeEf.faces();
      bool addEdge(true);
      for (const auto& faceFeef : facesEef)
      {
        if (faces.find(faceFeef) == faces.end())
        {
          addEdge = false;
          break;
        }
      }
      if(addEdge)
        {edges.insert(edgeEf);}
    }
  }
  else if (ent.isEdge())
  {
    smtk::model::Edge ee = ent.as<smtk::model::Edge>();
    smtk::model::Vertices ve = ee.vertices();
    // same idea to the face case
    for (auto && vertexVe:ve)
    {
      smtk::model::Edges edgesVve = vertexVe.edges();
      bool addVertex(true);
      for (const auto& edgeEvve: edgesVve)
      {
        if (edges.find(edgeEvve) == edges.end())
        {
          addVertex = false;
          break;
        }
      }
      if (addVertex)
        {verts.insert(vertexVe);}
    }
  }
  else if (ent.isVertex())
  {
    //base case we are done
    smtk::model::Vertex vv = ent.as<smtk::model::Vertex>();
    verts.insert(vv);
  }
}

smtk::model::OperatorResult Delete::operateInternal()
{
  this->m_debugLevel = 100;

  smtk::attribute::VoidItem::Ptr deleteHigherDimen =
      this->findVoid("delete higher-dimensional neighbors");
  smtk::attribute::VoidItem::Ptr deleteLowerDimen =
      this->findVoid("delete lower-dimensional neighbors");
  bool deleteBoundingCells = deleteHigherDimen->isEnabled();
  bool deleteBoundaryCells = deleteLowerDimen->isEnabled();

  smtkOpDebug("associations: " << this->specification()->associations()->numberOfValues());

  smtk::model::EntityRefArray entities,oArray,fArray,eArray,vArray;

  // sort the input entities and go from high to low dimension
  smtk::model::EntityRefs entitySet = this->associatedEntitiesAs<smtk::model::EntityRefs>();
  smtk::model::EntityRefs::iterator eSit;
  for (eSit = entitySet.begin(); eSit != entitySet.end(); ++eSit)
  {
    if (eSit->isVertex())
    {
      vArray.push_back((*eSit).as<smtk::model::Vertex>());
    }
    else if (eSit->isEdge())
    {
      eArray.push_back((*eSit).as<smtk::model::Edge>());
    }
    else if (eSit->isFace())
    {
      fArray.push_back((*eSit).as<smtk::model::Face>());
    }
    else if  (eSit->isAuxiliaryGeometry())
    {
      oArray.push_back((*eSit).as<smtk::model::AuxiliaryGeometry>());
    }
    else
    {
      smtkWarningMacro(this->log(), "Cannot delete non-cell entity " << (*eSit).name() << ". Skipping.");
    }
  }
  entities.reserve(vArray.size() + eArray.size() + fArray.size() + oArray.size());
  std::move(oArray.begin(), oArray.end(), std::back_inserter(entities));
  std::move(fArray.begin(), fArray.end(), std::back_inserter(entities));
  std::move(eArray.begin(), eArray.end(), std::back_inserter(entities));
  std::move(vArray.begin(), vArray.end(), std::back_inserter(entities));

  // cells to be deleted
  smtk::model::VertexSet verts;
  smtk::model::EdgeSet edges;
  smtk::model::FaceSet faces;
  smtk::model::EntityRefs other;

  this->m_numInUse = 0;
  this->m_numWarnings = 0;
  bool ok = false;
  smtk::model::EntityRefArray::iterator eit;

  // check bounding cells option
  for (eit = entities.begin(); eit != entities.end(); ++eit)
  {
    ok |= this->checkAndAddBoundingCells(*eit, deleteBoundingCells, verts, edges, faces, other);
  }

  // check boundary cells
  if (ok && deleteBoundaryCells)
  {
    if (!deleteBoundingCells)
    {
      // go over valid cells to be deleted
      for (auto && face:faces)
      {
        smtk::model::EntityRef entity = face.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, verts, edges, faces, other);
      }
      for (auto && edge:edges)
      {
        smtk::model::EntityRef entity = edge.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, verts, edges, faces, other);
      }
      for (auto && vertex:verts)
      {
        smtk::model::EntityRef entity = vertex.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, verts, edges, faces, other);
      }
    }
    else
    {
      // go over the entities provided by user
      smtk::model::VertexSet vertsAsso = this->
          associatedEntitiesAs<smtk::model::VertexSet>();
      smtk::model::EdgeSet edgesAsso = this->
          associatedEntitiesAs<smtk::model::EdgeSet>();
      smtk::model::FaceSet facesAsso = this->
          associatedEntitiesAs<smtk::model::FaceSet>();
      for (auto && face:facesAsso)
      {
        smtk::model::EntityRef entity = face.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, vertsAsso, edgesAsso, facesAsso, other);
      }
      for (auto && edge:edgesAsso)
      {
        smtk::model::EntityRef entity = edge.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, vertsAsso, edgesAsso, facesAsso, other);
      }
      for (auto && vertex:vertsAsso)
      {
        smtk::model::EntityRef entity = vertex.as<smtk::model::EntityRef>();
        this->addBoundaryCells(entity, vertsAsso, edgesAsso, facesAsso, other);
      }
      // combine entities into cells to be deleted
      faces.insert(facesAsso.begin(), facesAsso.end());
      edges.insert(edgesAsso.begin(), edgesAsso.end());
      verts.insert(vertsAsso.begin(), vertsAsso.end());
    }
  }


  if (!ok)
  {
    if (this->m_numWarnings > MAX_WARNINGS)
    {
      smtkErrorMacro(this->log(),
                     "... and " << (this->m_numWarnings - MAX_WARNINGS) << " more entities with dependents.");
    }
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  smtkOpDebug("Given "
              << entities.size() << ", found "
              << faces.size() << " faces, "
              << edges.size() << " edges, "
              << verts.size() << " verts, and "
              << other.size() << " others "
              << (deleteBoundingCells ? " (including bounding cells) " : " ")
              << (deleteBoundaryCells ? " (including boundary cells)." : "."));

  this->polygonSession()->consistentInternalDelete(faces, this->m_modified, this->m_expunged, this->m_debugLevel > 0);
  this->polygonSession()->consistentInternalDelete(edges, this->m_modified, this->m_expunged, this->m_debugLevel > 0);
  this->polygonSession()->consistentInternalDelete(verts, this->m_modified, this->m_expunged, this->m_debugLevel > 0);
  this->polygonSession()->consistentInternalDelete(other, this->m_modified, this->m_expunged, this->m_debugLevel > 0);

  smtk::model::OperatorResult result =
      this->createResult(smtk::model::OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, this->m_expunged, EXPUNGED);
  smtk::model::EntityRefArray modray(this->m_modified.begin(), this->m_modified.end());
  this->addEntitiesToResult(result, modray, MODIFIED);

  smtkInfoMacro(this->log(),
                "Deleted " << this->m_expunged.size() << " of " << entities.size() << " requested entities");

  return result;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
    SMTKPOLYGONSESSION_EXPORT,
    smtk::bridge::polygon::Delete,
    polygon_delete,
    "delete",
    Delete_xml,
    smtk::bridge::polygon::Session);
