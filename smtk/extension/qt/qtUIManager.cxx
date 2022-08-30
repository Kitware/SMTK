//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtComponentItem.h"
#include "smtk/extension/qt/qtDateTimeItem.h"
#include "smtk/extension/qt/qtDirectoryItem.h"
#include "smtk/extension/qt/qtDoubleItem.h"
#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtGroupItem.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtInfixExpressionEditor.h"
#include "smtk/extension/qt/qtIntItem.h"
#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/qtReferenceItem.h"
#include "smtk/extension/qt/qtReferenceItemEditor.h"
#include "smtk/extension/qt/qtReferenceTree.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtResourceItem.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtStringItem.h"
#include "smtk/extension/qt/qtVoidItem.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/io/Logger.h"

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QComboBox>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QToolButton>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/FileSystemItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"

#include <cmath>

using namespace smtk::attribute;
using namespace smtk::extension;

qtTextEdit::qtTextEdit(QWidget* inParent)
  : QTextEdit(inParent)
{
}

QSize qtTextEdit::sizeHint() const
{
  return QSize(200, 70);
}
qtUIManager::qtUIManager(const smtk::attribute::ResourcePtr& resource)
  : m_parentWidget(nullptr)
  , m_attResource(resource)
  , m_useInternalFileBrowser(false)
{
  if (auto attResource = m_attResource.lock())
  {
    m_managers.insert(attResource->manager());
  }
  this->commonConstructor();
}

qtUIManager::qtUIManager(
  const smtk::operation::OperationPtr& op,
  const smtk::resource::ManagerPtr& resourceManager,
  const smtk::view::ManagerPtr& viewManager)
  : m_parentWidget(nullptr)
  , m_operation(op)
{
  m_managers.insert(resourceManager);
  m_managers.insert(viewManager);

  if (!op)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Asked to create an operation view with no operation.");
  }

  auto spec = op->specification();
  if (!spec)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Asked to create an operation view of " << op->index() << " (" << op->typeName() << ")"
                                              << " but operator has no specification.");
  }
  m_attResource = spec;

  this->commonConstructor();
}

qtUIManager::qtUIManager(
  const smtk::resource::ManagerPtr& resourceManager,
  const smtk::view::ManagerPtr& viewManager)
  : m_parentWidget(nullptr)
{
  m_managers.insert(resourceManager);
  m_managers.insert(viewManager);

  if (
    !m_managers.get<smtk::resource::Manager::Ptr>() || !m_managers.get<smtk::view::Manager::Ptr>())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Asked to create a browser view with missing resource or view manager.");
  }
  this->commonConstructor();
}

void qtUIManager::commonConstructor()
{
  m_useInternalFileBrowser = true;
  m_topView = nullptr;
  m_maxValueLabelLength = 200;
  m_minValueLabelLength = 50;
  m_highlightOnHover = true;
  m_alertPixmap = QPixmap(":/icons/attribute/errorAlert.png");

  // default settings
  this->advFont.setBold(true);
  this->advFont.setItalic(false);
  this->DefaultValueColor.setRgbF(1.0, 1.0, 0.5);
  this->InvalidValueColor.setRgbF(1.0, 0.5, 0.5);

  m_currentAdvLevel = 0;
  m_selectionBit = 0;
  m_hoverBit = 0;

  qtSMTKUtilities::registerModelViewConstructor(
    "ResourceTree", qtResourceBrowser::createDefaultView);

  // Lets register some basic item constructors
  this->registerItemConstructor("Default", qtUIManager::defaultItemConstructor);
  this->registerItemConstructor("qtComponentItem", qtComponentItem::createItemWidget);
  this->registerItemConstructor("qtDateTimeItem", qtDateTimeItem::createItemWidget);
  this->registerItemConstructor("qtDirectoryItem", qtDirectoryItem::createItemWidget);
  this->registerItemConstructor("qtDoubleItem", qtDoubleItem::createItemWidget);
  this->registerItemConstructor("qtFileItem", qtFileItem::createItemWidget);
  this->registerItemConstructor("qtGroupItem", qtGroupItem::createItemWidget);
  this->registerItemConstructor("qtIntItem", qtIntItem::createItemWidget);
  this->registerItemConstructor("qtReferenceItem", qtReferenceItem::createItemWidget);
  this->registerItemConstructor("qtReferenceItemEditor", qtReferenceItemEditor::createItemWidget);
  this->registerItemConstructor("qtReferenceTree", qtReferenceTree::createItemWidget);
  this->registerItemConstructor("qtResourceItem", qtResourceItem::createItemWidget);
  this->registerItemConstructor("qtStringItem", qtStringItem::createItemWidget);
  this->registerItemConstructor("qtVoidItem", qtVoidItem::createItemWidget);

  this->registerItemConstructor("InfixExpression", qtInfixExpressionEditor::createItemWidget);

  // register constructors coming from plugins.
  qtSMTKUtilities::updateItemConstructors(this);

  // Initialize the selection object.
  m_managers.insert<smtk::view::Selection::Ptr>(nullptr);
}

