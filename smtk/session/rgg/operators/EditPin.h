//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_EditPin_h
#define __smtk_session_rgg_EditPin_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace session
{
namespace rgg
{

/**\brief Edit a rgg pin
  * The nuclear pin is converted into an auxiliary geometry in smtk world. All
  * parameters are stored as properties. SubParts(Cylinder and Frustum) and
  * layers are converted into children auxiliary geometries.
  * SubParts: Line extension along height
  * Layers: Annular extension along radius
  *
  */
class SMTKRGGSESSION_EXPORT EditPin : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::EditPin);
  smtkCreateMacro(EditPin);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
  smtk::model::EntityRefArray m_expunged;
  smtk::model::EntityRefArray m_modified;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_EditPin_h
