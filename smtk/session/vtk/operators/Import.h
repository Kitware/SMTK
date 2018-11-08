//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_vtk_Import_h
#define __smtk_session_vtk_Import_h

#include "smtk/session/vtk/Operation.h"
#include "smtk/session/vtk/Resource.h"

namespace smtk
{
namespace session
{
namespace vtk
{

class Read;
class LegacyRead;

class SMTKVTKSESSION_EXPORT Import : public Operation
{
  friend Read;
  friend LegacyRead;

public:
  smtkTypeMacro(smtk::session::vtk::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual Result importExodus(const smtk::session::vtk::Resource::Ptr&);
  virtual Result importSLAC(const smtk::session::vtk::Resource::Ptr&);
  virtual Result importLabelMap(const smtk::session::vtk::Resource::Ptr&);

  virtual const char* xmlDescription() const override;

  std::vector<smtk::common::UUID> m_preservedUUIDs;
};

SMTKVTKSESSION_EXPORT smtk::resource::ResourcePtr importResource(const std::string&);
} // namespace vtk
} // namespace session
} // namespace smtk

#endif // __smtk_session_vtk_Import_h
