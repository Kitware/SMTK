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

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace bridge
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
class SMTKRGGSESSION_EXPORT EditPin : public Operator
{
public:
  smtkTypeMacro(EditPin);
  smtkCreateMacro(EditPin);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
  smtk::model::EntityRefArray m_expunged;
  smtk::model::EntityRefArray m_modified;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_EditPin_h
