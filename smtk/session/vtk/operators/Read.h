//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_vtk_Read_h
#define smtk_session_vtk_Read_h

#include "smtk/session/vtk/Operation.h"
#include "smtk/session/vtk/Resource.h"

namespace smtk
{
namespace session
{
namespace vtk
{

/**\brief Read an SMTK vtk model file.
  */
class SMTKVTKSESSION_EXPORT Read : public Operation
{
public:
  smtkTypeMacro(smtk::session::vtk::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKVTKSESSION_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_Read_h
