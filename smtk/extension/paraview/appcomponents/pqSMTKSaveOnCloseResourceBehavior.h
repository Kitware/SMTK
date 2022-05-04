//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKSaveOnCloseResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKSaveOnCloseResourceBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/paraview/appcomponents/fixWrap.h"

#include <QObject>

/// A behavior for prompting the user to save resources before closing them.
/// Currently, this occurs in three places:
///
/// 1. A pipeline source is destroyed.
/// 2. A resource manager is removed from the server.
/// 3. The main window event manager has received a close event.
///
/// Absent from this list is the case when "File->Close Resource" is selected by
/// the user. This instance is handled by pqSMTKCloseResourceBehavior, which
/// additionally provides an option to cancel the close action. Of the three
/// instances where this behavior prompts to save before closing resources, only
/// the third instance is cancelable. The other two are connected to ParaView
/// signals that do not support cancelling the action.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSaveOnCloseResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKSaveOnCloseResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKSaveOnCloseResourceBehavior() override;

  static int showDialog(bool& cbChecked, std::size_t numberOfUnsavedResources, bool showCancel);
  static int showDialogWithPrefs(std::size_t numberOfUnsavedResources, bool showCancel);

protected:
  pqSMTKSaveOnCloseResourceBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKSaveOnCloseResourceBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKSaveOnCloseResourceBehavior_h
