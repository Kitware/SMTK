//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_vtk_Export_h
#define __smtk_session_vtk_Export_h

#include "smtk/session/vtk/Operation.h"

namespace smtk
{
namespace session
{
namespace vtk
{

class SMTKVTKSESSION_EXPORT Export : public Operation
{
public:
  smtkTypeMacro(smtk::session::vtk::Export);
  smtkCreateMacro(Export);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  virtual Result exportExodus();
  virtual Result exportSLAC();
  virtual Result exportLabelMap();

  const char* xmlDescription() const override;
};

SMTKVTKSESSION_EXPORT bool exportResource(const smtk::resource::ResourcePtr&);

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // __smtk_session_vtk_Export_h
