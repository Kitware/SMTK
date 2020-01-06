//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_qtUIManager_h
#define __smtk_extension_qtUIManager_h

#include "smtk/attribute/Resource.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h" // Needed for ViewInfo definition
#include "smtk/extension/qt/qtItem.h"
#include <QColor>
#include <QFont>
#include <QMap>
#include <QPointer>
#include <QTextEdit>
#include <QVariant>
#include <map>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;

namespace smtk
{
namespace extension
{
class qtItem;
class qtFileItem;
class qtModelEntityItem;
class qtBaseView;
class qtModelView;

typedef qtBaseView* (*widgetConstructor)(const ViewInfo& info);
typedef qtItem* (*qtItemConstructor)(const qtAttributeItemInfo& info);

/**\brief Container for managers whose content is presented via Qt widgets.
  *
  * This class serves as a clearing-house where Qt widgets that present
  * SMTK attributes can fetch content such as operations and resources.
  */
class SMTKQTEXT_EXPORT qtUIManager : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QColor defaultValueColor READ defaultValueColor WRITE setDefaultValueColor);
  Q_PROPERTY(
    QVariantList defaultValueColorRgbF READ defaultValueColorRgbF WRITE setDefaultValueColorRgbF);

  Q_PROPERTY(QColor invalidValueColor READ invalidValueColor WRITE setInvalidValueColor);
  Q_PROPERTY(
    QVariantList invalidValueColorRgbF READ invalidValueColorRgbF WRITE setInvalidValueColorRgbF);

public:
  qtUIManager(const smtk::attribute::ResourcePtr& resource);
  qtUIManager(const smtk::operation::OperationPtr& operation,
    const smtk::resource::ManagerPtr& resourceManager, const smtk::view::ManagerPtr& viewManager);
  qtUIManager(
    const smtk::resource::ManagerPtr& resourceManager, const smtk::view::ManagerPtr& viewManager);
  virtual ~qtUIManager();

  void initializeUI(QWidget* pWidget, bool useInternalFileBrowser = false);
  void initializeUI(const smtk::extension::ViewInfo& v, bool useInternalFileBrowser = false);

  /// If this instance was constructed with an operation, return an appropriate view for it.
  smtk::view::ConfigurationPtr findOrCreateOperationView() const;

  /// If this instance was constructed with an operation, return it.
  smtk::operation::OperationPtr operation() const { return m_operation; }

  /// Use the given smtk::view::Configuration to construct widgets matching the specification.
  qtBaseView* setSMTKView(smtk::view::ConfigurationPtr v);
  qtBaseView* setSMTKView(
    smtk::view::ConfigurationPtr v, QWidget* pWidget, bool useInternalFileBrowser = true);
  qtBaseView* setSMTKView(const smtk::extension::ViewInfo& v, bool useInternalFileBrowser = true);
  smtk::view::ConfigurationPtr smtkView() const { return m_smtkView; }

  smtk::resource::ManagerPtr resourceManager() const { return m_resourceManager; }
  void setResourceManager(smtk::resource::ManagerPtr mgr) { m_resourceManager = mgr; }

  smtk::operation::ManagerPtr operationManager() const { return m_operationManager; }
  void setOperationManager(smtk::operation::ManagerPtr mgr) { m_operationManager = mgr; }

  smtk::view::ManagerPtr viewManager() const { return m_viewManager; }
  void setViewManager(smtk::view::ManagerPtr mgr) { m_viewManager = mgr; }

  smtk::attribute::ResourcePtr attResource() const { return m_attResource.lock(); }

  void setActiveModelView(smtk::extension::qtModelView*);
  smtk::extension::qtModelView* activeModelView();

  // Description:
  /// Set/Get the color used for indicating items with default values
  void setDefaultValueColor(const QColor& color);
  void setDefaultValueColorRgbF(const QVariantList& color);
  QColor defaultValueColor() const { return this->DefaultValueColor; }
  QVariantList defaultValueColorRgbF() const;

  void setInvalidValueColor(const QColor& color);
  void setInvalidValueColorRgbF(const QVariantList& color);
  QColor invalidValueColor() const { return this->InvalidValueColor; }
  QVariantList invalidValueColorRgbF() const;

  // Description:
  /// Set the advanced values font to be bold and/or italic
  void setAdvanceFontStyleBold(bool val);
  bool advanceFontStyleBold() const;
  void setAdvanceFontStyleItalic(bool val);
  bool advanceFontStyleItalic() const;

