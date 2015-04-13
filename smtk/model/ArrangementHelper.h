//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ArrangementHelper_h
#define __smtk_model_ArrangementHelper_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/SMTKCoreExports.h" // for SMTKCORE_EXPORT
#include "smtk/SharedFromThis.h" // for smtkTypeMacro

namespace smtk {
  namespace model {

/**\brief Superclass for session-specific updates to arrangments of entities.
  *
  * Subclasses of this class are used by subclasses of the Session class
  * to store session-specific information used to update arrangment information
  * during transcription.
  */
class SMTKCORE_EXPORT ArrangementHelper
{
public:
  smtkTypeMacro(ArrangementHelper);
  virtual ~ArrangementHelper();

  virtual void doneAddingEntities(SessionPtr sess);

  void mark(const EntityRef& ent, bool m);
  bool isMarked(const EntityRef& ent) const;
  void resetMarks();

  void reset(const EntityRef& ent);

protected:
  friend class Session;
  ArrangementHelper();

  EntityRefs m_marked;

private:
  ArrangementHelper(const ArrangementHelper& other); // Not implemented.
  void operator = (const ArrangementHelper& other); // Not implemented.
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ArrangementHelper_h
