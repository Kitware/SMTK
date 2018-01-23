//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_TerrainExtraction_h
#define __smtk_model_TerrainExtraction_h

#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT TerrainExtraction : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(TerrainExtraction);
  smtkCreateMacro(TerrainExtraction);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(smtk::operation::XMLOperator);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} //namespace model
} // namespace smtk

#endif // __smtk_model_TerrainExtraction_h
