/*=========================================================================

  Module:    qtUIManager.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "smtk/Qt/qtUIManager.h"

#include "smtk/Qt/qtItem.h"
#include "smtk/Qt/qtComboItem.h"
#include "smtk/Qt/qtFileItem.h"
#include "smtk/Qt/qtGroupView.h"
#include "smtk/Qt/qtRootView.h"
#include "smtk/Qt/qtInputsItem.h"
#include "smtk/Qt/qtAttributeView.h"
#include "smtk/Qt/qtInstancedView.h"
#include "smtk/Qt/qtModelEntityView.h"
#include "smtk/Qt/qtSimpleExpressionView.h"

#include <QTableWidget>
#include <QLayout>
#include <QMimeData>
#include <QClipboard>
#include <QSpinBox>
#include <QLineEdit>
#include <QApplication>
#include <QComboBox>
#include <QStringList>
#include <QIntValidator>
#include <QTextEdit>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/view/Root.h"
#include "smtk/view/Attribute.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/ModelEntity.h"
#include "smtk/view/SimpleExpression.h"

#include <math.h>

#if defined(_WIN32) //VS2008 is not c99 complient.
#include <float.h>
double nextafter(double x, double y)
{
  return _nextafter(x,y);
}
#endif

using namespace smtk::attribute;

#define SB_DOUBLE_CONSTRAINT_PRECISION 0.000001

//-----------------------------------------------------------------------------
qtUIManager* qtUIManager::Instance = 0;

//-----------------------------------------------------------------------------
qtDoubleValidator::qtDoubleValidator(QObject * inParent)
  :QDoubleValidator(inParent)
{
}

//-----------------------------------------------------------------------------
void qtDoubleValidator::fixup(QString &input) const
{
  if ( input.length() == 0 )
    {
    return;
    }

  QLineEdit* editBox =
    static_cast<QLineEdit*>(this->property("MyWidget").value<void *>());

  double v = input.toDouble();
  bool outofrange = false;
  if (v < this->bottom())
    {
    input = QString::number(this->bottom()+smtk_DOUBLE_CONSTRAINT_PRECISION);
    outofrange = true;
    }
  else if (v > this->top())
    {
    input = QString::number(this->top()-smtk_DOUBLE_CONSTRAINT_PRECISION);
    outofrange = true;
    }
  if(editBox && outofrange)
    {
    QPalette pal = editBox->palette();
    pal.setColor(QPalette::Base, qtUIManager::instance()->invalidValueColor());
    editBox->setPalette(pal);
    }
}

//-----------------------------------------------------------------------------
qtUIManager* qtUIManager::instance()
{
  return qtUIManager::Instance;
}

//----------------------------------------------------------------------------
qtUIManager::qtUIManager(smtk::attribute::Manager &manager) :
  m_AttManager(manager)
{
  if (!qtUIManager::Instance)
    {
    qtUIManager::Instance = this;
    }
  this->RootView = NULL;

  if(manager.rootView())
    {
    this->advFont.setBold(manager.rootView()->advancedBold());
    this->advFont.setItalic(manager.rootView()->advancedItalic());
    const double* rgba = manager.rootView()->defaultColor();
    this->DefaultValueColor.setRgbF(rgba[0], rgba[1], rgba[2]);
    rgba = manager.rootView()->invalidColor();
    this->InvalidValueColor.setRgbF(rgba[0], rgba[1], rgba[2]);
    }
  else
    { // default settings
    this->advFont.setBold(true);
    this->advFont.setItalic(false);
    this->DefaultValueColor.setRgbF(1.0, 1.0, 0.5);
    this->InvalidValueColor.setRgbF(1.0, 0.5, 0.5);
    }
}

//----------------------------------------------------------------------------
qtUIManager::~qtUIManager()
{
  if(this->RootView)
    {
    delete this->RootView;
    }
  if (qtUIManager::Instance == this)
    {
    qtUIManager::Instance = 0;
    }
}

//----------------------------------------------------------------------------
void qtUIManager::initializeUI(QWidget* pWidget)
{
  if(!this->m_AttManager.rootView())
    {
    return;
    }
  if(this->RootView)
    {
    delete this->RootView;
    }
  smtk::view::RootPtr rs = this->m_AttManager.rootView();
  const double *dcolor = rs->defaultColor();
  this->DefaultValueColor.setRgbF(dcolor[0], dcolor[1], dcolor[2], dcolor[3]);
  dcolor = rs->invalidColor();
  this->InvalidValueColor.setRgbF(dcolor[0], dcolor[1], dcolor[2], dcolor[3]);

  this->RootView = new qtRootView(
    this->m_AttManager.rootView(), pWidget);
}

//----------------------------------------------------------------------------
void qtUIManager::updateModelViews()
{
  if(!this->RootView)
    {
    return;
    }
  foreach(qtBaseView* childView, this->RootView->getRootGroup()->childViews())
    {
    if(childView->getObject()->type() == smtk::view::Base::ATTRIBUTE ||
       childView->getObject()->type() == smtk::view::Base::MODEL_ENTITY)
      {
      childView->updateModelAssociation();
      }
    }
}

//----------------------------------------------------------------------------
std::string qtUIManager::currentCategory()
{
  return this->RootView ? this->RootView->currentCategory() : "";
}
//----------------------------------------------------------------------------
bool qtUIManager::categoryEnabled()
{
  return this->RootView ? this->RootView->categoryEnabled() : false;
}
//----------------------------------------------------------------------------
bool qtUIManager::passAdvancedCheck(bool advanced)
{
  return (!advanced || this->RootView == NULL ||
    advanced == this->RootView->showAdvanced());
}
//----------------------------------------------------------------------------
bool qtUIManager::passAttributeCategoryCheck(
  smtk::attribute::ConstDefinitionPtr AttDef)
{
  return this->passCategoryCheck(AttDef->categories());
}
//----------------------------------------------------------------------------
bool qtUIManager::passItemCategoryCheck(
  smtk::attribute::ConstItemDefinitionPtr ItemDef)
{
  return this->passCategoryCheck(ItemDef->categories());
}

//----------------------------------------------------------------------------
bool qtUIManager::passCategoryCheck(const std::set<std::string> & categories)
{
  return !this->categoryEnabled() ||
    categories.find(this->currentCategory()) != categories.end();
}
//----------------------------------------------------------------------------
void qtUIManager::processAttributeView(qtAttributeView* qtView)
{
  smtk::view::AttributePtr v = smtk::dynamic_pointer_cast<smtk::view::Attribute>(
    qtView->getObject());

  qtUIManager::processBasicView(qtView);
}
//----------------------------------------------------------------------------
void qtUIManager::processInstancedView(qtInstancedView* qtView)
{
  smtk::view::InstancedPtr v = smtk::dynamic_pointer_cast<smtk::view::Instanced>(
    qtView->getObject());

  qtUIManager::processBasicView(qtView);
}
//----------------------------------------------------------------------------
void qtUIManager::processModelEntityView(qtModelEntityView* qtView)
{
  smtk::view::ModelEntityPtr v = smtk::dynamic_pointer_cast<smtk::view::ModelEntity>(
    qtView->getObject());

  qtUIManager::processBasicView(qtView);
}
//----------------------------------------------------------------------------
void qtUIManager::processSimpleExpressionView(qtSimpleExpressionView* qtView)
{
  smtk::view::SimpleExpressionPtr v = smtk::dynamic_pointer_cast<smtk::view::SimpleExpression>(
    qtView->getObject());

  qtUIManager::processBasicView(qtView);
}
//----------------------------------------------------------------------------
void qtUIManager::processGroupView(qtGroupView* pQtGroup)
{
  smtk::view::GroupPtr group = smtk::dynamic_pointer_cast<smtk::view::Group>(
    pQtGroup->getObject());
  qtUIManager::processBasicView( pQtGroup);
  std::size_t i, n = group->numberOfSubViews();
  smtk::view::BasePtr v;
  qtBaseView* qtView = NULL;
  for (i = 0; i < n; i++)
    {
    v = group->subView(i);
    switch(v->type())
      {
      case smtk::view::Base::ATTRIBUTE:
        qtView = new qtAttributeView(v, pQtGroup->widget());
        qtUIManager::processAttributeView(qobject_cast<qtAttributeView*>(qtView));
        break;
      case smtk::view::Base::GROUP:
        qtView = new qtGroupView(v, pQtGroup->widget());
        qtUIManager::processGroupView(qobject_cast<qtGroupView*>(qtView));
        break;
      case smtk::view::Base::INSTANCED:
        qtView = new qtInstancedView(v, pQtGroup->widget());
        qtUIManager::processInstancedView(qobject_cast<qtInstancedView*>(qtView));
        break;
      case smtk::view::Base::MODEL_ENTITY:
        qtView = new qtModelEntityView(v, pQtGroup->widget());
        qtUIManager::processModelEntityView(qobject_cast<qtModelEntityView*>(qtView));
        break;
      case smtk::view::Base::SIMPLE_EXPRESSION:
        qtView = new qtSimpleExpressionView(v, pQtGroup->widget());
        qtUIManager::processSimpleExpressionView(qobject_cast<qtSimpleExpressionView*>(qtView));
        break;
      default:
        break;
        //this->m_errorStatus << "Unsupport View Type "
        //                    << View::type2String(sec->type()) << "\n";
      }
    if(qtView)
      {
      pQtGroup->addChildView(qtView);
      }
    }
}

//----------------------------------------------------------------------------
void qtUIManager::processBasicView(qtBaseView* /*v*/)
{
  //node.append_attribute("Title").set_value(v->title().c_str());
  //if (v->iconName() != "")
  //  {
  //  node.append_attribute("Icon").set_value(v->title().c_str());
  //  }
}