qtUIManager::~qtUIManager()
{
  delete m_topView;
}

void qtUIManager::initializeUI(QWidget* pWidget, bool useInternalFileBrowser)
{
  m_useInternalFileBrowser = useInternalFileBrowser;
  m_parentWidget = pWidget;
  if (m_topView)
  {
    delete m_topView;
    m_topView = nullptr;
  }

  if (!m_smtkView)
  {
    return;
  }
  this->internalInitialize();

  smtk::view::Information vinfo;
  vinfo.insert<smtk::view::ConfigurationPtr>(m_smtkView);
  vinfo.insert<QWidget*>(pWidget);
  vinfo.insert<qtUIManager*>(this);
  if (!m_operation)
  {
    vinfo.insert<std::weak_ptr<smtk::attribute::Resource>>(m_attResource);
  }
  else
  {
    vinfo.insert<smtk::operation::OperationPtr>(m_operation);
    vinfo.insert<std::weak_ptr<smtk::attribute::Resource>>(m_operation->specification());
  }

  m_topView = this->createView(vinfo);

  if (m_topView)
  {
    if (m_topView->configuration()->details().attribute(m_activeAdvLevelXmlAttName))
    {
      m_topView->configuration()->details().attributeAsInt(
        m_activeAdvLevelXmlAttName, m_currentAdvLevel);
    }
    if (m_currentAdvLevel) // only build advanced level when needed)
    {
      m_topView->showAdvanceLevel(m_currentAdvLevel);
    }
    // In case we are filtering by categories by default or
    // its set on permanently we need to  have the top level view
    // initially set its category.  The reason is because the UIManager's
    // currentCategory method is used to determine the current category but at
    // qtView construction time the UIManager's TopLevel View is not currently set
    // resulting in the current category not being set.  This method allows the UI
    // Manager to have the current category being set.
    m_topView->setInitialCategory();
  }
}

void qtUIManager::initializeUI(const smtk::view::Information& viewInfo, bool useInternalFileBrowser)
{
  m_useInternalFileBrowser = useInternalFileBrowser;
  m_parentWidget = viewInfo.get<QWidget*>();
  m_smtkView = viewInfo.get<smtk::view::ConfigurationPtr>();
  if (m_topView)
  {
    delete m_topView;
    m_topView = nullptr;
  }

  if (!m_smtkView)
  {
    return;
  }
  this->internalInitialize();

  m_topView = this->createView(viewInfo);
  if (m_topView)
  {
    if (m_currentAdvLevel) // only build advanced level when needed
    {
      m_topView->showAdvanceLevel(m_currentAdvLevel);
    }
    m_topView->setInitialCategory();
  }
}

bool qtUIManager::hasViewConstructor(const std::string& vtype) const
{
  const auto& viewManager = m_managers.get<smtk::view::Manager::Ptr>();
  if (!viewManager)
  {
    return false;
  }

  return viewManager->viewWidgetFactory().contains(vtype) ||
    viewManager->viewWidgetFactory().containsAlias(vtype);
}

