//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtBaseAttributeView - a base class for all view types
// .SECTION Description

#ifndef smtk_extension_qtBaseAttributeView_h
#define smtk_extension_qtBaseAttributeView_h

#include "smtk/extension/qt/qtBaseView.h"

class qtBaseAttributeViewInternals;
class QScrollArea;

namespace smtk
{
namespace extension
{
class qtUIManager;
class qtItem;
class qtViewInfoDialog;

class SMTKQTEXT_EXPORT qtBaseAttributeView : public qtBaseView
{
  Q_OBJECT

public:
  qtBaseAttributeView(const ViewInfo& info);
  ~qtBaseAttributeView() override;

  void setInitialCategory() override;

  /// Determines if an item should be displayed
  virtual bool displayItem(smtk::attribute::ItemPtr);
  virtual void getDefinitions(
    smtk::attribute::DefinitionPtr attDef, QList<smtk::attribute::DefinitionPtr>& defs);
  int fixedLabelWidth() { return m_fixedLabelWidth; }

  bool setFixedLabelWidth(int w);
  bool advanceLevelVisible() { return m_advOverlayVisible; }
  bool useSelectionManager() const { return m_useSelectionManager; }

  int advanceLevel() override;
  bool categoryEnabled() override;
  std::string currentCategory() override;

  void setTopLevelCategories(const std::set<std::string>& categories) override;

  //Returns true if the view does not contain any information to display - the default
  // behavior is to return false
  bool isEmpty() const override;

signals:
  void modified(smtk::attribute::ItemPtr);

public slots:
  void updateUI() override
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
  void showAdvanceLevel(int i) override;
  void enableShowBy(int /* enable */) override;
  void onInfo() override;

  virtual void requestModelEntityAssociation() { ; }

protected slots:
  virtual void updateAttributeData() { ; }
  virtual void onAdvanceLevelChanged(int levelIdx);

protected:
  /// Create the UI related to the view and assigns it to the parent widget.
  void buildUI() override;
  /// Adds properties associated with respects to a top level view
  void makeTopLevel() override;

  /// \brief Test for category filtering.
  /// Returns true if the item's categories pass
  virtual bool categoryTest(smtk::attribute::ItemPtr);

  /// \brief Test for advance level filtering.
  /// Returns true if the item's advance level pass
  virtual bool advanceLevelTest(smtk::attribute::ItemPtr);

  QScrollArea* m_ScrollArea;
  bool m_topLevelInitialized;

private:
  int m_fixedLabelWidth;
  qtBaseAttributeViewInternals* Internals;

}; // class

} // namespace attribute
} // namespace smtk

#endif
