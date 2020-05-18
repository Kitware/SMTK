//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_oscillator_EditSource_h
#define smtk_session_oscillator_EditSource_h

#include "smtk/session/oscillator/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

/**\brief Construct a 2- or 3-dimensional Gaussian source.
  */
class SMTKOSCILLATORSESSION_EXPORT EditSource : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::oscillator::EditSource);
  smtkCreateMacro(EditSource);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  /**\brief Change item parameters to match associated source (if any).
    *
    * When \a changedItem is the ReferenceItem that serves as the
    * operation's associations and it contains a single "source" (i.e.,
    * a 2-d auxiliary geometry representing an oscillator source),
    * update the position and radius of the operation to match.
    */
  bool configure(
    const smtk::attribute::AttributePtr& changedAttribute,
    const smtk::attribute::ItemPtr& changedItem) override;

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;

  void assignName(smtk::model::Model& model, smtk::model::AuxiliaryGeometry& source);
};
} // namespace oscillator
} // namespace session
} // namespace smtk

#endif
