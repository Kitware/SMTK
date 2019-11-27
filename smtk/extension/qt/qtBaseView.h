//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtBaseView - a base class for all view types
// .SECTION Description

#ifndef smtk_extension_qtBaseView_h
#define smtk_extension_qtBaseView_h

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QLayout>

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtViewInfoDialog.h"

class QScrollArea;

namespace smtk
{
namespace extension
{
class qtUIManager;
class qtItem;
class qtViewInfoDialog;

// This struct is used to initialize qtView-based classes
class SMTKQTEXT_EXPORT ViewInfo
{
public:
  ViewInfo(smtk::view::ConfigurationPtr view, QWidget* parent, qtUIManager* uiman)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
  {
  }

  ViewInfo(smtk::view::ConfigurationPtr view, QWidget* parent, qtUIManager* uiman,
    const std::map<std::string, QLayout*>& layoutDict)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
    , m_layoutDict(layoutDict)
  {
  }

  ViewInfo() {}
  virtual ~ViewInfo() {}

  smtk::view::ConfigurationPtr m_view;          // View Definition
  QWidget* m_parent;                            // Parent Widget of the View
  qtUIManager* m_UIManager;                     // UI Manager
  std::map<std::string, QLayout*> m_layoutDict; // Widget Layout Dictionary
};

class SMTKQTEXT_EXPORT qtBaseView : public QObject
{
  Q_OBJECT

public:
  qtBaseView(const ViewInfo& info);
  ~qtBaseView() override;

  smtk::view::ConfigurationPtr getObject() const { return m_viewInfo.m_view; }
  QWidget* widget() const { return this->Widget; }
  QWidget* parentWidget() const { return m_viewInfo.m_parent; }
  qtUIManager* uiManager() const { return m_viewInfo.m_UIManager; }

  virtual int advanceLevel() { return 0; }
  virtual bool categoryEnabled() { return false; }
  virtual std::string currentCategory() { return ""; }
  // When category filtering is requested to be either on by default or is requested to be
  // on permanently, we need to have a mechanism to force the views to display info based on
  // the initial category.  This method will check to see if this is the case and call the
  // onShowCategory method
  virtual void setInitialCategory() { ; }

  bool isTopLevel() const { return m_isTopLevel; }
  virtual void setTopLevelCategories(const std::set<std::string>&) { ; }

  /// Return true if the view does not contain any information to display.
  /// Subclasses should override this method and return false by default
  /// unless they have specific knowledge that the view is vacuous.
  virtual bool isEmpty() const { return true; }

signals:
  void aboutToDestroy();

public slots:
  virtual void updateUI() { ; }
  virtual void childrenResized() { ; }
  virtual void showAdvanceLevelOverlay(bool val) { m_advOverlayVisible = val; }
  virtual void showAdvanceLevel(int /* level */) { ; }
  virtual void updateViewUI(int /* currentTab */) { ; }
  virtual void enableShowBy(int /* enable */) { ; }
  /// Display view information (i.e., help text)
  virtual void onInfo();

  virtual void onShowCategory() {}

protected:
  /// Creates the UI related to the view and properly assigns it
  /// to the parent widget.
  virtual void buildUI() { ; }
  /// Creates the main QT Widget that is associated with a View.  Typically this
  /// is the only method a derived View needs to override.
  virtual void createWidget() { ; }

  // Description:
  // Adds properties associated with respects to a top level view
  virtual void makeTopLevel();

  // Description:
  // Set the information to be displayed in the ViewInfoDialog
  virtual void setInfoToBeDisplayed();

  QWidget* Widget;
  bool m_isTopLevel;
  bool m_useSelectionManager;
  ViewInfo m_viewInfo;
  QPointer<qtViewInfoDialog> m_infoDialog;
  bool m_advOverlayVisible;
}; // class

} // namespace attribute
} // namespace smtk

#endif
