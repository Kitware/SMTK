//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_discrete_ArrangementHelper_h
#define __smtk_session_discrete_ArrangementHelper_h

#include "smtk/SharedFromThis.h" // for smtkTypeMacro
#include "smtk/model/ArrangementHelper.h"
#include "smtk/session/discrete/Exports.h" // for SMTKDISCRETESESSION_EXPORT

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/EntityRef.h"

#include <set>

class vtkModelEdge;
class vtkModelEdgeUse;
class vtkModelRegion;

namespace smtk
{
namespace session
{
namespace discrete
{

/**\brief Superclass for session-specific updates to arrangments of entities.
  *
  * Subclasses of this class are used by subclasses of the Session class
  * to store session-specific information used to update arrangment information
  * during transcription.
  */
class SMTKDISCRETESESSION_EXPORT ArrangementHelper : public smtk::model::ArrangementHelper
{
public:
  smtkTypeMacro(smtk::session::discrete::ArrangementHelper);
  smtkSuperclassMacro(smtk::model::ArrangementHelper);
  ArrangementHelper();
  virtual ~ArrangementHelper();

  void addArrangement(const smtk::model::EntityRef& parent, smtk::model::ArrangementKind k,
    const smtk::model::EntityRef& child);
  void addArrangement(const smtk::model::EntityRef& parent, smtk::model::ArrangementKind k,
    const smtk::model::EntityRef& child, int sense, smtk::model::Orientation orientation,
    int iter_pos = 0);
  void resetArrangements();

  void doneAddingEntities(
    smtk::model::SessionPtr baseSession, smtk::model::SessionInfoBits flags) override;

  // Start of discrete-session specific methods:
  int findOrAssignSense(vtkModelEdgeUse* eu1);

  smtk::common::UUID useForRegion(vtkModelRegion*);
  vtkModelRegion* regionFromUseId(const smtk::common::UUID&);

  smtk::common::UUID chainForEdgeUse(vtkModelEdgeUse*);
  vtkModelEdgeUse* edgeUseFromChainId(const smtk::common::UUID&);

protected:
  typedef std::map<vtkModelEdgeUse*, int> EdgeUseToSenseMap;
  typedef std::map<vtkModelEdge*, EdgeUseToSenseMap> EdgeToUseSenseMap;
  typedef std::map<vtkModelRegion*, smtk::common::UUID> VolumeToUseIdMap;
  typedef std::map<smtk::common::UUID, vtkModelRegion*> UseIdToVolumeMap;
  typedef std::map<vtkModelEdgeUse*, smtk::common::UUID> EdgeToChainIdMap;
  typedef std::map<smtk::common::UUID, vtkModelEdgeUse*> ChainIdToEdgeMap;

  struct Spec
  {
    smtk::model::EntityRef parent;
    smtk::model::EntityRef child;
    smtk::model::ArrangementKind kind;
    int sense;
    mutable int iter_pos;

    Spec(const smtk::model::EntityRef& p, const smtk::model::EntityRef& c,
      smtk::model::ArrangementKind k, int s, int ip)
      : parent(p)
      , child(c)
      , kind(k)
      , sense(s)
      , iter_pos(ip)
    {
    }

    bool operator<(const Spec& other) const
    {
      return (this->kind < other.kind ||
               (this->kind == other.kind &&
                 (this->parent < other.parent ||
                   (this->parent == other.parent &&
                     (this->child < other.child ||
                       (this->child == other.child && this->sense < other.sense))))))
        ? true
        : false;
    }
  };

  std::set<Spec> m_arrangements;
  EdgeToUseSenseMap m_edgeUseSenses;
  VolumeToUseIdMap m_regionIds;
  UseIdToVolumeMap m_regions;
  EdgeToChainIdMap m_chainIds;
  ChainIdToEdgeMap m_chains;

private:
  ArrangementHelper(const ArrangementHelper& other); // Not implemented.
  void operator=(const ArrangementHelper& other);    // Not implemented.
};

} // namespace discrete
} // namespace session
} // namespace smtk

#endif // __smtk_session_discrete_ArrangementHelper_h
