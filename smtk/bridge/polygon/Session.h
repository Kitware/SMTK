//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Session_h
#define __smtk_session_polygon_Session_h

#include "smtk/bridge/polygon/Exports.h"
#include "smtk/bridge/polygon/PointerDefs.h"
#include "smtk/model/Session.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Methods that handle translation between polygon and SMTK instances.
  *
  * While the TDUUID class keeps a map from SMTK UUIDs to polygon ToolDataUser
  * pointers, this is not enough to handle everything SMTK provides:
  * there is no way to track cell-use or shell entities since they do
  * not inherit ToolDataUser instances. Also, some engines (e.g., facet)
  * do not appear to store some entity types (e.g., RefGroup).
  *
  * Also, simply loading a polygon file does not translate the entire model
  * into SMTK; instead, it assigns UUIDs to entities if they do not already
  * exist. This class (Session) provides a method for requesting the
  * entity, arrangement, and/or tessellation information for a UUID be
  * mapped into SMTK from polygon.
  */
class SMTKPOLYGONSESSION_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  smtkDeclareModelingKernel();
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  virtual ~Session();

  virtual SessionInfoBits allSupportedInformation() const;

protected:
  Session();

  virtual smtk::model::SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo, int depth = -1);

private:
  Session(const Session&); // Not implemented.
  void operator = (const Session&); // Not implemented.
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Session_h
