//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_vtk_LegacyRead_h
#define __smtk_session_vtk_LegacyRead_h

#include "smtk/session/vtk/Operation.h"
#include "smtk/session/vtk/Resource.h"

class vtkIdTypeArray;
class vtkPolyData;

namespace smtk
{
namespace session
{
namespace vtk
{

/**\brief Read a legacy SMTK vtk model file.

   When moving from CJSON to nlohmann::json, the file format for vtk
   models changed slightly. This operation facilitates reading the old format
   using our new tools.
  */
class SMTKVTKSESSION_EXPORT LegacyRead : public Operation
{
public:
  smtkTypeMacro(smtk::session::vtk::LegacyRead);
  smtkCreateMacro(LegacyRead);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

SMTKVTKSESSION_EXPORT smtk::resource::ResourcePtr legacyRead(const std::string&);

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // __smtk_session_vtk_LegacyRead_h
