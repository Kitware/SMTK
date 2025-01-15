//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKToolbar_h
#define smtk_extension_paraview_appcomponents_pqSMTKToolbar_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include <QToolBar>

class vtkSMSMTKWrapperProxy;
class pqSMTKWrapper;
class pqServer;
class qtOperationTypeModel;

/**\brief A base class for toolbars that wish to include SMTK operation buttons.
  *
  * Inherit this class and provide an implementation for the pure virtual
  * populateToolbar() method which will be invoked whenever an SMTK wrapper
  * object is created (due to a client-server connection being established).
  * This method should clear the toolbar and re-insert all its items.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationToolbar : public QToolBar
{
  Q_OBJECT

  using Superclass = QToolBar;

public:
  /// Construct the toolbar.
  ///
  /// Your subclass should set the object name and window title here.
  /// ```cpp
  ///   this->setObjectName("ToolbarName");
  ///   this->setWindowTitle("User-presentable toolbar name");
  /// ```
  ///
  /// The window title is used to provide the name of the toolbar
  /// in the application's "View" menu.
  pqSMTKOperationToolbar(QWidget* parent = nullptr);

  /// Destroy the toolbar.
  ~pqSMTKOperationToolbar() override;

protected: // NOLINT(readability-redundant-access-specifiers)
  /// This method is called immediately after startup (as soon as the
  /// event loop starts) to populate the toolbar as your subclass sees fit.
  ///
  /// Call operationModel->insertIntoToolBar<OpList>() as many times as you
  /// like to insert a tool-button for each operation in \a OpList.
  ///
  /// This method is delayed until startup to ensure \a operationModel
  /// is available.
  virtual void populateToolbar(qtOperationTypeModel* operationModel) = 0;

  class pqInternal;
  pqInternal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKOperationToolbar);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKToolbar_h
