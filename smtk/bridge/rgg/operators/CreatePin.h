//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_CreatePin_h
#define __smtk_session_rgg_CreatePin_h

#include "smtk/bridge/rgg/Operator.h"

/**\brief a type to define auxgeoms used for pin
  */
#define SMTK_BRIDGE_RGG_PIN "_rgg_pin"
/**\brief a name for subpart
  */
#define SMTK_BRIDGE_RGG_PIN_SUBPART "_subpart_"
/**\brief a name for layer
  */
#define SMTK_BRIDGE_RGG_PIN_LAYER "_layer_"
/**\brief a name for materialLayer
  */
#define SMTK_BRIDGE_RGG_PIN_MATERIAL "_material"

namespace smtk
{
namespace bridge
{
namespace rgg
{
enum RGGType
{
  CYLINDER = 0,
  FRUSTUM,
  ANNULUS
};

/**\brief create a rgg pin
  * The nuclear pin is converted into an auxiliary geometry in smtk world. All
  * parameters are stored as properties. SubParts(Cylinder and Frustum) and
  * layers are converted into children auxiliary geometries.
  * SubParts: Line extension along height
  * Layers: Annular extension along radius
  *
  */
class SMTKRGGSESSION_EXPORT CreatePin : public Operator
{
public:
  smtkTypeMacro(CreatePin);
  smtkCreateMacro(CreatePin);
  smtkSharedFromThisMacro(Operator);
  smtkSuperclassMacro(Operator);
  smtkDeclareModelOperator();

  static void populatePin(smtk::model::Operator* op, smtk::model::AuxiliaryGeometry& pin,
    std::vector<smtk::model::EntityRef>& subAuxGeoms, bool isCreation = false);

protected:
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace rgg
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_rgg_CreatePin_h