//----------------------------------------------------------------------------
QString qtUIManager::clipBoardText()
{
  const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
  return clipboard->text();
}

//----------------------------------------------------------------------------
void qtUIManager::setClipBoardText(QString& text)
{
  QApplication::clipboard()->setText(text);
}

//----------------------------------------------------------------------------
void qtUIManager::clearRoot()
{
  if(this->RootView)
    {
    delete this->RootView;
    this->RootView = NULL;
    }
}

//----------------------------------------------------------------------------
void qtUIManager::setInvalidValueColor(const QColor &color)
{
  this->InvalidValueColor = color;
}

//----------------------------------------------------------------------------
void qtUIManager::setWidgetToDefaultValueColor(QWidget *widget,
                                                       bool setToDefault)
{
  if (setToDefault)
    {
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Base, this->DefaultValueColor);
    widget->setPalette(pal);
    }
  else
    {
    widget->setPalette(widget->parentWidget()->palette());
    }
}

//----------------------------------------------------------------------------
void qtUIManager::updateArrayTableWidget(
  smtk::attribute::GroupItemPtr dataItem, QTableWidget* widget)
{
  widget->clear();
  widget->setRowCount(0);
  widget->setColumnCount(0);

  if(!dataItem)
    {
    return;
    }

  std::size_t n = dataItem->numberOfGroups();
  std::size_t m = dataItem->numberOfItemsPerGroup();
  if(!m  || !n)
    {
    return;
    }
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return;
    }

  int numCols = static_cast<int>(n*m), numRows = static_cast<int>(item->numberOfValues());
  widget->setColumnCount(numCols);
  widget->setRowCount(numRows);
  for(int h=0; h<numCols; h++)
    {
    QTableWidgetItem *qtablewidgetitem = new QTableWidgetItem();
    widget->setHorizontalHeaderItem(h, qtablewidgetitem);
    }
  for (int j = 0; j < numCols; j++) // expecting one item for each column
    {
    qtUIManager::updateTableColRows(dataItem->item(j), j, widget);
    }
}

