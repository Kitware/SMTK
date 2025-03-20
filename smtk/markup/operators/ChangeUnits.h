//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_ChangeUnits_h
#define smtk_markup_ChangeUnits_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/**\brief Scale geometric data of associated objects from one length unit to another.
  *
  * If the destination unit is empty (the default) and the resource has a
  * unit system with an active context (that specifies default units for
  * each dimension), then the default length unit will be the destination.
  *
  * If the input association is spatial data, then the input nodes are
  * blanked and output versions are created with the transformed geometry.
  *
  * If the input association is a markup resource, this changes the
  * default length unit for the resource (and rescales all of its spatial
  * data components if the default length unit was different but valid;
  * no rescaling is performed if there was no prior default length unit).
  */
class SMTKMARKUP_EXPORT ChangeUnits : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::markup::ChangeUnits);
  smtkCreateMacro(ChangeUnits);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_ChangeUnits_h
