//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtUIManager_h
#define smtk_extension_qtUIManager_h

#include "smtk/attribute/Categories.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/Deprecation.h"
#include "smtk/common/TypeContainer.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Selection.h"

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

#include <QColor>
#include <QFont>
#include <QMap>
#include <QPixmap>
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

  Q_PROPERTY(QColor tempInvalidValueColor READ invalidValueColor WRITE setTempInvalidValueColor);
  Q_PROPERTY(QVariantList tempInvalidValueColorRgbF READ tempInvalidValueColorRgbF WRITE
               setTempInvalidValueColorRgbF);

public:
  qtUIManager(const smtk::attribute::ResourcePtr& resource);
  qtUIManager(
    const smtk::operation::OperationPtr& operation,
    const smtk::resource::ManagerPtr& resourceManager,
    const smtk::view::ManagerPtr& viewManager);
  qtUIManager(
    const smtk::resource::ManagerPtr& resourceManager,
    const smtk::view::ManagerPtr& viewManager);
  ~qtUIManager() override;

  void initializeUI(QWidget* pWidget, bool useInternalFileBrowser = false);
  void initializeUI(const smtk::view::Information& v, bool useInternalFileBrowser = false);

  /// If this instance was constructed with an operation, return an appropriate view for it.
  smtk::view::ConfigurationPtr findOrCreateOperationView() const;

  /// If this instance was constructed with an operation, return it.
  smtk::operation::OperationPtr operation() const { return m_operation; }

  ///@{
  /// Use the given smtk::view::Configuration to construct widgets matching the specification.
  qtBaseView* setSMTKView(const smtk::view::ConfigurationPtr& v);
  qtBaseView* setSMTKView(
    const smtk::view::ConfigurationPtr& v,
    QWidget* pWidget,
    bool useInternalFileBrowser = true);
  qtBaseView* setSMTKView(const smtk::view::Information& v, bool useInternalFileBrowser = true);
  smtk::view::ConfigurationPtr smtkView() const { return m_smtkView; }
  const smtk::view::Configuration::Component& findStyle(
    const smtk::attribute::DefinitionPtr& def,
    const std::string& styleName = "") const;
  ///}@

  smtk::resource::ManagerPtr& resourceManager()
  {
    return m_managers.get<smtk::resource::ManagerPtr>();
  }

  void setResourceManager(const smtk::resource::ManagerPtr& mgr)
  {
    m_managers.get<smtk::resource::ManagerPtr>() = mgr;
  }

  smtk::operation::ManagerPtr& operationManager()
  {
    return m_managers.get<smtk::operation::ManagerPtr>();
  }

  void setOperationManager(const smtk::operation::ManagerPtr& mgr)
  {
    m_managers.get<smtk::operation::ManagerPtr>() = mgr;
  }

  smtk::view::ManagerPtr& viewManager() { return m_managers.get<smtk::view::ManagerPtr>(); }

  void setViewManager(const smtk::view::ManagerPtr& mgr)
  {
    m_managers.get<smtk::view::ManagerPtr>() = mgr;
  }

  const smtk::common::TypeContainer& managers() const { return m_managers; }
  smtk::common::TypeContainer& managers() { return m_managers; }

  ///@{
  /// Set/Get the color used for indicating items with default values
  void setDefaultValueColor(const QColor& color);
  void setDefaultValueColorRgbF(const QVariantList& color);
  QColor defaultValueColor() const { return this->DefaultValueColor; }
  QVariantList defaultValueColorRgbF() const;
  ///}@

  ///@{
  /// Set/Get the invalid value color
  void setInvalidValueColor(const QColor& color);
  void setInvalidValueColorRgbF(const QVariantList& color);
  QColor invalidValueColor() const { return this->InvalidValueColor; }
  QVariantList invalidValueColorRgbF() const;
  ///}@

  ///@{
  /// Set/Get the "temporary" invalid value color
  void setTempInvalidValueColor(const QColor& color);
  void setTempInvalidValueColorRgbF(const QVariantList& color);
  QColor tempInvalidValueColor() const { return this->InvalidValueColor; }
  QVariantList tempInvalidValueColorRgbF() const;
  ///}@

  ///@{
  /// Get color values corrected based on the text color - this allows
  /// the system to change color themes and the resulting GUI can still be
  /// legible
  QColor correctedInvalidValueColor() const;
  QColor correctedNormalValueColor() const;
  QColor correctedDefaultValueColor() const;
  QColor correctedTempInvalidValueColor() const;
  ///@}

  ///@{
  /// Set the advanced values font to be bold and/or italic
  void setAdvanceFontStyleBold(bool val);
  bool advanceFontStyleBold() const;
  void setAdvanceFontStyleItalic(bool val);
  bool advanceFontStyleItalic() const;

  void setAdvancedBold(bool b) { this->AdvancedBold = b; }
  bool advancedBold() { return this->AdvancedBold; }
  void setAdvancedItalic(bool i) { this->AdvancedItalic = i; }
  bool advancedItalic() { return this->AdvancedItalic; }
  ///}@

  ///@{
  /// Set and Get Value Label Lengths
  int maxValueLabelLength() const { return m_maxValueLabelLength; }
  void setMaxValueLabelLength(int w) { m_maxValueLabelLength = w; }
  int minValueLabelLength() const { return m_minValueLabelLength; }
  void setMinValueLabelLength(int w) { m_minValueLabelLength = w; }
  ///}@

  /// Check if view type string has a registered view construction function
  bool hasViewConstructor(const std::string& vtype) const;

  /// Registers a qtItem construction function with a qtItem type string
  void registerItemConstructor(const std::string& vtype, qtItemConstructor f);
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
    smtk::attribute::GroupItemPtr dataItem,
    QTableWidget* table,
    double* vals,
    int numVals);
  static void removeSelectedTableValues(
    smtk::attribute::GroupItemPtr dataItem,
    QTableWidget* table);

  bool updateTableItemCheckState(QTableWidgetItem* labelitem, smtk::attribute::ItemPtr attItem);

  virtual int getWidthOfAttributeMaxLabel(smtk::attribute::DefinitionPtr def, const QFont& font);
  virtual int getWidthOfItemsMaxLabel(
    const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
    const QFont& font);
  virtual int getWidthOfText(const std::string& text, const QFont& font);

  ///Mechanism for creating new GUI view based on registered factory functions
  qtBaseView* createView(const smtk::view::Information& info);

  ///Mechanism for creating new GUI item based on registered factory functions
  qtItem* createItem(const qtAttributeItemInfo& info);

  /// Methods for dealing with selection process
  const smtk::view::SelectionPtr& selection() const
  {
    return m_managers.get<smtk::view::SelectionPtr>();
  }

  void setSelection(const smtk::view::SelectionPtr& newSel)
  {
    m_managers.get<smtk::view::SelectionPtr>() = newSel;
  }

  int selectionBit() const { return m_selectionBit; }
  void setSelectionBit(int val) { m_selectionBit = val; }

  int hoverBit() const { return m_hoverBit; }
  void setHoverBit(int val) { m_hoverBit = val; }

  bool highlightOnHover() const { return m_highlightOnHover; }

  void setHighlightOnHover(bool val);

  static qtItem* defaultItemConstructor(const qtAttributeItemInfo& info);

  ///\brief Return the pixmap used for alert icons
  const QPixmap& alertPixmap() const { return m_alertPixmap; }

  ///@{
  /// Set and Get methods to make the Views of the UI Manager read-only
  void setReadOnly(bool mode);
  bool isReadOnly() const { return m_readOnly; }