//----------------------------------------------------------------------------
void qtUIManager::updateTableColRows(smtk::attribute::ItemPtr dataItem,
    int col, QTableWidget* widget)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(dataItem);
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return;
    }
  int numRows = static_cast<int>(item->numberOfValues());
  widget->setRowCount(numRows);
  QString strValue;
  for(int row=0; row < numRows; row++)
    {
    strValue = item->valueAsString(row).c_str();
    widget->setItem(row, col, new QTableWidgetItem(strValue));
    }
}

//----------------------------------------------------------------------------
void qtUIManager::updateArrayDataValue(
  smtk::attribute::GroupItemPtr dataItem, QTableWidgetItem* item)
{
  if(!dataItem)
    {
    return;
    }
  smtk::attribute::DoubleItemPtr dItem =dynamic_pointer_cast<DoubleItem>(
    dataItem->item(item->column()));
  smtk::attribute::IntItemPtr iItem =dynamic_pointer_cast<IntItem>(
    dataItem->item(item->column()));
  if(dItem)
    {
    dItem->setValue(item->row(), item->text().toDouble());
    }
  else if(iItem)
    {
    iItem->setValue(item->row(), item->text().toInt());
    }
}

//----------------------------------------------------------------------------
bool qtUIManager::getExpressionArrayString(
  smtk::attribute::GroupItemPtr dataItem, QString& strValues)
{
  if(!dataItem || !dataItem->numberOfRequiredGroups())
    {
    return false;
    }
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(dataItem->item(0));
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return false;
    }
  int numberOfComponents = dataItem->numberOfItemsPerGroup();
  int nVals = static_cast<int>(item->numberOfValues());
  QStringList strVals;
  smtk::attribute::ValueItemPtr valueitem;
  for(int i=0; i < nVals; i++)
    {
    for(int c=0; c<numberOfComponents-1; c++)
      {
      valueitem =dynamic_pointer_cast<ValueItem>(dataItem->item(c));
      strVals << valueitem->valueAsString(i).c_str() <<"\t";
      }
    valueitem =dynamic_pointer_cast<ValueItem>(dataItem->item(numberOfComponents-1));
    strVals << valueitem->valueAsString(i).c_str();
    strVals << LINE_BREAKER_STRING;
    }
  strValues = strVals.join(" ");
  return true;
}

