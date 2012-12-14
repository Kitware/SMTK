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
#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/ValueItem.h"
#include "attribute/ValueItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/IntItem.h"

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

  if(!dataItem || !dataItem->numberOfGroups())
    {
    return;
    }

  std::size_t i, n = dataItem->numberOfGroups();
  std::size_t j, m = dataItem->numberOfItemsPerGroup();
  if(!m  || !n)
    {
    return;
    }
  int numCols = (int)n, numRows = (int)m;  
  widget->setColumnCount(numCols);
  for(int h=0; h<numCols; h++)
    {
    QTableWidgetItem *qtablewidgetitem = new QTableWidgetItem();
    widget->setHorizontalHeaderItem(h, qtablewidgetitem);
    }

  for(i = 0; i < n; i++)
    {
    for (j = 0; j < m; j++) // expecting one item for each column
      {
      qtUIManager::updateTableColRows(dataItem->item(i,j), i, widget);
      }
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
    dataItem->item(item->column(),0));
  slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(
    dataItem->item(item->column(),0));
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
        slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(dataItem->item(i,0));
        slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(dataItem->item(i,0));
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
    slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(dataItem->item(i,0));
    slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(dataItem->item(i,0));
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