  void setAdvancedBold(bool b) { this->AdvancedBold = b; }
  bool advancedBold() { return this->AdvancedBold; }
  void setAdvancedItalic(bool i) { this->AdvancedItalic = i; }
  bool advancedItalic() { return this->AdvancedItalic; }

  //Description:
  /// Set and Get Value Label Lengths
  int maxValueLabelLength() const { return m_maxValueLabelLength; }
  void setMaxValueLabelLength(int w) { m_maxValueLabelLength = w; }
  int minValueLabelLength() const { return m_minValueLabelLength; }
  void setMinValueLabelLength(int w) { m_minValueLabelLength = w; }

  //Description:
  /// Registers a view construction function with a view type string
  void registerViewConstructor(const std::string& vtype, widgetConstructor f);
  //Description:
  /// Check if view type string has a registered view construction function
  bool hasViewConstructor(const std::string& vtype) const;

  // Registers a qtItem construction function with a qtItem type string
  void registerItemConstructor(const std::string& vtype, qtItemConstructor f);
  //Description:
  /// Check if view type string has a registered view construction function
  bool hasItemConstructor(const std::string& vtype) const
  {
    return m_itemConstructors.find(vtype) != m_itemConstructors.end();
  }

  qtBaseView* topView() { return m_topView; }
  static QString clipBoardText();
  static void setClipBoardText(QString& text);
  std::string currentCategory();
  bool categoryEnabled();
  void clearRoot();

  /// The default colors defined in smtk::attribute::Definition presuppose the
  /// use of a dark font. This method tests the font lightness and, if the font
  /// color is light, it adapts the input color to contrast with it.
  //
  // TODO: We may not want to define color in smtk::core. Instead, we may want
  //       to use Qt's convention of naming entities within a color palette
  //       rather than hard-coding colors; these descriptions should also live
  //       in the smtk::extensions::qt library, where they are used.
  static QColor contrastWithText(const QColor&);

  bool passAdvancedCheck(int level);
  bool passAttributeCategoryCheck(smtk::attribute::ConstDefinitionPtr AttDef);
  bool passItemCategoryCheck(smtk::attribute::ConstItemDefinitionPtr ItemDef);
  bool passCategoryCheck(const std::set<std::string>& categories);
  void disableCategoryChecks();
  void enableCategoryChecks();
  void setTopLevelCategories(const std::set<std::string>& categories);

  bool checkAttributeValidity(const smtk::attribute::Attribute* att);

  const QFont& advancedFont() { return this->advFont; }
  int advanceLevel() const { return m_currentAdvLevel; }
  void initAdvanceLevels(QComboBox* combo);

  void setWidgetColorToInvalid(QWidget* widget);
  void setWidgetColorToDefault(QWidget* widget);
  void setWidgetColorToNormal(QWidget* widget);
  bool getExpressionArrayString(smtk::attribute::GroupItemPtr dataItem, QString& strValues);

  static void updateArrayTableWidget(smtk::attribute::GroupItemPtr dataItem, QTableWidget* widget);
  static void updateTableColRows(smtk::attribute::ItemPtr dataItem, int col, QTableWidget* widget);

  static void updateArrayDataValue(smtk::attribute::GroupItemPtr dataItem, QTableWidgetItem* item);
  static void addNewTableValues(
    smtk::attribute::GroupItemPtr dataItem, QTableWidget* table, double* vals, int numVals);
  static void removeSelectedTableValues(
    smtk::attribute::GroupItemPtr dataItem, QTableWidget* table);

  bool updateTableItemCheckState(QTableWidgetItem* labelitem, smtk::attribute::ItemPtr attItem);

  virtual int getWidthOfAttributeMaxLabel(smtk::attribute::DefinitionPtr def, const QFont& font);
  virtual int getWidthOfItemsMaxLabel(
    const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs, const QFont& font);

  ///Mechanism for creating new GUI view based on registered factory functions
  qtBaseView* createView(const ViewInfo& info);

  ///Mechanism for creating new GUI item based on registered factory functions
  qtItem* createItem(const qtAttributeItemInfo& info);

  /// Methods for dealing with selection process
  smtk::view::SelectionPtr selection() const { return m_selection; }

  void setSelection(smtk::view::SelectionPtr newSel) { m_selection = newSel; }

  int selectionBit() const { return m_selectionBit; }
  void setSelectionBit(int val) { m_selectionBit = val; }