smtk::view::ConfigurationPtr qtUIManager::findOrCreateOperationView() const
{
  smtk::view::ConfigurationPtr config;
  auto op = m_operation;
  if (!op)
  {
    return config;
  }

  smtk::attribute::ResourcePtr rsrc = op->specification();
  if (!rsrc)
  {
    return config;
  }

  smtk::attribute::AttributePtr params = op->parameters();

  // See if the operation has a manually-specified view.
  for (auto it = rsrc->views().begin(); it != rsrc->views().end(); ++it)
  {
    // If this is an "Operation" view we need to check its InstancedAttributes child,
    // otherwise we need to check AttributeTypes:
    int i; // View Component index we need to check
    if (it->second->type() == "Operation")
    {
      i = it->second->details().findChild("InstancedAttributes");
    }
    else
    {
      i = it->second->details().findChild("AttributeTypes");
    }
    if (i < 0)
    {
      continue;
    }
    smtk::view::Configuration::Component& comp = it->second->details().child(i);
    for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
      std::string optype;
      if (comp.child(ci).attribute("Type", optype) && optype == params->type())
      {
        config = it->second;
        // If we are dealing with an Operation View - The Name attribute needs to be
        // set to the same of the attribute the operator is using - so in practice
        // the Name attribute does not have to be set in the operator's sbt info
        if (config->type() == "Operation")
        {
          comp.child(ci).setAttribute("Name", params->name());
        }
        break;
      }
    }
    if (config && this->hasViewConstructor(config->type()))
    {
      break;
    }
  }

  // There is no config or there is a config but the UI manager does
  // not know about it (perhaps because the plugin which should
  // register widgets for the config is not loaded, perhaps because
  // the config name is incorrect).
  if (!config || !this->hasViewConstructor(config->type()))
  { // Create an "Operation" view.
    if (op)
    {
      config = smtk::view::Configuration::New("Operation", op->typeName());
      config->details().setAttribute("UseSelectionManager", "true");
      smtk::view::Configuration::Component& comp =
        config->details().addChild("InstancedAttributes").addChild("Att");
      comp.setAttribute("Type", params->type()).setAttribute("Name", params->name());
      rsrc->addView(config);
    }
  }

  config->details().setAttribute("TopLevel", "true");
  return config;
}

qtBaseView* qtUIManager::setSMTKView(
  const smtk::view::Information& viewInfo,
  bool useInternalFileBrowser)
{
  if (
    (m_smtkView == viewInfo.get<smtk::view::ConfigurationPtr>()) &&
    (m_parentWidget == viewInfo.get<QWidget*>()) &&
    (m_useInternalFileBrowser == useInternalFileBrowser))
  {
    return m_topView;
  }
  this->initializeUI(viewInfo, m_useInternalFileBrowser);
  return m_topView;
}

qtBaseView* qtUIManager::setSMTKView(const smtk::view::ConfigurationPtr& v)
{
  if (m_smtkView != v)
  {
    m_smtkView = v;
    this->initializeUI(m_parentWidget, m_useInternalFileBrowser);
  }
  return m_topView;
}

qtBaseView* qtUIManager::setSMTKView(
  const smtk::view::ConfigurationPtr& v,
  QWidget* pWidget,
  bool useInternalFileBrowser)
{
  if (
    (m_smtkView != v) || (m_parentWidget != pWidget) ||
    (m_useInternalFileBrowser != useInternalFileBrowser))
  {
    m_smtkView = v;
    this->initializeUI(pWidget, useInternalFileBrowser);
  }
  return m_topView;
}

void qtUIManager::internalInitialize()
{
  this->findDefinitionsLongLabels();

  // initialize initial advance level
  if (auto attResource = m_attResource.lock())
  {
    const std::map<int, std::string>& levels = attResource->advanceLevels();
    if (!levels.empty())
    {
      // use the minimum enum value as initial advance level
      std::map<int, std::string>::const_iterator ait = levels.begin();
      int minLevel = ait->first;
      ait++;
      for (; ait != levels.end(); ++ait)
      {
        minLevel = std::min(minLevel, ait->first);
      }
      // m_currentAdvLevel can not be lower than the minLevel
      m_currentAdvLevel = std::max(minLevel, m_currentAdvLevel);
    }
  }
}

