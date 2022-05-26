//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_pqSMTKProjectBrowser_h
#define smtk_extension_paraview_project_pqSMTKProjectBrowser_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/extension/qt/qtResourceBrowser.h"

#include "smtk/PublicPointerDefs.h"

class QAbstractItemModel;
class QItemSelection;

class pqSMTKResource;
class pqSMTKWrapper;

class pqRepresentation;
class pqServer;
class pqView;

/**\brief A widget that displays SMTK projects available to the application/user.
  */
class SMTKPQPROJECTEXT_EXPORT pqSMTKProjectBrowser : public smtk::extension::qtResourceBrowser
{
  Q_OBJECT
  typedef smtk::extension::qtResourceBrowser Superclass;

public:
  smtkTypenameMacro(pqSMTKProjectBrowser);

  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);
  pqSMTKProjectBrowser(const smtk::view::Information& info);
  ~pqSMTKProjectBrowser() override;

  /// Return the string that represents the configuration for browser components,
  /// specialized from qtResourceBrowser version.
  static const std::string getJSONConfiguration();

protected Q_SLOTS:
  virtual void searchTextChanged(const QString& searchText);

  virtual void sourceAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void sourceRemoved(pqSMTKWrapper* mgr, pqServer* server);

  /// Called when vtkSMTKSettings is modified, indicating highlight-on-hover behavior may change.
  virtual void updateSettings();

protected:
  void initSubphraseGenerator();
};

#endif
