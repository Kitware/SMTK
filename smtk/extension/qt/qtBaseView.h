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

#ifndef __smtk_extension_qtBaseView_h
#define __smtk_extension_qtBaseView_h

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QLayout>

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtViewInfoDialog.h"

class qtBaseViewInternals;
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
  ViewInfo(smtk::view::ViewPtr view, QWidget* parent, qtUIManager* uiman)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
  {
  }

  ViewInfo(smtk::view::ViewPtr view, QWidget* parent, qtUIManager* uiman,
    const std::map<std::string, QLayout*>& layoutDict)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
    , m_layoutDict(layoutDict)
  {
  }

  ViewInfo() {}
  virtual ~ViewInfo() {}

  smtk::view::ViewPtr m_view;                   // View Definition
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

  smtk::view::ViewPtr getObject() const { return m_viewInfo.m_view; }
  QWidget* widget() const { return this->Widget; }
  QWidget* parentWidget() const { return m_viewInfo.m_parent; }
  qtUIManager* uiManager() const { return m_viewInfo.m_UIManager; }

  // Description:
  // Determines if an item should be displayed
  virtual bool displayItem(smtk::attribute::ItemPtr);
  virtual void getDefinitions(
    smtk::attribute::DefinitionPtr attDef, QList<smtk::attribute::DefinitionPtr>& defs);
  int fixedLabelWidth() { return m_fixedLabelWidth; }

  bool setFixedLabelWidth(int w);
  bool advanceLevelVisible() { return m_advOverlayVisible; }
  bool useSelectionManager() const { return m_useSelectionManager; }

  virtual int advanceLevel();
  virtual bool categoryEnabled();
  virtual std::string currentCategory();
  // When category filtering is requested to be either on by default or is requested to be
  // on permanently, we need to have a mechanism to force the views to display info based on
  // the initial category.  This method will check to see if this is the case and call the
  // onShowCategory method
  void setInitialCategory();

  bool isTopLevel() const { return m_isTopLevel; }

  //Returns true if the view does not contain any information to display - the default
  // behavior is to return false
  virtual bool isEmpty() const;

signals:
  void modified(smtk::attribute::ItemPtr);
  void aboutToDestroy();

public slots:
  virtual void updateUI()
  {
    this->updateAttributeData();
    this->updateModelAssociation();
    this->showAdvanceLevelOverlay(m_advOverlayVisible);
  }
  virtual void updateModelAssociation() { ; }
  virtual void valueChanged(smtk::attribute::ItemPtr);
  /// Invoke the Signal dummy operation to indicate an attribute has been created.
  virtual void attributeCreated(const smtk::attribute::AttributePtr&);
  /// Invoke the Signal dummy operation to indicate an attribute has been changed (renamed).
  virtual void attributeChanged(const smtk::attribute::AttributePtr&);
  /// Invoke the Signal dummy operation to indicate an attribute has been removed.
  virtual void attributeRemoved(const smtk::attribute::AttributePtr&);
  virtual void childrenResized() { ; }
  virtual void showAdvanceLevelOverlay(bool val) { m_advOverlayVisible = val; }
  virtual void showAdvanceLevel(int i);
  virtual void updateViewUI(int /* currentTab */) {}
  virtual void enableShowBy(int /* enable */);
  virtual void onInfo(); /* for displaying View Information */

  virtual void onShowCategory() {}

  virtual void requestModelEntityAssociation() { ; }

protected slots:
  virtual void updateAttributeData() { ; }
  virtual void onAdvanceLevelChanged(int levelIdx);

protected:
  // Description:
  // Creates the UI related to the view and properly assigns it
  // to the parent widget.
  virtual void buildUI();
  // Description:
  // Creates the main QT Widget that is associated with a View.  Typically this
  // is the only method a derived View needs to override.
  virtual void createWidget() {}

  // Description:
  // Adds properties associated with respects to a top level view
  virtual void makeTopLevel();

  // Description:
  // Set the information to be displayed in the ViewInfoDialog
  virtual void setInfoToBeDisplayed();

  QWidget* Widget;
  QScrollArea* m_ScrollArea;
  bool m_isTopLevel;
  bool m_topLevelInitialized;
  bool m_useSelectionManager;
  ViewInfo m_viewInfo;
  QPointer<qtViewInfoDialog> m_infoDialog;

private:
  bool m_advOverlayVisible;
  int m_fixedLabelWidth;
  qtBaseViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