//----------------------------------------------------------------------------
void qtUIManager::removeSelectedTableValues(
  smtk::attribute::GroupItemPtr dataItem, QTableWidget* table)
{
  if(!dataItem)
    {
    return;
    }

  int numRows = table->rowCount(), numCols = table->columnCount();
  for(int r=numRows-1; r>=0; --r)
    {
    if(table->item(r, 0)->isSelected())
      {
      for(int i = 0; i < numCols; i++)
        {
        smtk::attribute::DoubleItemPtr dItem =dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
        smtk::attribute::IntItemPtr iItem =dynamic_pointer_cast<IntItem>(dataItem->item(i));
        if(dItem)
          {
          dItem->removeValue(r);
          }
        else if(iItem)
          {
          iItem->removeValue(r);
          }
        }
      table->removeRow(r);
      }
    }
}

//----------------------------------------------------------------------------
void qtUIManager::addNewTableValues(smtk::attribute::GroupItemPtr dataItem,
  QTableWidget* table, double* vals, int numVals)
{
  int numCols = table->columnCount();
  if(!dataItem || numCols != numVals)
    {
    return;
    }
  int totalRow = table->rowCount();
  table->setRowCount(++totalRow);

  for(int i=0; i<numVals; i++)
    {
    smtk::attribute::DoubleItemPtr dItem =dynamic_pointer_cast<DoubleItem>(dataItem->item(i));
    smtk::attribute::IntItemPtr iItem =dynamic_pointer_cast<IntItem>(dataItem->item(i));
    if(dItem)
      {
      dItem->appendValue(vals[i]);
      }
    else if(iItem)
      {
      iItem->appendValue(vals[i]);
      }
    QString strValue = QString::number(vals[i]);
    table->setItem(totalRow-1, i, new QTableWidgetItem(strValue));
    }
}
//----------------------------------------------------------------------------
void qtUIManager::onFileItemCreated(qtFileItem* fileItem)
{
  emit this->fileItemCreated(fileItem);
}