#ifdef _WIN32
#define LINE_BREAKER_STRING "\n";
#else
#define LINE_BREAKER_STRING "\r";
#endif

public Q_SLOTS:
  void onFileItemCreated(smtk::extension::qtFileItem*);
  void onModelEntityItemCreated(smtk::extension::qtModelEntityItem*);
  void onViewUIModified(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
  void setAdvanceLevel(int b);
  void onOperationFinished();

Q_SIGNALS:
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

  QFont advFont;
  QColor DefaultValueColor;
  QColor InvalidValueColor;
  QColor TempInvalidValueColor;
  bool AdvancedBold;   // true by default
  bool AdvancedItalic; // false by default

  std::weak_ptr<smtk::attribute::Resource> m_attResource;
  smtk::common::TypeContainer m_managers;
  smtk::operation::OperationPtr m_operation;
  bool m_useInternalFileBrowser;

  int m_maxValueLabelLength;
  int m_minValueLabelLength;

  // current advance level to show advanced attributes/items
  int m_currentAdvLevel;

  bool m_readOnly = false;

  // map for <Definition, its longest item label>
  // The longest label is used as a hint when aligning all item labels
  QMap<smtk::attribute::DefinitionPtr, std::string> Def2LongLabel;
  void findDefinitionsLongLabels();
  void findDefinitionLongLabel(smtk::attribute::DefinitionPtr def, std::string& labelText);
  void getItemsLongLabel(
    const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
    std::string& labelText);
  std::map<std::string, qtItemConstructor> m_itemConstructors;

  int m_selectionBit;

  // For Hover-based information
  bool m_highlightOnHover;
  int m_hoverBit;

  std::map<std::string, std::string> m_activeTabInfo;
  QPixmap m_alertPixmap;

  const std::string m_activeAdvLevelXmlAttName = "ActiveAdvanceLevel";

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
