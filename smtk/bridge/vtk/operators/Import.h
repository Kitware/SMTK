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

#include "smtk/bridge/vtk/Operation.h"
#include "smtk/bridge/vtk/Resource.h"

namespace smtk
{
namespace bridge
{
namespace vtk
{

class LegacyRead;
class Read;

class SMTKVTKSESSION_EXPORT Import : public Operation
{
  friend class LegacyRead;
  friend class Read;

public:
  smtkTypeMacro(smtk::bridge::vtk::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  virtual Result importExodus(const smtk::bridge::vtk::Resource::Ptr&);
  virtual Result importSLAC(const smtk::bridge::vtk::Resource::Ptr&);
  virtual Result importLabelMap(const smtk::bridge::vtk::Resource::Ptr&);

  virtual const char* xmlDescription() const override;

  std::vector<smtk::common::UUID> m_preservedUUIDs;
};

SMTKVTKSESSION_EXPORT smtk::resource::ResourcePtr importResource(const std::string&);
} // namespace vtk
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_vtk_Import_h
