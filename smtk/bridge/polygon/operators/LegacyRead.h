//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_LegacyRead_h
#define __smtk_session_polygon_LegacyRead_h

#include "smtk/bridge/polygon/Operation.h"
#include "smtk/bridge/polygon/Resource.h"

class vtkIdTypeArray;
class vtkPolyData;

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Read a legacy SMTK polygon model file.

   When moving from CJSON to nlohmann::json, the file format for polygon
   models changed slightly. This operation facilitates reading the old format
   using our new tools.
  */
class SMTKPOLYGONSESSION_EXPORT LegacyRead : public Operation
{
public:
  smtkTypeMacro(LegacyRead);
  smtkCreateMacro(LegacyRead);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_LegacyRead_h
