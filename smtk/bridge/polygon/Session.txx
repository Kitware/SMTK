//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_Session_txx
#define smtk_session_polygon_Session_txx

#include "smtk/model/Manager.h"
#include "smtk/model/Manager.txx"

namespace smtk {
  namespace bridge {
    namespace polygon {

/** Consistently delete internal (and only polygon-session-specific) records related to the entities listed in \a container.
  *
  * This will call removeStorage() on each entity but additionally performs
  * cleanup on other entities referencing this entity.
  * For example, given a face, it will remove the face from all vertex records.
  * (Also, for faces, note that there is no internal face storage.)
  * Given an edge, it removes the corresponding incident-edge records from each endpoint.
  * Only lower-dimensional references are processed; this method assumes you have already
  * invoked consistentInternalDelete() on any parent entities.
  * Finally, the SMTK model manager (assumed to be the same for all entities in \a container)
  * is told to delete/reconcile records related to the entities.
  *
  * This method is provided by the session instead of the
  * polygon's internal pmodel class because the container
  * may include cells from multiple models.
  */
template<typename T, typename U, typename V>
void Session::consistentInternalDelete(T& container, U& modified, V& expunged, bool logDebug)
{
  smtk::model::Manager::Ptr mgr;
  typename T::iterator it;
  for (it = container.begin(); it != container.end(); ++it)
    {
    if (!it->isCellEntity())
      {
      if (!it->isAuxiliaryGeometry())
        {
        smtkWarningMacro(this->log(), "Trying to delete polygon storage for " << it->name() << " (" << it->flagSummary() << ")");
        }
      if (!mgr)
        {
        mgr = it->manager();
        }
      continue;
      }
    if (!mgr)
      {
      mgr = it->manager();
      }
    switch (it->dimensionBits())
      {
    case smtk::model::DIMENSION_2: this->removeFaceReferences(*it); break;
    case smtk::model::DIMENSION_1: this->removeEdgeReferences(*it); break;
    case smtk::model::DIMENSION_0: this->removeVertReferences(*it); break;
      }
    }
  if (mgr)
    {
    mgr->deleteEntities(container, modified, expunged, logDebug);
    }
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // smtk_session_polygon_Session_txx
