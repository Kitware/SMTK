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

#include <QLayout>
#include <QList>
#include <QObject>

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"

class qtBaseViewInternals;
class QScrollArea;

namespace smtk
{
namespace extension
{
class qtUIManager;
class qtItem;

// This struct is used to initialize qtView-based classes
class SMTKQTEXT_EXPORT ViewInfo
{
public:
  ViewInfo(smtk::common::ViewPtr view, QWidget* parent, qtUIManager* uiman)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
  {
  }

  ViewInfo(smtk::common::ViewPtr view, QWidget* parent, qtUIManager* uiman,
    const std::map<std::string, QLayout*>& layoutDict)
    : m_view(view)
    , m_parent(parent)
    , m_UIManager(uiman)
    , m_layoutDict(layoutDict)
  {
  }

  ViewInfo() {}
  virtual ~ViewInfo() {}

  smtk::common::ViewPtr m_view;                 // View Definition
  QWidget* m_parent;                            // Parent Widget of the View
  qtUIManager* m_UIManager;                     // UI Manager
  std::map<std::string, QLayout*> m_layoutDict; // Widget Layout Dictionary
};

class SMTKQTEXT_EXPORT qtBaseView : public QObject
{
  Q_OBJECT

public:
  qtBaseView(const ViewInfo& info);
  virtual ~qtBaseView();

  smtk::common::ViewPtr getObject() const { return this->m_viewInfo.m_view; }
  QWidget* widget() const { return this->Widget; }
  QWidget* parentWidget() const { return this->m_viewInfo.m_parent; }
  qtUIManager* uiManager() const { return this->m_viewInfo.m_UIManager; }

  // Description:
  // Determines if an item should be displayed
  virtual bool displayItem(smtk::attribute::ItemPtr);
  virtual void getDefinitions(
    smtk::attribute::DefinitionPtr attDef, QList<smtk::attribute::DefinitionPtr>& defs);
  int fixedLabelWidth() { return m_fixedLabelWidth; }

  bool setFixedLabelWidth(int w);
  bool advanceLevelVisible() { return m_advOverlayVisible; }
  virtual int advanceLevel();
  virtual bool categoryEnabled();
  virtual std::string currentCategory();

  bool isTopLevel() const { return this->m_isTopLevel; }

signals:
  void modified(smtk::attribute::ItemPtr);

public slots:
  virtual void updateUI()
  {
    this->updateAttributeData();
    this->updateModelAssociation();
    this->showAdvanceLevelOverlay(m_advOverlayVisible);
  }
  virtual void updateModelAssociation() { ; }
  virtual void valueChanged(smtk::attribute::ItemPtr);
  virtual void childrenResized() { ; }
  virtual void showAdvanceLevelOverlay(bool val) { m_advOverlayVisible = val; }
  virtual void showAdvanceLevel(int i);
  virtual void updateViewUI(int /* currentTab */) {}
  virtual void enableShowBy(int /* enable */);
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

  QWidget* Widget;
  QScrollArea* m_ScrollArea;
  bool m_isTopLevel;
  bool m_topLevelInitialized;
  ViewInfo m_viewInfo;

private:
  bool m_advOverlayVisible;
  int m_fixedLabelWidth;
  qtBaseViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