  int hoverBit() const { return m_hoverBit; }
  void setHoverBit(int val) { m_hoverBit = val; }

  /// See if we are dealing with a subset of categories
  bool topLevelCategoriesSet() const { return m_topLevelCategoriesSet; }

  ///methods for saving/retrieving the active tab in a group view
  void setActiveTabInfo(const std::string& groupViewName, const std::string& activeTabName);
  std::string activeTabInfo(const std::string& groupViewName) const;

  bool highlightOnHover() const { return m_highlightOnHover; }

  void setHighlightOnHover(bool val);

  static qtItem* defaultItemConstructor(const qtAttributeItemInfo& info);

#ifdef _WIN32
#define LINE_BREAKER_STRING "\n";
#else
#define LINE_BREAKER_STRING "\r";
#endif

public slots:
  void onFileItemCreated(smtk::extension::qtFileItem*);
  void onModelEntityItemCreated(smtk::extension::qtModelEntityItem*);
  void onViewUIModified(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
  void setAdvanceLevel(int b);
  void onOperationFinished();

signals:
  void fileItemCreated(smtk::extension::qtFileItem* fileItem);
  void modelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
  void viewUIChanged(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
  /**\brief Emitted by the UI manager when the user setting has changed so
    *       that children can reset any active hovers and update themselves.
    */
  void highlightOnHoverChanged(bool);
  /**\brief Emitted by the UI manager when the defaultValueColor property is changed internally.
    *
    * This is currently a dummy signal required by pqPropertyLinks to synchronize
    * the vtkSMProperty (used by ParaView to store user preferences) and the
    * QColor value (used by the UI manager). Since UI components never change the
    * default color, only responding to changes in user preferences, this signal
    * is unused but must be present.
    */
  void defaultValueColorChanged();
  /**\brief Emitted by the UI manager when the invalidValueColor property is changed internally.
    *
    * This is currently a dummy signal required by pqPropertyLinks to synchronize
    * the vtkSMProperty (used by ParaView to store user preferences) and the
    * QColor value (used by the UI manager). Since UI components never change the
    * invalid color, only responding to changes in user preferences, this signal
    * is unused but must be present.
    */
  void invalidValueColorChanged();
  void refreshEntityItems();

  friend class qtBaseView;
  friend class qtAssociationWidget;

protected:
  void commonConstructor();
  virtual void internalInitialize();

private:
  qtBaseView* m_topView;
  smtk::view::ConfigurationPtr m_smtkView;
  QWidget* m_parentWidget;
  qtModelView* m_activeModelView;

  QFont advFont;
  QColor DefaultValueColor;
  QColor InvalidValueColor;
  bool AdvancedBold;   // true by default
  bool AdvancedItalic; // false by default

  std::weak_ptr<smtk::attribute::Resource> m_attResource;
  smtk::resource::ManagerPtr m_resourceManager;
  smtk::operation::ManagerPtr m_operationManager;
  smtk::operation::OperationPtr m_operation;
  smtk::view::ManagerPtr m_viewManager;
  bool m_useInternalFileBrowser;

  int m_maxValueLabelLength;
  int m_minValueLabelLength;

  // current advance level to show advanced attributes/items
  int m_currentAdvLevel;

  // map for <Definition, its longest item label>
  // The longest label is used as a hint when aligning all item labels
  QMap<smtk::attribute::DefinitionPtr, std::string> Def2LongLabel;
  void findDefinitionsLongLabels();
  void findDefinitionLongLabel(smtk::attribute::DefinitionPtr def, std::string& labelText);
  void getItemsLongLabel(
    const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs, std::string& labelText);
  std::map<std::string, widgetConstructor> m_constructors;
  std::map<std::string, qtItemConstructor> m_itemConstructors;

  smtk::view::SelectionPtr m_selection;
  int m_selectionBit;

  // For Hover-based information
  bool m_highlightOnHover;
  int m_hoverBit;

  // indicates if the UI Manager should be filtering on categories at all
  bool m_categoryChecks;
  bool m_topLevelCategoriesSet;
  std::set<std::string> m_topLevelCategories;
  std::map<std::string, std::string> m_activeTabInfo;

}; // class

//A sublcass of QTextEdit to give initial sizehint
class SMTKQTEXT_EXPORT qtTextEdit : public QTextEdit
{
  Q_OBJECT
public:
  qtTextEdit(QWidget* parent);
  QSize sizeHint() const override;
};

}; // namespace extension
}; // namespace smtk

#endif
