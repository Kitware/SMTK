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

#include "smtk/extension/qt/qtResourceBrowser.h"

#include "smtk/view/VisibilityContent.h"

#include "smtk/PublicPointerDefs.h"

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
class pqSMTKResourceBrowser : public smtk::extension::qtResourceBrowser
{
  Q_OBJECT
  typedef smtk::extension::qtResourceBrowser Superclass;

public:
  static smtk::extension::qtBaseView* createViewWidget(const smtk::extension::ViewInfo& info);
  pqSMTKResourceBrowser(const smtk::extension::ViewInfo& info);
  ~pqSMTKResourceBrowser() override;

  /// This method may be used by other ParaView plugins that wish to expose
  /// per-active-view visibility decorations on qtResourceBrowser widgets.
  static int panelPhraseDecorator(smtk::view::VisibilityContent::Query qq, int val,
    smtk::view::ConstPhraseContentPtr data, std::map<smtk::common::UUID, int>& visibleThings);

  /// Return the string that represents the configuration for browser components,
  /// specialized from qtResourceBrowser version.
  static const std::string getJSONConfiguration();

protected slots:
  virtual void searchTextChanged(const QString& searchText);

  virtual void resourceManagerAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server);

  /// Used to update phrase model with new visibility info for the active view.
  virtual void activeViewChanged(pqView*);

  /// Used to keep list of resource representations in active view up-to-date.
  virtual void representationAddedToActiveView(pqRepresentation*);
  virtual void representationRemovedFromActiveView(pqRepresentation*);

  /// Used to listen for self and others making changes to component visibilities in active view's representations.
  virtual void componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible);

  /// Called when vtkSMTKSettings is modified, indicating highlight-on-hover behavior may change.
  virtual void updateSettings();

protected:
  void initSubphraseGenerator();
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h
