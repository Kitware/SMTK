//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtManager_h
#define smtk_extension_qtManager_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/task/qtTaskNodeFactory.h"

namespace smtk
{
namespace extension
{

///\brief A qtManager is responsible for creating new qt-based objects.
///
/// Currently it provides a factory for creating qtTaskNodes.
class SMTKQTEXT_EXPORT qtManager : smtkEnableSharedPtr(qtManager)
{
public:
  smtkTypeMacroBase(smtk::extension::qtManager);
  smtkCreateMacro(smtk::extension::qtManager);

  virtual ~qtManager() = default;

  qtTaskNodeFactory& taskNodeFactory() { return m_taskNodeFactory; }
  const qtTaskNodeFactory& taskNodeFactory() const { return m_taskNodeFactory; }

private:
  qtTaskNodeFactory m_taskNodeFactory;

protected:
  qtManager() = default;
};
} // namespace extension
} // namespace smtk
#endif
