//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_polygon_Write_h
#define smtk_session_polygon_Write_h

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Write a CMB polygon model file.
  */
class SMTKPOLYGONSESSION_EXPORT Write : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

SMTKPOLYGONSESSION_EXPORT bool write(
  const smtk::resource::ResourcePtr&,
  const std::shared_ptr<smtk::common::Managers>&);

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_Write_h