//----------------------------------------------------------------------------
QWidget* qtUIManager::createExpressionRefWidget(
  smtk::attribute::ItemPtr attitem, int elementIdx, QWidget* pWidget)
{
  smtk::attribute::ValueItemPtr inputitem =dynamic_pointer_cast<ValueItem>(attitem);
  if(!inputitem)
    {
    return NULL;
    }
  smtk::attribute::RefItemPtr item =inputitem->expressionReference(elementIdx);
  if(!item)
    {
    return NULL;
    }

  const RefItemDefinition *itemDef =
    dynamic_cast<const RefItemDefinition*>(item->definition().get());
  smtk::attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if(!attDef)
    {
    return NULL;
    }
  QList<QString> attNames;
  std::vector<smtk::attribute::AttributePtr> result;
  Manager *lAttManager = attDef->manager();
  lAttManager->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    attNames.push_back((*it)->name().c_str());
    }

  QComboBox* combo = new QComboBox(pWidget);
  QVariant vdata(elementIdx);
  combo->setProperty("ElementIndex", vdata);
  QVariant vobject;
  vobject.setValue(static_cast<void*>(attitem.get()));
  combo->setProperty("AttItemObj", vobject);
  combo->addItems(attNames);

  int setIndex = -1;
  if (item->isSet(elementIdx))
    {
    setIndex = attNames.indexOf(item->valueAsString(elementIdx).c_str());
    }
  combo->setCurrentIndex(setIndex);

  QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onExpressionReferenceChanged()), Qt::QueuedConnection);

  return combo;
}
//----------------------------------------------------------------------------
void qtUIManager::onExpressionReferenceChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();
  ValueItem* inputitem =static_cast<ValueItem*>(
    comboBox->property("AttItemObj").value<void *>());
  if(!inputitem)
    {
    return;
    }
  smtk::attribute::RefItemPtr item =inputitem->expressionReference(elementIdx);
  if(!item)
    {
    return;
    }

  if(curIdx>=0)
    {
    Manager *lAttManager = item->attribute()->manager();
    AttributePtr attPtr = lAttManager->findAttribute(comboBox->currentText().toStdString());
    if(attPtr)
      {
      inputitem->setExpression(elementIdx, attPtr);
      }
    else
      {
      item->unset(elementIdx);
      inputitem->unset(elementIdx);
      }
    }
  else
    {
    item->unset(elementIdx);
    inputitem->unset(elementIdx);
    }
}

//----------------------------------------------------------------------------
QWidget* qtUIManager::createComboBox(
  smtk::attribute::ItemPtr attitem, int elementIdx, QWidget* pWidget)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(attitem);
  if(!item)
    {
    return NULL;
    }
  const ValueItemDefinition *itemDef =
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());

  QList<QString> discreteVals;
  QString tooltip;
  for (size_t i = 0; i < itemDef->numberOfDiscreteValues(); i++)
    {
    std::string enumText = itemDef->discreteEnum(i);
    if(itemDef->hasDefault() &&
      static_cast<size_t>(itemDef->defaultDiscreteIndex()) == i)
      {
      tooltip = "Default: " + QString(enumText.c_str());
      }
    discreteVals.push_back(enumText.c_str());
    }

  QComboBox* combo = new QComboBox(pWidget);
  if(!tooltip.isEmpty())
    {
    combo->setToolTip(tooltip);
    }
  QVariant vdata(elementIdx);
  combo->setProperty("ElementIndex", vdata);
  QVariant vobject;
  vobject.setValue(static_cast<void*>(attitem.get()));
  combo->setProperty("AttItemObj", vobject);
  combo->addItems(discreteVals);
  int setIndex = -1;
  if (item->isSet(elementIdx))
    {
    setIndex = item->discreteIndex(elementIdx);
    }
  if(setIndex < 0 && itemDef->hasDefault() &&
    itemDef->defaultDiscreteIndex() < combo->count())
    {
    setIndex = itemDef->defaultDiscreteIndex();
    }
  combo->setCurrentIndex(setIndex);

  QObject::connect(combo,  SIGNAL(currentIndexChanged(int)),
    this, SLOT(onComboIndexChanged()), Qt::QueuedConnection);

  return combo;
}

