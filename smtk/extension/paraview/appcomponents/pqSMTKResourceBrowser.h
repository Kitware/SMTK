//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/qt/qtResourceBrowser.h"

#include "smtk/PublicPointerDefs.h"

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS protected
#define Q_OBJECT
#endif

#include <QDockWidget>

class QAbstractItemModel;
class QItemSelection;

class pqSMTKResource;
class pqSMTKWrapper;

class pqRepresentation;
class pqServer;
class pqView;

/**\brief A widget that displays SMTK resources available to the application/user.
  *
  * This adds the following functionality to the qtResourceBrowser:
  * + Phrases related to resources and components will be decorated with an
  *   eyeball icon for controlling their visibility in the active ParaView view.
  * + The highlight on hover setting is controlled by a ParaView setting.
  * + The smtk PhraseModel used to populate the widget is connected-to and
  *   disconnected-from resource managers as client-server connections are
  *   made and broken.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourceBrowser : public smtk::extension::qtResourceBrowser
{
  Q_OBJECT
  typedef smtk::extension::qtResourceBrowser Superclass;

public:
  smtkTypenameMacro(pqSMTKResourceBrowser);

  static smtk::extension::qtBaseView* createViewWidget(const smtk::view::Information& info);
  pqSMTKResourceBrowser(const smtk::view::Information& info);
  ~pqSMTKResourceBrowser() override;

  /// Return the string that represents the configuration for browser components,
  /// specialized from qtResourceBrowser version.
  static const std::string getJSONConfiguration();

protected Q_SLOTS:
  virtual void searchTextChanged(const QString& searchText);

  virtual void resourceManagerAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server);

  /// Called when vtkSMTKSettings is modified, indicating highlight-on-hover behavior may change.
  virtual void updateSettings();

protected:
  void initSubphraseGenerator();
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h
