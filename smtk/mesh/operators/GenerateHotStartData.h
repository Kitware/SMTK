//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_mesh_operators_GenerateHotStartData_h
#define __smtk_mesh_operators_GenerateHotStartData_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief A class for generating hot start data for AdH.

   This class performs the same actions as InterpolateOntoMesh, with the
   following differences:
   1) the API is tweaked to apply directly to AdH's use case
   2) only point data can be generated
   3) when computing initial water surface elevation, the z-coordinate of each
      point is subtracted from the interpolated data, resulting in a value that
      represents the water level above ground (as opposed to a baseline value)
  */
class SMTKCORE_EXPORT GenerateHotStartData : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(GenerateHotStartData);
  smtkCreateMacro(GenerateHotStartData);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
}
}

#endif // __smtk_mesh_operators_GenerateHotStartData_h
