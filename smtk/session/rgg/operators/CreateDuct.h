//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_rgg_CreateDuct_h
#define __smtk_session_rgg_CreateDuct_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/operation/XMLOperation.h"

/**\brief a type to define auxgeoms used for duct
  */
#define SMTK_SESSION_RGG_DUCT "_rgg_duct"
/**\brief a type to define auxgeoms used for duct segment
  */
#define SMTK_SESSION_RGG_DUCT_SEGMENT "_segment_"
/**\brief a type to define auxgeoms used for duct layer
  */
#define SMTK_SESSION_RGG_DUCT_LAYER "_layer_"

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
namespace session
{
namespace rgg
{

/**\brief create a rgg duct
  * The nuclear duct is converted into an auxiliary geometry in smtk world. All
  * parameters are stored as properties. Segments and
  * layers are converted into children auxiliary geometries.
  * Segments: Line extension along height
  * Layers: Annular extension along radius
  *
  */
class SMTKRGGSESSION_EXPORT CreateDuct : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::rgg::CreateDuct);
  smtkCreateMacro(CreateDuct);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  static void populateDuct(smtk::operation::Operation* op, smtk::model::AuxiliaryGeometry& duct,
    std::vector<smtk::model::EntityRef>& subAuxgeoms);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_CreateDuct_h
