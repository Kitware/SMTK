/*=========================================================================

  Module:    qtUIManager.cxx,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "qtUIManager.h"

#include "qtItem.h"
#include "qtComboItem.h"
#include "qtFileItem.h"
#include "qtGroupSection.h"
#include "qtRootSection.h"
#include "qtInputsItem.h"
#include "qtAttributeSection.h"
#include "qtInstancedSection.h"
#include "qtModelEntitySection.h"
#include "qtSimpleExpressionSection.h"

#include <QTableWidget>
#include <QLayout>
#include <QMimeData>
#include <QClipboard>
#include <QSpinBox>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QApplication>
#include <QComboBox>
#include <QStringList>
#include <QIntValidator>
#include <QTextEdit>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>

#include "attribute/RootSection.h"
#include "attribute/AttributeSection.h"
#include "attribute/InstancedSection.h"
#include "attribute/ModelEntitySection.h"
#include "attribute/SimpleExpressionSection.h"
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/Manager.h"
#include "attribute/AttributeRefItem.h"
#include "attribute/AttributeRefItemDefinition.h"
#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"

using namespace slctk::attribute;

#define SB_DOUBLE_CONSTRAINT_PRECISION 0.000001

//-----------------------------------------------------------------------------
qtUIManager* qtUIManager::Instance = 0;

//-----------------------------------------------------------------------------
qtUIManager* qtUIManager::instance()
{
  return qtUIManager::Instance;
}

//----------------------------------------------------------------------------
qtUIManager::qtUIManager(slctk::attribute::Manager &manager) :
  m_AttManager(manager)
{
  if (!qtUIManager::Instance)
    {
    qtUIManager::Instance = this;
    }
  this->RootSection = NULL;
  this->ShowAdvanced =false;
  this->advFont.setBold(true);
  this->DefaultValueColor.setRgbF(1.0, 1.0, 0.5);
}

//----------------------------------------------------------------------------
qtUIManager::~qtUIManager()
{
  if(this->RootSection)
    {
    delete this->RootSection;
    }
  if (qtUIManager::Instance == this)
    {
    qtUIManager::Instance = 0;
    }
}

//----------------------------------------------------------------------------
void qtUIManager::initializeUI(QWidget* pWidget)
{
  if(!this->m_AttManager.rootSection())
    {
    return;
    }
  if(this->RootSection)
    {
    delete this->RootSection;
    }
  this->RootSection = new qtRootSection(
    this->m_AttManager.rootSection(), pWidget);
}

//----------------------------------------------------------------------------
bool qtUIManager::passItemAdvancedCheck(bool advancedItem)
{
  return (!advancedItem || advancedItem==this->showAdvanced());
}
//----------------------------------------------------------------------------
bool qtUIManager::passAttributeAdvancedCheck(bool advancedAtt)
{
  return (!advancedAtt || advancedAtt==this->showAdvanced());
}

//----------------------------------------------------------------------------
void qtUIManager::processAttributeSection(qtAttributeSection* qtSec)
{
  slctk::AttributeSectionPtr sec = slctk::dynamicCastPointer<AttributeSection>(
    qtSec->getObject());

  qtUIManager::processBasicSection(qtSec);
}
//----------------------------------------------------------------------------
void qtUIManager::processInstancedSection(qtInstancedSection* qtSec)
{
  slctk::InstancedSectionPtr sec = slctk::dynamicCastPointer<InstancedSection>(
    qtSec->getObject());

  qtUIManager::processBasicSection(qtSec);
}
//----------------------------------------------------------------------------
void qtUIManager::processModelEntitySection(qtModelEntitySection* qtSec)
{
  slctk::ModelEntitySectionPtr sec = slctk::dynamicCastPointer<ModelEntitySection>(
    qtSec->getObject());

  qtUIManager::processBasicSection(qtSec);
}
//----------------------------------------------------------------------------
void qtUIManager::processSimpleExpressionSection(qtSimpleExpressionSection* qtSec)
{
  slctk::SimpleExpressionSectionPtr sec = slctk::dynamicCastPointer<SimpleExpressionSection>(
    qtSec->getObject());

  qtUIManager::processBasicSection(qtSec);
}
//----------------------------------------------------------------------------
void qtUIManager::processGroupSection(qtGroupSection* pQtGroup)
{
  slctk::GroupSectionPtr group = slctk::dynamicCastPointer<GroupSection>(
    pQtGroup->getObject());
  qtUIManager::processBasicSection( pQtGroup);
  std::size_t i, n = group->numberOfSubsections();
  slctk::SectionPtr sec;
  qtSection* qtSec = NULL;
  for (i = 0; i < n; i++)
    {
    sec = group->subsection(i);
    switch(sec->type())
      {
      case Section::ATTRIBUTE:
        qtSec = new qtAttributeSection(sec, pQtGroup->widget());
        qtUIManager::processAttributeSection(qobject_cast<qtAttributeSection*>(qtSec));
        break;
      case Section::GROUP:
        qtSec = new qtGroupSection(sec, pQtGroup->widget());
        qtUIManager::processGroupSection(qobject_cast<qtGroupSection*>(qtSec));
        break;
      case Section::INSTANCED:
        qtSec = new qtInstancedSection(sec, pQtGroup->widget());
        qtUIManager::processInstancedSection(qobject_cast<qtInstancedSection*>(qtSec));
        break;
      case Section::MODEL_ENTITY:
        qtSec = new qtModelEntitySection(sec, pQtGroup->widget());
        qtUIManager::processModelEntitySection(qobject_cast<qtModelEntitySection*>(qtSec));
        break;
      case Section::SIMPLE_EXPRESSION:
        qtSec = new qtSimpleExpressionSection(sec, pQtGroup->widget());
        qtUIManager::processSimpleExpressionSection(qobject_cast<qtSimpleExpressionSection*>(qtSec));
        break;
      default:
        break;
        //this->m_errorStatus << "Unsupport Section Type " 
        //                    << Section::type2String(sec->type()) << "\n";
      }
    if(qtSec)
      {
      pQtGroup->addChildSection(qtSec);
      }
    }
}

//----------------------------------------------------------------------------
void qtUIManager::processBasicSection(qtSection* sec)
{
  //node.append_attribute("Title").set_value(sec->title().c_str());
  //if (sec->iconName() != "")
  //  {
  //  node.append_attribute("Icon").set_value(sec->title().c_str());
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
  if(this->RootSection)
    {
    delete this->RootSection;
    this->RootSection = NULL;
    }
  this->setShowAdvanced(false);
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
  slctk::GroupItemPtr dataItem, QTableWidget* widget)
{
  widget->clear();
  widget->setRowCount(0);
  widget->setColumnCount(0);

  if(!dataItem)
    {
    return;
    }

  std::size_t i, n = dataItem->numberOfGroups();
  std::size_t j, m = dataItem->numberOfItemsPerGroup();
  if(!m  || !n)
    {
    return;
    }
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(dataItem->item(0));
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return;
    }

  int numCols = (int)(n*m), numRows = (int)(item->numberOfValues());
  widget->setColumnCount(numCols);
  widget->setRowCount(numRows);
  for(int h=0; h<numCols; h++)
    {
    QTableWidgetItem *qtablewidgetitem = new QTableWidgetItem();
    widget->setHorizontalHeaderItem(h, qtablewidgetitem);
    }
  for (j = 0; j < numCols; j++) // expecting one item for each column
    {
    qtUIManager::updateTableColRows(dataItem->item(j), j, widget);
    }
}

//----------------------------------------------------------------------------
void qtUIManager::updateTableColRows(slctk::AttributeItemPtr dataItem,
    int col, QTableWidget* widget)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(dataItem);
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return;
    }
  int numRows = (int)(item->numberOfValues());
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
  slctk::GroupItemPtr dataItem, QTableWidgetItem* item)
{
  if(!dataItem)
    {
    return;
    }
  slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(
    dataItem->item(item->column()));
  slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(
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
  slctk::GroupItemPtr dataItem, QString& strValues)
{
  if(!dataItem || !dataItem->numberOfRequiredGroups())
    {
    return false;
    }
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(dataItem->item(0));
  if(!item || item->isDiscrete() || item->isExpression())
    {
    return false;
    }
  int numberOfComponents = dataItem->numberOfItemsPerGroup();   
  int nVals = (int)(item->numberOfValues());
  QStringList strVals;
  slctk::ValueItemPtr valueitem;
  for(int i=0; i < nVals; i++)
    {
    for(int c=0; c<numberOfComponents-1; c++)
      {
      valueitem =dynamicCastPointer<ValueItem>(dataItem->item(c));
      strVals << valueitem->valueAsString(i).c_str() <<"\t";
      }
    valueitem =dynamicCastPointer<ValueItem>(dataItem->item(numberOfComponents-1));
    strVals << valueitem->valueAsString(i).c_str();
    strVals << LINE_BREAKER_STRING;
    }
  strValues = strVals.join(" ");
  return true;
}

//----------------------------------------------------------------------------
void qtUIManager::removeSelectedTableValues(
  slctk::GroupItemPtr dataItem, QTableWidget* table)
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
        slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(dataItem->item(i));
        slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(dataItem->item(i));
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
void qtUIManager::addNewTableValues(slctk::GroupItemPtr dataItem,
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
    slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(dataItem->item(i));
    slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(dataItem->item(i));
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
  slctk::AttributeItemPtr attitem, int elementIdx, QWidget* pWidget)
{
  slctk::ValueItemPtr inputitem =dynamicCastPointer<ValueItem>(attitem);
  if(!inputitem)
    {
    return NULL;
    }
  slctk::AttributeRefItemPtr item =inputitem->expressionReference(elementIdx);
  if(!item)
    {
    return NULL;
    }

  const AttributeRefItemDefinition *itemDef = 
    dynamic_cast<const AttributeRefItemDefinition*>(item->definition().get());
  slctk::AttributeDefinitionPtr attDef = itemDef->attributeDefinition();
  if(!attDef)
    {
    return NULL;
    }
  QList<QString> attNames;
  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  std::vector<slctk::AttributePtr>::iterator it;
  for (it=result.begin(); it!=result.end(); ++it)
    {
    attNames.push_back((*it)->name().c_str());
    }

  QComboBox* combo = new QComboBox(pWidget);
  QVariant vdata(elementIdx);
  combo->setProperty("ElementIndex", vdata);
  QVariant vobject;
  vobject.setValue((void*)(attitem.get()));
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
  slctk::AttributeRefItemPtr item =inputitem->expressionReference(elementIdx);
  if(!item)
    {
    return;
    }

  if(curIdx>=0)
    {
    const AttributeRefItemDefinition *itemDef = 
      dynamic_cast<const AttributeRefItemDefinition*>(item->definition().get());
    AttributeDefinitionPtr attDef = itemDef->attributeDefinition();
    Manager *attManager = attDef->manager();
    AttributePtr attPtr = attManager->findAttribute(comboBox->currentText().toStdString());
    if(attPtr)
      {
      item->setValue(elementIdx, attPtr);
      }
    else
      {
      item->unset(elementIdx);
      }
    }
  else
    {
    item->unset(elementIdx);
    }
}

//----------------------------------------------------------------------------
QWidget* qtUIManager::createComboBox(
  slctk::AttributeItemPtr attitem, int elementIdx, QWidget* pWidget)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(attitem);
  if(!item)
    {
    return NULL;
    }
  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(item->definition().get());

  QList<QString> discreteVals;
  for (size_t i = 0; i < itemDef->numberOfDiscreteValues(); i++)
    {
    std::string enumText = itemDef->discreteEnum(i);
    if(itemDef->hasDefault() &&
      itemDef->defaultDiscreteIndex() == i)
      {
      enumText.append(" (Default)");
      }
    discreteVals.push_back(enumText.c_str());
    }

  QComboBox* combo = new QComboBox(pWidget);
  QVariant vdata(elementIdx);
  combo->setProperty("ElementIndex", vdata);
  QVariant vobject;
  vobject.setValue((void*)(attitem.get()));
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
  slctk::AttributeItemPtr attitem,int elementIdx,QWidget* pWidget)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(attitem);
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
  slctk::AttributeItemPtr attitem,int elementIdx,QWidget* pWidget)
{
  slctk::ValueItemPtr item =dynamicCastPointer<ValueItem>(attitem);
  if(!item)
    {
    return NULL;
    }

  QWidget* inputWidget = NULL;
  QVariant vdata(elementIdx);
  bool isDefault = false;
  switch (item->type())
    {
    case slctk::attribute::Item::DOUBLE:
      {
      QLineEdit* editBox = new QLineEdit(pWidget);
      const DoubleItemDefinition *dDef = 
        dynamic_cast<const DoubleItemDefinition*>(item->definition().get());
      QDoubleValidator *validator = new QDoubleValidator(pWidget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);
      
      double value=slctk_DOUBLE_MIN;
      if(dDef->hasMinRange())
        {
        value = dDef->minRangeInclusive() ?
          dDef->minRange() : dDef->minRange() + slctk_DOUBLE_CONSTRAINT_PRECISION;
        validator->setBottom(value);
        }
      value=slctk_DOUBLE_MAX;
      if(dDef->hasMaxRange())
        {
        value = dDef->maxRangeInclusive() ?
          dDef->maxRange() : dDef->maxRange() - slctk_DOUBLE_CONSTRAINT_PRECISION;
        validator->setTop(value);
        }

      slctk::DoubleItemPtr ditem =dynamicCastPointer<DoubleItem>(item);
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
      inputWidget = editBox;
      break;
      }
    case slctk::attribute::Item::INT:
      {
      QLineEdit* editBox = new QLineEdit(pWidget);
      const IntItemDefinition *iDef = 
        dynamic_cast<const IntItemDefinition*>(item->definition().get());
      QIntValidator *validator = new QIntValidator(pWidget);
      editBox->setValidator(validator);
      editBox->setFixedWidth(100);

      int value=slctk_INT_MIN;
      if(iDef->hasMinRange())
        {
        value = iDef->minRangeInclusive() ?
          iDef->minRange() : iDef->minRange() + 1;
        validator->setBottom(value);
        }
      value=slctk_INT_MAX;
      if(iDef->hasMaxRange())
        {
        value = iDef->maxRangeInclusive() ?
          iDef->maxRange() : iDef->maxRange() - 1;
        validator->setTop(value);
        }

      slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(item);
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
      inputWidget = editBox;
      break;
      }
    case slctk::attribute::Item::STRING:
      {
      const StringItemDefinition *sDef = 
        dynamic_cast<const StringItemDefinition*>(item->definition().get());
      slctk::StringItemPtr sitem =dynamicCastPointer<StringItem>(item);
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
      break;
      }
    default:
      //this->m_errorStatus << "Error: Unsupported Item Type: " <<
      // slctk::attribute::Item::type2String(item->type()) << "\n";
      break;
    }
  if(inputWidget)
    {
    inputWidget->setProperty("ElementIndex", vdata);
    QVariant vobject;
    vobject.setValue((void*)(attitem.get()));
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
  //slctk::ValueItemPtr item(rawitem);
  //slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(item);
  //slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(item);
  //slctk::StringItemPtr sItem =dynamicCastPointer<StringItem>(item);
  int elementIdx = editBox ? editBox->property("ElementIndex").toInt() :
    textBox->property("ElementIndex").toInt();

  if(editBox && !editBox->text().isEmpty())
    {
    if(rawitem->type()==slctk::attribute::Item::DOUBLE)
      {
      slctk::dynamicCastPointer<DoubleItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toDouble());
      }
    else if(rawitem->type()==slctk::attribute::Item::INT)
      {
      slctk::dynamicCastPointer<IntItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toInt());
      }
    else if(rawitem->type()==slctk::attribute::Item::STRING)
      {
      slctk::dynamicCastPointer<StringItem>(rawitem->pointer())
        ->setValue(elementIdx, editBox->text().toStdString());
      }
    else
      {
      rawitem->unset(elementIdx);
      }
    }
  else if(textBox && !textBox->toPlainText().isEmpty() &&
     rawitem->type()==slctk::attribute::Item::STRING)
    {
    slctk::dynamicCastPointer<StringItem>(rawitem->pointer())
      ->setValue(elementIdx, textBox->toPlainText().toStdString());
    }
  else
    {
    rawitem->unset(elementIdx);
    }
}
//----------------------------------------------------------------------------
std::string qtUIManager::getValueItemCommonLabel(
  slctk::ValueItemPtr attItem) const
{
  const ValueItemDefinition *itemDef = 
    dynamic_cast<const ValueItemDefinition*>(attItem->definition().get());

  if(itemDef && itemDef->usingCommonLabel() && itemDef->hasValueLabels() &&
    !itemDef->valueLabel(0).empty())
    {
    return itemDef->valueLabel(0);
    }
  return "";
}
//----------------------------------------------------------------------------
std::string qtUIManager::getGroupItemCommonLabel(
  slctk::GroupItemPtr attItem) const
{
  const GroupItemDefinition *groupDef = 
    dynamic_cast<const GroupItemDefinition*>(attItem->definition().get());

  if(groupDef && groupDef->usingCommonSubGroupLabel() &&
    groupDef->hasSubGroupLabels() && !groupDef->subGroupLabel(0).empty())
    {
    return groupDef->subGroupLabel(0);
    }
  return "";
}

//----------------------------------------------------------------------------
std::string qtUIManager::getItemCommonLabel(
  slctk::AttributeItemPtr attItem)
{
  if(attItem->type() == slctk::attribute::Item::GROUP)
    {
    return this->getGroupItemCommonLabel(dynamicCastPointer<GroupItem>(attItem));
    }
  else
    {
    return this->getValueItemCommonLabel(dynamicCastPointer<ValueItem>(attItem));
    }
  return "";
}
//----------------------------------------------------------------------------
bool qtUIManager::updateTableItemCheckState(
  QTableWidgetItem* labelitem, slctk::AttributeItemPtr attItem)
{
  bool bEnabled = true;
  if(attItem->definition()->isOptional())
    {
    Qt::CheckState checkState = attItem->definition()->isEnabledByDefault() ?
      Qt::Checked : Qt::Unchecked;
    labelitem->setCheckState(checkState);
    QVariant vdata;
    vdata.setValue((void*)attItem.get());
    labelitem->setData(Qt::UserRole, vdata);
    labelitem->setFlags(labelitem->flags() | Qt::ItemIsUserCheckable);
    bEnabled = (checkState==Qt::Checked);
    }
  return bEnabled;
}
