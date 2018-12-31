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
class qtMeshSelectionItem;
class qtModelEntityItem;
class qtBaseView;
class qtModelView;

typedef qtBaseView* (*widgetConstructor)(const ViewInfo& info);
typedef qtItem* (*qtItemConstructor)(const AttributeItemInfo& info);

/**\brief Container for managers whose content is presented via Qt widgets.
  *
  * This class serves as a clearing-house where Qt widgets that present
  * SMTK attributes can fetch content such as operations and resources.
  */
class SMTKQTEXT_EXPORT qtUIManager : public QObject
{

  Q_OBJECT

public:
  qtUIManager(smtk::attribute::ResourcePtr resource);
  qtUIManager(
    smtk::operation::OperationPtr operation, smtk::resource::ManagerPtr resourceManager = nullptr);
  virtual ~qtUIManager();

  void initializeUI(QWidget* pWidget, bool useInternalFileBrowser = false);
  void initializeUI(const smtk::extension::ViewInfo& v, bool useInternalFileBrowser = false);

  /// If this instance was constructed with an operation, return an appropriate view for it.
  smtk::view::ViewPtr findOrCreateOperationView() const;

  /// If this instance was constructed with an operation, return it.
  smtk::operation::OperationPtr operation() const { return m_operation; }

  /// Use the given smtk::view::View to construct widgets matching the specification.
  qtBaseView* setSMTKView(smtk::view::ViewPtr v);
  qtBaseView* setSMTKView(
    smtk::view::ViewPtr v, QWidget* pWidget, bool useInternalFileBrowser = true);
  qtBaseView* setSMTKView(const smtk::extension::ViewInfo& v, bool useInternalFileBrowser = true);
  smtk::view::ViewPtr smtkView() const { return m_smtkView; }

  smtk::resource::ManagerPtr resourceManager() const { return m_resourceManager; }
  void setResourceManager(smtk::resource::ManagerPtr mgr) { m_resourceManager = mgr; }

  smtk::operation::ManagerPtr operationManager() const { return m_operationManager; }
  void setOperationManager(smtk::operation::ManagerPtr mgr) { m_operationManager = mgr; }

  smtk::attribute::ResourcePtr attResource() const { return m_attResource; }

  void setActiveModelView(smtk::extension::qtModelView*);
  smtk::extension::qtModelView* activeModelView();

  // Description:
  // Set/Get the color used for indicating items with default values
  void setDefaultValueColor(const QColor& color);
  QColor defaultValueColor() const { return this->DefaultValueColor; }
  void setInvalidValueColor(const QColor& color);
  QColor invalidValueColor() const { return this->InvalidValueColor; }

  // Description:
  // Set the advanced values font to be bold and/or italic
  void setAdvanceFontStyleBold(bool val);
  bool advanceFontStyleBold() const;
  void setAdvanceFontStyleItalic(bool val);
  bool advanceFontStyleItalic() const;

  void setAdvancedBold(bool b) { this->AdvancedBold = b; }
  bool advancedBold() { return this->AdvancedBold; }
  void setAdvancedItalic(bool i) { this->AdvancedItalic = i; }
  bool advancedItalic() { return this->AdvancedItalic; }

  //Description:
  // Set and Get Value Label Lengths
  int maxValueLabelLength() const { return m_maxValueLabelLength; }
  void setMaxValueLabelLength(int w) { m_maxValueLabelLength = w; }
  int minValueLabelLength() const { return m_minValueLabelLength; }
  void setMinValueLabelLength(int w) { m_minValueLabelLength = w; }

  //Description:
  // Registers a view construction function with a view type string
  void registerViewConstructor(const std::string& vtype, widgetConstructor f);
  //Description:
  // Check if view type string has a registered view construction function
  bool hasViewConstructor(const std::string& vtype) const
  {
    return m_constructors.find(vtype) != m_constructors.end();
  }

  // Registers a qtItem construction function with a qtItem type string
  void registerItemConstructor(const std::string& vtype, qtItemConstructor f);
  //Description:
  // Check if view type string has a registered view construction function
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

  // The default colors defined in smtk::attribute::Definition presuppose the
  // use of a dark font. This method tests the font lightness and, if the font
  // color is light, it adapts the input color to contrast with it.
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

  //Mechanism for creating new GUI view based on registered factory functions
  qtBaseView* createView(const ViewInfo& info);

  //Mechanism for creating new GUI item based on registered factory functions
  qtItem* createItem(const AttributeItemInfo& info);

  // Methods for dealing with selection process
  smtk::view::SelectionPtr selection() const { return m_selection; }

  void setSelection(smtk::view::SelectionPtr newSel) { m_selection = newSel; }

  int selectionBit() const { return m_selectionBit; }
  void setSelectionBit(int val) { m_selectionBit = val; }

  static qtItem* defaultItemConstructor(const AttributeItemInfo& info);

#ifdef _WIN32
#define LINE_BREAKER_STRING "\n";
#else
#define LINE_BREAKER_STRING "\r";
#endif

public slots:
  void onFileItemCreated(smtk::extension::qtFileItem*);
  void onModelEntityItemCreated(smtk::extension::qtModelEntityItem*);
  void updateModelViews();
  void onViewUIModified(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
  void setAdvanceLevel(int b);
  void onOperationFinished();

signals:
  void fileItemCreated(smtk::extension::qtFileItem* fileItem);
  void modelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
  void viewUIChanged(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
  void refreshEntityItems();

  friend class qtBaseView;
  friend class qtAssociationWidget;

protected:
  void commonConstructor();
  virtual void internalInitialize();

private:
  qtBaseView* m_topView;
  smtk::view::ViewPtr m_smtkView;
  QWidget* m_parentWidget;
  qtModelView* m_activeModelView;

  QFont advFont;
  QColor DefaultValueColor;
  QColor InvalidValueColor;
  bool AdvancedBold;   // true by default
  bool AdvancedItalic; // false by default

  smtk::attribute::ResourcePtr m_attResource;
  smtk::resource::ManagerPtr m_resourceManager;
  smtk::operation::ManagerPtr m_operationManager;
  smtk::operation::OperationPtr m_operation;
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
