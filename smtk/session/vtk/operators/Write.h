//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_vtk_Write_h
#define smtk_session_vtk_Write_h

#include "smtk/session/vtk/Operation.h"
#include "smtk/session/vtk/Resource.h"

namespace smtk
{
namespace session
{
namespace vtk
{

/**\brief Write a CMB vtk model file.
  */
class SMTKVTKSESSION_EXPORT Write : public Operation
{
public:
  smtkTypeMacro(smtk::session::vtk::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKVTKSESSION_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_Write_h