void qtUIManager::setAdvanceLevel(int b)
{
  if (m_currentAdvLevel == b)
  {
    return;
  }

  m_currentAdvLevel = b;
  if (m_topView)
  {
    m_topView->showAdvanceLevel(b);
    m_topView->configuration()->details().setAttribute(
      m_activeAdvLevelXmlAttName, std::to_string(b));
  }
}

void qtUIManager::initAdvanceLevels(QComboBox* combo)
{
  if (auto attResource = m_attResource.lock())
  {
    combo->blockSignals(true);
    const std::map<int, std::string>& levels = attResource->advanceLevels();
    if (levels.empty())
    {
      // for backward compatibility, we automatically add
      // two levels which is implicitly supported in previous version
      combo->addItem("General", 0);
      combo->addItem("Advanced", 1);

      combo->setCurrentIndex(m_currentAdvLevel);
    }
    else
    {
      std::map<int, std::string>::const_iterator ait;
      for (ait = levels.begin(); ait != levels.end(); ++ait)
      {
        combo->addItem(ait->second.c_str(), ait->first);
        if (m_currentAdvLevel == ait->first)
        {
          combo->setCurrentIndex(combo->count() - 1);
        }
      }
    }
    combo->blockSignals(false);
  }
}

QColor qtUIManager::contrastWithText(const QColor& color)
{
  int textLightness = QGuiApplication::palette().text().color().lightness();
  if (textLightness > 127)
  {
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    l = 255 - l;
    return QColor::fromHsl(h, s, l, a);
  }
  return color;
}

std::string qtUIManager::currentCategory()
{
  return m_topView ? m_topView->currentCategory() : "";
}

bool qtUIManager::categoryEnabled()
{
  return m_topView ? m_topView->categoryEnabled() : false;
}

bool qtUIManager::passAdvancedCheck(int level)
{
  return (level <= this->advanceLevel());
}

QString qtUIManager::clipBoardText()
{
  const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
  return clipboard->text();
}

void qtUIManager::setClipBoardText(QString& text)
{
  QApplication::clipboard()->setText(text);
}

void qtUIManager::clearRoot()
{
  if (m_topView)
  {
    delete m_topView;
    m_topView = nullptr;
  }
}

void qtUIManager::setAdvanceFontStyleBold(bool val)
{
  this->advFont.setBold(val);
}

bool qtUIManager::advanceFontStyleBold() const
{
  return this->advFont.bold();
}

void qtUIManager::setAdvanceFontStyleItalic(bool val)
{
  this->advFont.setItalic(val);
}

bool qtUIManager::advanceFontStyleItalic() const
{
  return this->advFont.italic();
}

void qtUIManager::setDefaultValueColor(const QColor& color)
{
  if (color == this->DefaultValueColor)
  {
    return;
  }
  this->DefaultValueColor = color;
  if (m_topView)
  {
    m_topView->onShowCategory();
  }
}

QVariantList qtUIManager::defaultValueColorRgbF() const
{
  QVariantList val;
  val << this->DefaultValueColor.redF() << this->DefaultValueColor.greenF()
      << this->DefaultValueColor.blueF()
    // << this->DefaultValueColor.alphaF()
    ;
  return val;
}

void qtUIManager::setDefaultValueColorRgbF(const QVariantList& color)
{
  if (color.size() != 3)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Bad color specification.");
    return;
  }
  auto replacement =
    QColor::fromRgbF(color[0].toDouble(), color[1].toDouble(), color[2].toDouble());
  if (replacement == this->DefaultValueColor)
  {
    return;
  }
  this->DefaultValueColor = replacement;
  if (m_topView)
  {
    m_topView->onShowCategory();
  }
}

void qtUIManager::setInvalidValueColor(const QColor& color)
{
  if (color == this->InvalidValueColor)
  {
    return;
  }
  this->InvalidValueColor = color;
  if (m_topView)
  {
    m_topView->onShowCategory();
  }
}

