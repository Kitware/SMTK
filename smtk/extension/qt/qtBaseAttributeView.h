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
  smtkTypenameMacro(qtBaseAttributeView);

  qtBaseAttributeView(const smtk::view::Information& info);
  ~qtBaseAttributeView() override;

  void setInitialCategory() override;

  /// Determines if an item should be displayed
  ///
  /// This method is called by qtAttribute::createWidget().
  /// If you override this method, you may wish to call this from
  /// your override, since it checks the advance-level and category
  /// settings for the item.
  virtual bool displayItem(const smtk::attribute::ItemPtr&) const;
  /// Determines if an Attribute should be displayed solely based on categories
  ///
  /// This method is used by qtAttribute to deal with Attributes with units.
  virtual bool displayAttribute(const smtk::attribute::AttributePtr&) const;
  virtual bool displayItemDefinition(const smtk::attribute::ItemDefinitionPtr&) const;
  /// Determines if an item can be modified
  virtual bool isItemWriteable(const smtk::attribute::ItemPtr&) const;
  virtual void getDefinitions(
    smtk::attribute::DefinitionPtr attDef,
    QList<smtk::attribute::DefinitionPtr>& defs);
  int fixedLabelWidth() { return m_fixedLabelWidth; }

  bool setFixedLabelWidth(int w);
  bool advanceLevelVisible() { return m_advOverlayVisible; }
  bool useSelectionManager() const { return m_useSelectionManager; }

  int advanceLevel() const override;
  bool categoryEnabled() const override;
  std::string currentCategory() const override;

  //Returns true if the view does not contain any information to display - the default
  // behavior is to return false
  bool isEmpty() const override;

  // Provide a mechanism to by-pass category filtering
  void setIgnoreCategories(bool mode);
  bool ignoreCategories() const { return m_ignoreCategories; }

  /// Return the attribute resource used by this View
  smtk::attribute::ResourcePtr attributeResource() const;

  // Validates the view information to see if it is suitable for creating a qtAttributeBaseView instance
  static bool validateInformation(const smtk::view::Information& info);

Q_SIGNALS:
  void modified(smtk::attribute::ItemPtr);

public Q_SLOTS:

  virtual void updateModelAssociation() { ; }
  virtual void valueChanged(smtk::attribute::ItemPtr);
  /// Invoke the Signal dummy operation to indicate an attribute has been created.
  virtual void attributeCreated(const smtk::attribute::AttributePtr&);
  /// Invoke the Signal dummy operation to indicate an attribute has been changed (renamed).
  virtual void attributeChanged(
    const smtk::attribute::AttributePtr&,
    std::vector<std::string> items = std::vector<std::string>());
  /// Invoke the Signal dummy operation to indicate an attribute has been removed.
  virtual void attributeRemoved(const smtk::attribute::AttributePtr&);
  void showAdvanceLevel(int i) override;
  void enableShowBy(int /* enable */) override;
  void onInfo() override;

protected Q_SLOTS:
  virtual void onAdvanceLevelChanged(int levelIdx);
  void onConfigurationChanged(int levelIdx);

protected:
  /// Create the UI related to the view and assigns it to the parent widget.
  void buildUI() override;
  /// Adds properties associated with respects to a top level view
  void makeTopLevel() override;

  /// Returns true if |att| is valid.
  bool checkStatus(const smtk::attribute::Attribute* att) const;

  /// \brief Test for category filtering.
  /// Returns true if the item's categories pass
  virtual bool categoryTest(const smtk::attribute::ItemPtr&) const;

  /// \brief Test for advance level filtering.
  /// Returns true if the item's advance level pass
  virtual bool advanceLevelTest(const smtk::attribute::ItemPtr&) const;

  void topLevelPrepCategories(
    const smtk::view::ConfigurationPtr& view,
    const smtk::attribute::ResourcePtr& attResource);
  void topLevelPrepConfigurations(
    const smtk::view::ConfigurationPtr& view,
    const smtk::attribute::ResourcePtr& attResource);
  void topLevelPrepAdvanceLevels(const smtk::view::ConfigurationPtr& view);
  void prepConfigurationComboBox(const std::string& newConfigurationName);
  void checkConfigurations(smtk::attribute::ItemPtr& item);
  QScrollArea* m_ScrollArea;
  bool m_topLevelInitialized;
  bool m_topLevelCanCreateConfigurations{ false };
  smtk::attribute::WeakDefinitionPtr m_topLevelConfigurationDef;
  bool m_ignoreCategories;
  std::string m_addressString;

private:
  int m_fixedLabelWidth;
  qtBaseAttributeViewInternals* Internals;

}; // class

} // namespace extension
} // namespace smtk

#endif
