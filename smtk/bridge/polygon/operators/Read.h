//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_Read_h
#define __smtk_session_polygon_Read_h

#include "smtk/bridge/polygon/Operation.h"
#include "smtk/bridge/polygon/Resource.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Read an SMTK polygon model file.
  */
class SMTKPOLYGONSESSION_EXPORT Read : public Operation
{
public:
  smtkTypeMacro(Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};

SMTKPOLYGONSESSION_EXPORT smtk::resource::ResourcePtr read(const std::string&);

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Read_h