QVariantList qtUIManager::invalidValueColorRgbF() const
{
  QVariantList val;
  val << this->InvalidValueColor.redF() << this->InvalidValueColor.greenF()
      << this->InvalidValueColor.blueF()
    // << this->InvalidValueColor.alphaF()
    ;
  return val;
}

QColor qtUIManager::correctedInvalidValueColor() const
{
  return qtUIManager::contrastWithText(this->InvalidValueColor);
}

QColor qtUIManager::correctedNormalValueColor() const
{
  return qtUIManager::contrastWithText(Qt::white);
}
QColor qtUIManager::correctedDefaultValueColor() const
{
  return qtUIManager::contrastWithText(this->DefaultValueColor);
}

void qtUIManager::setInvalidValueColorRgbF(const QVariantList& color)
{
  if (color.size() != 3)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Bad color specification.");
    return;
  }
  auto replacement =
    QColor::fromRgbF(color[0].toDouble(), color[1].toDouble(), color[2].toDouble());
  if (replacement == this->InvalidValueColor)
  {
    return;
  }
  this->InvalidValueColor = replacement;
  if (m_topView)
  {
    m_topView->onShowCategory();
  }
}

void qtUIManager::setWidgetColorToInvalid(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, this->correctedInvalidValueColor());
  widget->setPalette(pal);
}

void qtUIManager::setWidgetColorToDefault(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, this->correctedDefaultValueColor());
  widget->setPalette(pal);
}

void qtUIManager::setWidgetColorToNormal(QWidget* widget)
{
  QPalette pal = widget->palette();
  pal.setColor(QPalette::Base, this->correctedNormalValueColor());
  widget->setPalette(pal);
}

void qtUIManager::updateArrayTableWidget(
  smtk::attribute::GroupItemPtr dataItem,
  QTableWidget* widget)
{
  widget->clear();
  widget->setRowCount(0);
  widget->setColumnCount(0);

  if (!dataItem)
  {
    return;
  }

  std::size_t n = dataItem->numberOfGroups();
  std::size_t m = dataItem->numberOfItemsPerGroup();
  if (!m || !n)
  {
    return;
  }
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return;
  }

  int numCols = static_cast<int>(n * m), numRows = static_cast<int>(item->numberOfValues());
  widget->setColumnCount(numCols);
  widget->setRowCount(numRows);
  for (int h = 0; h < numCols; h++)
  {
    QTableWidgetItem* qtablewidgetitem = new QTableWidgetItem();
    widget->setHorizontalHeaderItem(h, qtablewidgetitem);
  }
  for (int j = 0; j < numCols; j++) // expecting one item for each column
  {
    qtUIManager::updateTableColRows(dataItem->item(j), j, widget);
  }
}

void qtUIManager::updateTableColRows(
  smtk::attribute::ItemPtr dataItem,
  int col,
  QTableWidget* widget)
{
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem);
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return;
  }
  int numRows = static_cast<int>(item->numberOfValues());
  widget->setRowCount(numRows);
  QString strValue;
  for (int row = 0; row < numRows; row++)
  {
    strValue = item->valueAsString(row).c_str();
    widget->setItem(row, col, new QTableWidgetItem(strValue));
  }
}

void qtUIManager::updateArrayDataValue(
  smtk::attribute::GroupItemPtr dataItem,
  QTableWidgetItem* item)
{
  if (!dataItem)
  {
    return;
  }
  smtk::attribute::DoubleItemPtr dItem =
    dynamic_pointer_cast<DoubleItem>(dataItem->item(item->column()));
  smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(item->column()));
  if (dItem)
  {
    dItem->setValue(item->row(), item->text().toDouble());
  }
  else if (iItem)
  {
    iItem->setValue(item->row(), item->text().toInt());
  }
}

