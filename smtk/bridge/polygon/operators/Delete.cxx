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
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.txx"

#include "smtk/bridge/polygon/Delete_xml.h"

#define MAX_WARNINGS 50

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Either add dependent cells or log messages.
  *
  * If deleteDependents is true, then for each cell in \a entities, add
  * any dependents to \a verts, \a edges, or \a faces.
  * If deleteDependents is false, add a log message for the first
  * umpteen entries in \a entities that have dependents.
  * After umpteen, just print a count.
  *
  * Returns true when \a deleteDependents is true and false
  * when both \a deleteDependents is false **and** any cell
  * in \a entities had a dependent cell.
  */
template<typename U, typename V, typename W>
bool Delete::addDependents(const smtk::model::EntityRef& ent, bool deleteDependents, U& verts, V& edges, W& faces)
{
  bool ok = true;
  if (ent.isVertex())
    {
    smtk::model::Vertex vv = ent.as<smtk::model::Vertex>();
    verts.insert(vv);
    // Add edges
    smtk::model::Edges ev = vv.edges();
    if (!ev.empty())
      { // FIXME: Detect when edges in ev (*and* their dependents) are explicitly scheduled for deletion. Don't fail in that case.
      ++this->m_numInUse;
      smtk::model::Edges::iterator it;
      if (deleteDependents)
        {
        for (it = ev.begin(); it != ev.end(); ++it)
          {
          this->addDependents(*it, deleteDependents, verts, edges, faces);
          }
        }
      else
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
        }
      }
    }
  else if (ent.isEdge())
    {
    smtk::model::Edge ee = ent.as<smtk::model::Edge>();
    edges.insert(ee);
    // Add faces
    smtk::model::Faces fe = ee.faces();
    if (!fe.empty())
      { // FIXME: Detect when faces in fe (*and* their dependents) are explicitly scheduled for deletion. Don't fail in that case.
      ++this->m_numInUse;
      smtk::model::Faces::iterator it;
      if (deleteDependents)
        {
        for (it = fe.begin(); it != fe.end(); ++it)
          {
          this->addDependents(*it, deleteDependents, verts, edges, faces);
          }
        }
      else
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
    }
  else if (ent.isFace())
    {
    smtk::model::Face ff = ent.as<smtk::model::Face>();
    faces.insert(ff);
    // No volumes in polygonal models... we're done.
    }
  else
    {
    smtkWarningMacro(this->log(), "Cannot delete non-cell entity " << ent.name() << ". Skipping.");
    }
  return ok;
}

smtk::model::OperatorResult Delete::operateInternal()
{
  this->m_debugLevel = 100;

  smtk::attribute::VoidItem::Ptr deleteDependentItem =
    this->findVoid("delete dependents");
  bool deleteDependents = deleteDependentItem->isEnabled();

  smtkOpDebug("associations: " << this->specification()->associations()->numberOfValues());
  smtk::model::EntityRefs entities = this->associatedEntitiesAs<smtk::model::EntityRefs>();
  smtk::model::VertexSet verts = this->associatedEntitiesAs<smtk::model::VertexSet>();
  smtk::model::EdgeSet edges = this->associatedEntitiesAs<smtk::model::EdgeSet>();
  smtk::model::FaceSet faces = this->associatedEntitiesAs<smtk::model::FaceSet>();

  smtk::model::Manager::Ptr mgr = this->session()->manager();

  this->m_numInUse = 0;
  this->m_numWarnings = 0;
  bool ok = true;
  smtk::model::EntityRefs::iterator eit;
  for (eit = entities.begin(); eit != entities.end(); ++eit)
    {
    ok &= this->addDependents(*eit, deleteDependents, verts, edges, faces);
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
    << edges.size() << " edges, and "
    << verts.size() << " verts"
    << (deleteDependents ? " (including dependents)." : "."));

  this->polygonSession()->consistentInternalDelete(faces, this->m_modified, this->m_expunged, this->m_debugLevel > 0);
  this->polygonSession()->consistentInternalDelete(edges, this->m_modified, this->m_expunged, this->m_debugLevel > 0);
  this->polygonSession()->consistentInternalDelete(verts, this->m_modified, this->m_expunged, this->m_debugLevel > 0);

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