//----------------------------------------------------------------------------
void qtUIManager::onComboIndexChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(
    QObject::sender());
  if(!comboBox)
    {
    return;
    }
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();
  ValueItem* item =static_cast<ValueItem*>(
    comboBox->property("AttItemObj").value<void *>());
  if(!item)
    {
    return;
    }
  if(curIdx>=0)
    {
    item->setDiscreteIndex(elementIdx, curIdx);
    }
  else
    {
    item->unset(elementIdx);
    }
}
//----------------------------------------------------------------------------
QWidget* qtUIManager::createInputWidget(
  smtk::attribute::ItemPtr attitem,int elementIdx,QWidget* pWidget)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(attitem);
  if(!item)
    {
    return NULL;
    }

  return (item->allowsExpressions() /*&& item->isExpression(elementIdx)*/) ?
    this->createExpressionRefWidget(item,elementIdx,pWidget) :
    (item->isDiscrete() ?
      this->createComboBox(item,elementIdx,pWidget) :
      this->createEditBox(item,elementIdx,pWidget));
}
//----------------------------------------------------------------------------
QWidget* qtUIManager::createEditBox(
  smtk::attribute::ItemPtr attitem,int elementIdx,QWidget* pWidget)
{
  smtk::attribute::ValueItemPtr item =dynamic_pointer_cast<ValueItem>(attitem);
  if(!item)
    {
    return NULL;
    }

  QWidget* inputWidget = NULL;
  QVariant vdata(elementIdx);
  bool isDefault = false;
  switch (item->type())
    {
    case smtk::attribute::Item::DOUBLE:
      {
      QLineEdit* editBox = new QLineEdit(pWidget);
      const DoubleItemDefinition *dDef =
        dynamic_cast<const DoubleItemDefinition*>(item->definition().get());
      qtDoubleValidator *validator = new qtDoubleValidator(pWidget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);
      QString tooltip;
      double value=smtk_DOUBLE_MIN;
      if(dDef->hasMinRange())
        {
        value = dDef->minRange();
        if(dDef->minRangeInclusive())
          {
          double multiplier = value >= 0 ? 1. : -1.;
          double to = multiplier*value*1.001+1;
          value = nextafter(value, to);
          }
        validator->setBottom(value);
        QString inclusive = dDef->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
        tooltip.append("Min(").append(inclusive).append("): ").append(QString::number(dDef->minRange()));
        }
      value=smtk_DOUBLE_MAX;
      if(dDef->hasMaxRange())
        {
        value = dDef->maxRange();
        if(dDef->maxRangeInclusive())
          {
          double multiplier = value >= 0 ? -1. : 1.;
          double to = multiplier*value*1.001-1;
          value = nextafter(value, to);
          }
        value = dDef->maxRangeInclusive() ?
          dDef->maxRange() : dDef->maxRange() - smtk_DOUBLE_CONSTRAINT_PRECISION;
        validator->setTop(value);
        if(!tooltip.isEmpty())
          {
          tooltip.append("; ");
          }
        QString inclusive = dDef->maxRangeInclusive() ? "Inclusive" : "Not Inclusive";
        tooltip.append("Max(").append(inclusive).append("): ").append(QString::number(dDef->maxRange()));
        }
      if(dDef->hasDefault())
        {
        value = dDef->defaultValue();
        if(!tooltip.isEmpty())
          {
          tooltip.append("; ");
          }
        tooltip.append("Default: ").append(QString::number(value));
        }

      smtk::attribute::DoubleItemPtr ditem =dynamic_pointer_cast<DoubleItem>(item);
      if(item->isSet(elementIdx))
        {
        editBox->setText(item->valueAsString(elementIdx).c_str());

        isDefault = dDef->hasDefault() &&
          dDef->defaultValue()==ditem->value(elementIdx);
        }
      else if(dDef->hasDefault())
        {
        editBox->setText(QString::number(dDef->defaultValue()));
        isDefault = true;
        }
      if(!tooltip.isEmpty())
        {
        editBox->setToolTip(tooltip);
        }
      QVariant tvdata;
      tvdata.setValue(static_cast<void*>(editBox));
      validator->setProperty("MyWidget", tvdata);
      inputWidget = editBox;
      break;
      }
    case smtk::attribute::Item::INT:
      {
      QLineEdit* editBox = new QLineEdit(pWidget);
      const IntItemDefinition *iDef =
        dynamic_cast<const IntItemDefinition*>(item->definition().get());
      QIntValidator *validator = new QIntValidator(pWidget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);
      QString tooltip;

      int value=smtk_INT_MIN;
      if(iDef->hasMinRange())
        {
        value = iDef->minRangeInclusive() ?
          iDef->minRange() : iDef->minRange() + 1;
        validator->setBottom(value);
        QString inclusive = iDef->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
        tooltip.append("Min(").append(inclusive).append("): ").append(QString::number(iDef->minRange()));
        }
      value=smtk_INT_MAX;
      if(iDef->hasMaxRange())
        {
        value = iDef->maxRangeInclusive() ?
          iDef->maxRange() : iDef->maxRange() - 1;
        validator->setTop(value);
        if(!tooltip.isEmpty())
          {
          tooltip.append("; ");
          }
        QString inclusive = iDef->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
        tooltip.append("Max(").append(inclusive).append("): ").append(QString::number(iDef->maxRange()));
        }
      if(iDef->hasDefault())
        {
        value = iDef->defaultValue();
        if(!tooltip.isEmpty())
          {
          tooltip.append("; ");
          }
        tooltip.append("Default: ").append(QString::number(value));
        }

      smtk::attribute::IntItemPtr iItem =dynamic_pointer_cast<IntItem>(item);
      if(item->isSet(elementIdx))
        {
        editBox->setText(item->valueAsString(elementIdx).c_str());

        isDefault = iDef->hasDefault() &&
          iDef->defaultValue()==iItem->value(elementIdx);
        }
      else if(iDef->hasDefault())
        {
        editBox->setText(QString::number(iDef->defaultValue()));
        isDefault = true;
        }
      if(!tooltip.isEmpty())
        {
        editBox->setToolTip(tooltip);
        }
      QVariant tvdata;
      tvdata.setValue(static_cast<void*>(editBox));
      validator->setProperty("MyWidget", tvdata);
      inputWidget = editBox;
      break;
      }
    case smtk::attribute::Item::STRING:
      {
      const StringItemDefinition *sDef =
        dynamic_cast<const StringItemDefinition*>(item->definition().get());
      smtk::attribute::StringItemPtr sitem =dynamic_pointer_cast<StringItem>(item);
      QString valText;
      if(item->isSet(elementIdx))
        {
        valText = item->valueAsString(elementIdx).c_str();
        isDefault = sDef->hasDefault() &&
          sDef->defaultValue()==sitem->value(elementIdx);
        }
      else if(sDef->hasDefault())
        {
        valText = sDef->defaultValue().c_str();
        isDefault = true;
        }

      if(sDef->isMultiline())
        {
        QTextEdit* textEdit = new QTextEdit(pWidget);
        textEdit->setPlainText(valText);
        QObject::connect(textEdit, SIGNAL(textChanged()),
          this, SLOT(onInputValueChanged()));
        inputWidget = textEdit;
        }
      else
        {
        QLineEdit* lineEdit = new QLineEdit(pWidget);
        lineEdit->setText(valText);
        inputWidget = lineEdit;
        }
      inputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      if(sDef->hasDefault())
        {
        QString tooltip;
        tooltip.append("Default: ").append(sDef->defaultValue().c_str());
        inputWidget->setToolTip(tooltip);
       }
     break;
      }
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  if(inputWidget)
    {
    inputWidget->setProperty("ElementIndex", vdata);
    QVariant vobject;
    vobject.setValue(static_cast<void*>(attitem.get()));
    inputWidget->setProperty("AttItemObj", vobject);

    qtUIManager::instance()->setWidgetToDefaultValueColor(inputWidget,isDefault);
    }
  if(QLineEdit* const editBox = qobject_cast<QLineEdit*>(inputWidget))
    {
    QObject::connect(editBox, SIGNAL(editingFinished()),
      this, SLOT(onInputValueChanged()), Qt::QueuedConnection);
    }

  return inputWidget;
}
//----------------------------------------------------------------------------
void qtUIManager::onInputValueChanged()
{
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(
    QObject::sender());
  QTextEdit* const textBox = qobject_cast<QTextEdit*>(
    QObject::sender());
  if(!editBox && !textBox)
    {
    return;
    }
  QWidget* inputBox;
  if(editBox!=NULL)
    {
    inputBox = editBox;
    }
  else
    {
    inputBox = textBox;
    }
  ValueItem* rawitem =static_cast<ValueItem*>(
    inputBox->property("AttItemObj").value<void *>());
  if(!rawitem)
    {
    return;
    }
  //smtk::attribute::ValueItemPtr item(rawitem);
  //smtk::attribute::DoubleItemPtr dItem =dynamic_pointer_cast<DoubleItem>(item);
  //smtk::attribute::IntItemPtr iItem =dynamic_pointer_cast<IntItem>(item);
  //smtk::attribute::StringItemPtr sItem =dynamic_pointer_cast<StringItem>(item);
  int elementIdx = editBox ? editBox->property("ElementIndex").toInt() :
    textBox->property("ElementIndex").toInt();

  if(editBox && !editBox->text().isEmpty())
    {
    if(rawitem->type()==smtk::attribute::Item::DOUBLE)
      {
      smtk::dynamic_pointer_cast<DoubleItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toDouble());
      }
    else if(rawitem->type()==smtk::attribute::Item::INT)
      {
      smtk::dynamic_pointer_cast<IntItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toInt());
      }
    else if(rawitem->type()==smtk::attribute::Item::STRING)
      {
      smtk::dynamic_pointer_cast<StringItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toStdString());
      }
    else
      {
      rawitem->unset(elementIdx);
      }
    }
  else if(textBox && !textBox->toPlainText().isEmpty() &&
     rawitem->type()==smtk::attribute::Item::STRING)
    {
    smtk::dynamic_pointer_cast<StringItem>(rawitem->pointer())
      ->setValue(elementIdx, textBox->toPlainText().toStdString());
    }
  else
    {
    rawitem->unset(elementIdx);
    }
}
//----------------------------------------------------------------------------
bool qtUIManager::updateTableItemCheckState(
  QTableWidgetItem* labelitem, smtk::attribute::ItemPtr attItem)
{
  bool bEnabled = true;
  if(attItem->definition()->isOptional())
    {
    Qt::CheckState checkState = attItem->isEnabled() ? Qt::Checked :
     (attItem->definition()->isEnabledByDefault() ? Qt::Checked : Qt::Unchecked);
    labelitem->setCheckState(checkState);
    QVariant vdata;
    vdata.setValue(static_cast<void*>(attItem.get()));
    labelitem->setData(Qt::UserRole, vdata);
    labelitem->setFlags(labelitem->flags() | Qt::ItemIsUserCheckable);
    bEnabled = (checkState==Qt::Checked);
    }
  return bEnabled;
}
