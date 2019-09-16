//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_operators_DivideInstance_h
#define smtk_model_operators_DivideInstance_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace model
{

class SMTKCORE_EXPORT DivideInstance : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::DivideInstance);
  smtkCreateMacro(DivideInstance);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;

  /**\brief Return the parent instance of clones in the provided \a item.
    *
    * If there are multiple clones but they do not share a common
    * parent, return an invalid instance.
    * This method will log an error if returning an invalid instance.
    */
  smtk::model::Instance parentOfClones(const smtk::attribute::ReferenceItemPtr& item) const;
};

} // namespace model
} // namespace smtk

#endif // smtk_model_operators_DivideInstance_h