bool qtUIManager::getExpressionArrayString(
  smtk::attribute::GroupItemPtr dataItem,
  QString& strValues)
{
  if (!dataItem || !dataItem->numberOfRequiredGroups())
  {
    return false;
  }
  smtk::attribute::ValueItemPtr item = dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if (!item || item->isDiscrete() || item->isExpression())
  {
    return false;
  }
  int numberOfComponents = static_cast<int>(dataItem->numberOfItemsPerGroup());
  int nVals = static_cast<int>(item->numberOfValues());
  QStringList strVals;
  smtk::attribute::ValueItemPtr valueitem;
  for (int i = 0; i < nVals; i++)
  {
    for (int c = 0; c < numberOfComponents - 1; c++)
    {
      valueitem = dynamic_pointer_cast<ValueItem>(dataItem->item(c));
      strVals << valueitem->valueAsString(i).c_str() << "\t";
    }
    valueitem = dynamic_pointer_cast<ValueItem>(dataItem->item(numberOfComponents - 1));
    strVals << valueitem->valueAsString(i).c_str();
    strVals << LINE_BREAKER_STRING;
  }
  strValues = strVals.join(" ");
  return true;
}

void qtUIManager::removeSelectedTableValues(
  smtk::attribute::GroupItemPtr dataItem,
  QTableWidget* table)
{
  if (!dataItem)
  {
    return;
  }

  int numRows = table->rowCount(), numCols = table->columnCount();
  for (int r = numRows - 1; r >= 0; --r)
  {
    if (table->item(r, 0)->isSelected())
    {
      for (int i = 0; i < numCols; i++)
      {
        smtk::attribute::DoubleItemPtr dItem = dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
        smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(i));
        if (dItem)
        {
          dItem->removeValue(r);
        }
        else if (iItem)
        {
          iItem->removeValue(r);
        }
      }
      table->removeRow(r);
    }
  }
}

void qtUIManager::addNewTableValues(
  smtk::attribute::GroupItemPtr dataItem,
  QTableWidget* table,
  double* vals,
  int numVals)
{
  int numCols = table->columnCount();
  if (!dataItem || numCols != numVals)
  {
    return;
  }
  int totalRow = table->rowCount();
  table->setRowCount(++totalRow);

  for (int i = 0; i < numVals; i++)
  {
    smtk::attribute::DoubleItemPtr dItem = dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
    smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(dataItem->item(i));
    if (dItem)
    {
      dItem->appendValue(vals[i]);
    }
    else if (iItem)
    {
      iItem->appendValue(vals[i]);
    }
    QString strValue = QString::number(vals[i]);
    table->setItem(totalRow - 1, i, new QTableWidgetItem(strValue));
  }
}

void qtUIManager::onFileItemCreated(qtFileItem* fileItem)
{
  if (m_useInternalFileBrowser)
  {
    fileItem->enableFileBrowser();
  }
  else
  {
    Q_EMIT this->fileItemCreated(fileItem);
  }
}

void qtUIManager::onModelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem)
{
  Q_EMIT this->modelEntityItemCreated(entItem);
}

bool qtUIManager::updateTableItemCheckState(
  QTableWidgetItem* labelitem,
  smtk::attribute::ItemPtr attItem)
{
  bool bEnabled = true;
  if (attItem->definition()->isOptional())
  {
    Qt::CheckState checkState = attItem->isEnabled()
      ? Qt::Checked
      : (attItem->definition()->isEnabledByDefault() ? Qt::Checked : Qt::Unchecked);
    labelitem->setCheckState(checkState);
    QVariant vdata;
    vdata.setValue(static_cast<void*>(attItem.get()));
    labelitem->setData(Qt::UserRole, vdata);
    labelitem->setFlags(labelitem->flags() | Qt::ItemIsUserCheckable);
    bEnabled = (checkState == Qt::Checked);
  }
  return bEnabled;
}

void qtUIManager::registerItemConstructor(const std::string& itype, qtItemConstructor f)
{
  m_itemConstructors[itype] = f;
}

