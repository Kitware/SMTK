//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_AuxiliaryGeometryExtension_h
#define smtk_model_AuxiliaryGeometryExtension_h

#include "smtk/common/Extension.h"

#include <vector>

namespace smtk
{
namespace model
{

class AuxiliaryGeometry;

/**\brief A base class for extensions that provide methods to
  * tessellate or mesh auxiliary geometry entities.
  */
class SMTKCORE_EXPORT AuxiliaryGeometryExtension : public smtk::common::Extension
{
public:
  smtkTypeMacro(AuxiliaryGeometryExtension);
  smtkSuperclassMacro(smtk::common::Extension);
  smtkSharedFromThisMacro(smtk::common::Extension);
  virtual ~AuxiliaryGeometryExtension();

  /// Returns true (and sets \a bboxOut) if \a entity's tessellation can be created.
  virtual bool canHandleAuxiliaryGeometry(
    smtk::model::AuxiliaryGeometry& entity, std::vector<double>& bboxOut) = 0;

protected:
  AuxiliaryGeometryExtension();
};
}
}

#endif
