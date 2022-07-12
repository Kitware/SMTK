//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_Operation_h
#define smtk_project_Operation_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace project
{

/// A base class for operations that require access to a project manager.
///
/// Operations that inherit from this class and that are created by an operation
/// manager that has a project manager observing it will have the project
/// manager assigned to them upon creation. Otherwise, the project manager must
/// be set manually.
class SMTKCORE_EXPORT Operation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::project::Operation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  void setProjectManager(smtk::project::WeakManagerPtr);
  smtk::project::ManagerPtr projectManager();

  /// This method is re-declared to give it public access.
  ///
  /// Many of the project operations use a project manager to properly define
  /// their input parameters. We must therefore call `createSpecification` from
  /// an instance that has the project manager set when registering these
  /// operations with the operation manager.
  using smtk::operation::XMLOperation::createSpecification;

private:
  smtk::project::WeakManagerPtr m_projectManager;
};
} // namespace project
} // namespace smtk

#endif
