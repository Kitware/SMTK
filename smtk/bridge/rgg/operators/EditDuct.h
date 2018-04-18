//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_EditDuct_h
#define __smtk_session_rgg_EditDuct_h

#include "smtk/bridge/rgg/Operator.h"

namespace smtk
{
namespace model
{
class AuxiliaryGeometry;
class EntityRef;
}
}

namespace smtk
{
namespace bridge
{
namespace rgg
{

/**\brief Edit a rgg duct
  * The nuclear duct is converted into an auxiliary geometry in smtk world. All
  * parameters are stored as properties. Segments and
  * layers are converted into children auxiliary geometries.
  * Segments: Line extension along height
  * Layers: Annular extension along radius/ width&length
  *
  */
class SMTKRGGSESSION_EXPORT EditDuct : public Operator
{
public:
  smtkTypeMacro(EditDuct);
  smtkCreateMacro(EditDuct);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_EditDuct_h
