//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_polygon_ImportPPG_h
#define smtk_session_polygon_ImportPPG_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/XMLOperation.h"
#include "smtk/session/polygon/Exports.h"

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Create a model resource from .ppg file input.
 *
 * SMTK supports a simple .ppg file format for specifying
 * 2-D geometry. The file is used to specify a set of
 * vertices as 2-D coordinates and a set of ploygon faces,
 * each specified as a list of vertex indices. Model edges
 * are internally created between adjacent vertices.
 * Only convex polygons are supported.
 *
  */
class SMTKPOLYGONSESSION_EXPORT ImportPPG : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::polygon::ImportPPG);
  smtkCreateMacro(ImportPPG);
  smtkSharedFromThisMacro(smtk::operation::XMLOperation);
  smtkSuperclassMacro(Operation);

  // Override ableToOperate() to support test mode
  bool ableToOperate() override;

  virtual ~ImportPPG();

protected:
  ImportPPG();
  smtk::operation::Operation::Result operateInternal() override;
  const char* xmlDescription() const override;

private:
  class Internal;
  Internal* m_internal;
};

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_ImportPPG_h
