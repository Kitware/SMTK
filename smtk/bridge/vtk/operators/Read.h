//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_vtk_Read_h
#define __smtk_session_vtk_Read_h

#include "smtk/bridge/vtk/Operation.h"
#include "smtk/bridge/vtk/Resource.h"

namespace smtk
{
namespace bridge
{
namespace vtk
{

/**\brief Read an SMTK vtk model file.
  */
class SMTKVTKSESSION_EXPORT Read : public Operation
{
public:
  smtkTypeMacro(smtk::bridge::vtk::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

SMTKVTKSESSION_EXPORT smtk::resource::ResourcePtr read(const std::string&);

} // namespace vtk
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_vtk_Read_h