qtBaseView* qtUIManager::createView(const smtk::view::Information& info)
{
  if (info.get<smtk::extension::qtUIManager*>() != this)
  {
    // The view being constructed is not referring to this manager!
    return nullptr;
  }

  auto& viewManager = m_managers.get<smtk::view::Manager::Ptr>();
  std::string viewType = info.get<smtk::view::ConfigurationPtr>()->type();
  if (!viewManager)
  {
    std::cerr << "No viewManager for View Type: " << viewType << " skipping view!\n";
    return nullptr;
  }
  qtBaseView* qtView = nullptr;
  if (viewManager->viewWidgetFactory().contains(viewType))
  {
    qtView = dynamic_cast<qtBaseView*>(
      viewManager->viewWidgetFactory()
        .createFromName(viewType, static_cast<const smtk::view::Information&>(info))
        .release());
  }
  else if (viewManager->viewWidgetFactory().containsAlias(viewType))
  {
    qtView = dynamic_cast<qtBaseView*>(
      viewManager->viewWidgetFactory()
        .createFromAlias(viewType, static_cast<const smtk::view::Information&>(info))
        .release());
  }
  if (!qtView)
  {
    // Constructor for that type could not be found)
    std::cerr << "Could not find View Type: " << viewType << " skipping view!\n";
  }
  else
  {
    // Views do not follow RAII. They require a call to the protected virtual
    // method buildUI() to be initialized. Since objects that inherit from
    // qtBaseView are only ever called by qtUIManager, we can code around this
    // issue by making qtBaseView a friend of qtUIManager and by calling buildUI()
    // immediately upon construction in qtUIManager.
    qtView->buildUI();
  }
  return qtView;
}

qtItem* qtUIManager::createItem(const qtAttributeItemInfo& info)
{
  if (info.uiManager() != this)
  {
    // The view being constructed is not refering to this manager!
    return nullptr;
  }

  // If there is a View associated with the item - does it want it
  // displayed?
  auto item = info.item();
  auto* iview = info.baseView();
  if (iview && (!iview->displayItem(item)))
  {
    return nullptr;
  }

  std::map<std::string, qtItemConstructor>::const_iterator it;
  std::string qtItemViewType;
  if (!info.component().attribute("Type", qtItemViewType))
  {
    // There is no type info assume its the default
    qtItemViewType = "Default";
  }

  it = m_itemConstructors.find(qtItemViewType);
  if (it == m_itemConstructors.end())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not create an ItemView of type: " << qtItemViewType << "- Using a Default Item View.");
    // OK - lets create a default view for the item

    it = m_itemConstructors.find("Default");
    if (it == m_itemConstructors.end())
    {
      return nullptr;
    }
  }

  qtItem* qitem = (it->second)(info);
  return qitem;
}

void qtUIManager::onViewUIModified(
  smtk::extension::qtBaseView* bview,
  smtk::attribute::ItemPtr item)
{
  Q_EMIT this->viewUIChanged(bview, item);
}

void qtUIManager::onOperationFinished()
{
  Q_EMIT this->refreshEntityItems();
}

int qtUIManager::getWidthOfAttributeMaxLabel(smtk::attribute::DefinitionPtr def, const QFont& font)
{
  std::string text;
  if (this->Def2LongLabel.contains(def))
  {
    text = this->Def2LongLabel[def];
  }
  else
  {
    this->findDefinitionLongLabel(def, text);
  }

  this->Def2LongLabel[def] = text;
  QFontMetrics fontsize(font);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  return fontsize.horizontalAdvance(text.c_str());
#else
  return fontsize.width(text.c_str());
#endif
}

void qtUIManager::findDefinitionLongLabel(
  smtk::attribute::DefinitionPtr def,
  std::string& labelText)
{
  QList<smtk::attribute::ItemDefinitionPtr> itemDefs;
  int i, n = static_cast<int>(def->numberOfItemDefinitions());
  for (i = 0; i < n; i++)
  {
    itemDefs.push_back(def->itemDefinition(i));
  }

  this->getItemsLongLabel(itemDefs, labelText);
}

