//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKSubtractUI_h
#define smtk_extension_paraview_appcomponents_pqSMTKSubtractUI_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/fixWrap.h"

#include <QObject>

#include <functional>

class pqProxy;
class pqServer;
class pqSMTKResource;
class pqSMTKWrapper;
class pqView;

/** \brief Provide applications a way to remove (and restore) portions of ParaView's UI.
  *
  * Applications may call methods to hide (not just disable) or show (and enable)
  * UI components present in ParaView at startup, including
  * + menu bars
  * + menu items
  * + toolbar buttons
  * + dock widgets (panels)
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSubtractUI : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKSubtractUI* instance(QObject* parent = nullptr);
  ~pqSMTKSubtractUI() override;

public Q_SLOTS:
  /// Remove (or restore) a menu item (via its QAction) by its text name.
  /// This method is often easier but not robust when language translations are enabled.
  virtual void
  toggleMenuItem(const std::string& itemPath, const std::string& sep = "->", bool remove = true);
  virtual void removeMenuItem(const std::string& itemPath, const std::string& sep = "->")
  {
    this->toggleMenuItem(itemPath, sep, true);
  }
  virtual void restoreMenuItem(const std::string& itemPath, const std::string& sep = "->")
  {
    this->toggleMenuItem(itemPath, sep, false);
  }

  /// Remove (or restore) a menu item (via its QAction) by its QObject name.
  /// This method is robust but requires the developer to have assigned the object a name.
  virtual void toggleMenuItemByObjectName(const std::string& itemObjectName, bool remove = true);
  virtual void removeMenuItemByObjectName(const std::string& itemObjectName)
  {
    this->toggleMenuItemByObjectName(itemObjectName, true);
  }
  virtual void restoreMenuItemByObjectName(const std::string& itemObjectName)
  {
    this->toggleMenuItemByObjectName(itemObjectName, false);
  }

  /// Remove (or restore) a toolbar button (via its QAction).
  virtual void toggleToolbar(const std::string& toolbar, bool remove = true);
  virtual void removeToolbar(const std::string& toolbar) { this->toggleToolbar(toolbar, true); }
  virtual void restoreToolbar(const std::string& toolbar) { this->toggleToolbar(toolbar, false); }

  /// Remove (or restore) a toolbar button (via its QAction).
  virtual void
  toggleToolbarButton(const std::string& toolbar, const std::string& button, bool remove = true);
  virtual void removeToolbarButton(const std::string& toolbar, const std::string& button)
  {
    this->toggleToolbarButton(toolbar, button, true);
  }
  virtual void restoreToolbarButton(const std::string& toolbar, const std::string& button)
  {
    this->toggleToolbarButton(toolbar, button, false);
  }

  /// Remove (or restore) a QAction by its object name.
  /// Note that this requires the developer to have assigned the action a name, not just a title.
  virtual void toggleActionByObjectName(const std::string& action, bool remove = true);
  virtual void removeActionByObjectName(const std::string& action)
  {
    this->toggleActionByObjectName(action, true);
  }
  virtual void restoreActionByObjectName(const std::string& action)
  {
    this->toggleActionByObjectName(action, false);
  }

  /// Remove (or restore) a dock widget (panel) given its title.
  virtual void togglePanel(const std::string& panel, bool remove = true);
  virtual void removePanel(const std::string& panel) { this->togglePanel(panel, true); }
  virtual void restorePanel(const std::string& panel) { this->togglePanel(panel, false); }

protected:
  pqSMTKSubtractUI(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKSubtractUI);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKSubtractUI_h
