//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_polygon_Read_h
#define smtk_session_polygon_Read_h

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Read an SMTK polygon model file.
  */
class SMTKPOLYGONSESSION_EXPORT Read : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  void markModifiedResources(Result& res) override;
  const char* xmlDescription() const override;
};

SMTKPOLYGONSESSION_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_Read_h
