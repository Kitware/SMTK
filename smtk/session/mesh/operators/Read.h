//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_Read_h
#define smtk_session_mesh_Read_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief Read an smtk mesh model file.
 *
 * Here and throughout SMTK, we use the terms read/write to describe
 * serialization of native SMTK files, while the terms import/export describe
 * transcription from/to a different format. Currently, the mesh session's
 * model file is a .smtk json file that describes model relationships and
 * contains the url to the mesh file. The read operation is therefore a veneer
 * around the import operation; it simply opens the .smtk file, accesses the url
 * of the mesh and imports it.
 */
class SMTKMESHSESSION_EXPORT Read : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::session::mesh::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKMESHSESSION_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace mesh
} // namespace session
} // namespace smtk

#endif