void qtUIManager::getItemsLongLabel(
  const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
  std::string& labelText)
{
  bool hasOptionalItem = false;
  Q_FOREACH (smtk::attribute::ItemDefinitionPtr itDef, itemDefs)
  {
    smtk::attribute::Item::Type itType = itDef->type();
    // GROUP and VOID type uses their own label length
    if (itType == Item::GroupType || itType == Item::VoidType)
    {
      continue;
    }
    std::string text = itDef->label().empty() ? itDef->name() : itDef->label();
    if (itDef->isOptional())
    {
      hasOptionalItem = true;
    }
    labelText = (text.length() > labelText.length()) ? text : labelText;
  }

  // Add spaces to compensate checkbox width and some spacing.
  labelText += (hasOptionalItem ? "     " : " ");
}

int qtUIManager::getWidthOfItemsMaxLabel(
  const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
  const QFont& font)
{
  std::string text;
  this->getItemsLongLabel(itemDefs, text);
  QFontMetrics fontsize(font);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  return fontsize.horizontalAdvance(text.c_str());
#else
  return fontsize.width(text.c_str());
#endif
}

int qtUIManager::getWidthOfText(const std::string& text, const QFont& font)
{
  QFontMetrics fontsize(font);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  return fontsize.horizontalAdvance(text.c_str());
#else
  return fontsize.width(text.c_str());
#endif
}

void qtUIManager::findDefinitionsLongLabels()
{
  this->Def2LongLabel.clear();
  if (auto attResource = m_attResource.lock())
  {
    // Generate list of all concrete definitions in the manager
    std::vector<smtk::attribute::DefinitionPtr> defs;
    std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
    attResource->findBaseDefinitions(baseDefinitions);
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;

    for (baseIter = baseDefinitions.begin(); baseIter != baseDefinitions.end(); baseIter++)
    {
      std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
      attResource->findAllDerivedDefinitions(*baseIter, true, derivedDefs);
      defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
    }

    std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
    for (defIter = defs.begin(); defIter != defs.end(); defIter++)
    {
      std::string text;
      this->findDefinitionLongLabel(*defIter, text);
      this->Def2LongLabel[*defIter] = text;
    }
  }
}

qtItem* qtUIManager::defaultItemConstructor(const qtAttributeItemInfo& info)
{
  auto item = info.item();
  qtItem* aItem = nullptr;
  switch (item->type())
  {
    case smtk::attribute::Item::ComponentType:
      return qtComponentItem::createItemWidget(info);
    case smtk::attribute::Item::DateTimeType:
      return qtDateTimeItem::createItemWidget(info);
    case smtk::attribute::Item::DirectoryType:
      return qtDirectoryItem::createItemWidget(info);
    case smtk::attribute::Item::DoubleType:
      return qtDoubleItem::createItemWidget(info);
    case smtk::attribute::Item::FileType:
      return qtFileItem::createItemWidget(info);
    case smtk::attribute::Item::GroupType:
      return qtGroupItem::createItemWidget(info);
    case smtk::attribute::Item::IntType:
      return qtIntItem::createItemWidget(info);
    case smtk::attribute::Item::ReferenceType:
      return qtReferenceItem::createItemWidget(info);
    case smtk::attribute::Item::ResourceType:
      return qtResourceItem::createItemWidget(info);
    case smtk::attribute::Item::StringType:
      return qtStringItem::createItemWidget(info);
    case smtk::attribute::Item::VoidType:
      return qtVoidItem::createItemWidget(info);
    default:
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Error: Unsupported Item Type: " << smtk::attribute::Item::type2String(item->type()));
  }
  return aItem;
}

void qtUIManager::setHighlightOnHover(bool val)
{
  if (val == m_highlightOnHover)
  {
    return;
  }
  m_highlightOnHover = val;
  Q_EMIT highlightOnHoverChanged(val);
}

const smtk::view::Configuration::Component& qtUIManager::findStyle(
  const smtk::attribute::DefinitionPtr& def,
  const std::string& styleName) const
{
  static smtk::view::Configuration::Component emptyStyle;
  auto attRes = m_attResource.lock();
  if (attRes == nullptr)
  {
    return emptyStyle;
  }
  return attRes->findStyle(def, styleName);
}
