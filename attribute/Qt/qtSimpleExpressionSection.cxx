/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "qtSimpleExpressionSection.h"

#include "qtUIManager.h"
#include "qtTableWidget.h"
#include "qtAttribute.h"

#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/Manager.h"
#include "attribute/GroupItem.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/SimpleExpressionSection.h"

#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
#include <QSplitter>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QPointer>

#include <sstream>

#define MAX_NUMBEWR_FUNC_POINTS 10000

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtSimpleExpressionSectionInternals
{
public:
  qtSimpleExpressionSectionInternals()
  {
    this->FunctionParserDescription = 0;
  }

  ~qtSimpleExpressionSectionInternals()
  {
    if(this->FunctionParserDescription)
      {
      delete [] this->FunctionParserDescription;
      this->FunctionParserDescription = 0;
      }
  }

  const char* getFunctionParserDescription()
    {
    if(!this->FunctionParserDescription)
      {
      std::stringstream ss;
      ss << "Example Function: f(X) = cos(X).\n";
      ss << "(Note: Use capital X as variable!)\n";
      ss << "                                  \n";
      ss << "Standard constants available:\n";
      ss << "  PI = 3.1415926535\n";
      ss << "                                \n";
      ss << "Standard operations available:\n";
      ss << "  + - * / ^\n";
      ss << "                                \n";
      ss << "Standard functions available:\n";
      ss << "  abs acos asin atan ceil cos cosh\n";
      ss << "  exp floor log mag min max norm\n";
      ss << "  sign sin sinh sqrt tan tanh\n";
      this->FunctionParserDescription = new char[ss.str().length() + 1];
      strcpy(this->FunctionParserDescription, ss.str().c_str());
      }

    return this->FunctionParserDescription;
    }

  qtTableWidget* FuncTable;
  QListWidget* FuncList;
  QPushButton* AddButton;
  QPushButton* DeleteButton;
  QPushButton* CopyButton;

  QSpinBox*    NumberBox;
  QLineEdit*   ExpressionInput;
  QLineEdit*   DeltaInput;
  QLineEdit*   InitValueInput;
  QPushButton* AddValueButton;
  QPushButton* RemoveValueButton;
  QGroupBox*   EditorGroup;

  char*        FunctionParserDescription;

};

//----------------------------------------------------------------------------
qtSimpleExpressionSection::qtSimpleExpressionSection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtSimpleExpressionSectionInternals;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtSimpleExpressionSection::~qtSimpleExpressionSection()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::createWidget()
{
  if(!this->getObject())
    {
    return;
    }
  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  QFrame* leftFrame = new QFrame(frame);
  QFrame* rightFrame = new QFrame(frame);
  //QGridLayout* gridLayout = new QGridLayout(frame);
  //gridLayout->setMargin(0);
  QVBoxLayout* leftLayout = new QVBoxLayout(leftFrame);
  leftLayout->setMargin(0);
  QVBoxLayout* rightLayout = new QVBoxLayout(rightFrame);
  rightLayout->setMargin(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // create a list box for all the array entries
  this->Internals->FuncList = new QListWidget(frame);
    
  this->Internals->FuncTable = new qtTableWidget(frame);
  QSizePolicy tableSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->Internals->FuncTable->setSizePolicy(tableSizePolicy);

  this->Internals->AddButton = new QPushButton("New", frame);
  this->Internals->AddButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->NumberBox = new QSpinBox(frame);
  this->Internals->NumberBox->setSizePolicy(sizeFixedPolicy);
  this->Internals->NumberBox->setRange(1, MAX_NUMBEWR_FUNC_POINTS);
  this->Internals->NumberBox->setValue(10);

// Editor UI
  QDoubleValidator *validator = new QDoubleValidator(frame);
  this->Internals->ExpressionInput = new QLineEdit(frame);
  this->Internals->DeltaInput = new QLineEdit(frame);
  this->Internals->DeltaInput->setValidator(validator);
  this->Internals->InitValueInput = new QLineEdit(frame);
  this->Internals->InitValueInput->setValidator(validator);
  
  QGridLayout* editorLayout = new QGridLayout();
  editorLayout->addWidget(new QLabel("Initial Value",frame), 0, 0);
  editorLayout->addWidget(new QLabel("Delta",frame), 0, 1);
  editorLayout->addWidget(new QLabel("Number of Values",frame), 0, 2);
  editorLayout->addWidget(this->Internals->InitValueInput, 1, 0);
  editorLayout->addWidget(this->Internals->DeltaInput, 1, 1);
  editorLayout->addWidget(this->Internals->NumberBox, 1, 2);
  this->Internals->InitValueInput->setText("0.0");
  this->Internals->DeltaInput->setText("0.0");
  
  this->Internals->EditorGroup = new QGroupBox("Use Function Expression", frame);
  this->Internals->EditorGroup->setCheckable(1);
  this->Internals->EditorGroup->setChecked(0);
  this->Internals->EditorGroup->setToolTip(
    this->Internals->getFunctionParserDescription());
  
  QVBoxLayout* addLayout = new QVBoxLayout(this->Internals->EditorGroup);
  QHBoxLayout* exprLayout = new QHBoxLayout();
  exprLayout->addWidget(new QLabel("f(X)=",frame));
  exprLayout->addWidget(this->Internals->ExpressionInput);
  addLayout->addLayout(exprLayout);
  addLayout->addLayout(editorLayout);

  QHBoxLayout* copyLayout = new QHBoxLayout();
  this->Internals->DeleteButton = new QPushButton("Delete", frame);
  this->Internals->DeleteButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->CopyButton = new QPushButton("Copy", frame);
  this->Internals->CopyButton->setSizePolicy(sizeFixedPolicy);

  copyLayout->addWidget(this->Internals->AddButton);
  copyLayout->addWidget(this->Internals->CopyButton);
  copyLayout->addWidget(this->Internals->DeleteButton);

  QHBoxLayout* rowButtonLayout = new QHBoxLayout();
  this->Internals->AddValueButton = new QPushButton("Add", frame);
  this->Internals->AddValueButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->RemoveValueButton = new QPushButton("Remove", frame);
  this->Internals->RemoveValueButton->setSizePolicy(sizeFixedPolicy);

  rowButtonLayout->addWidget(this->Internals->AddValueButton);
  rowButtonLayout->addWidget(this->Internals->RemoveValueButton);

  leftLayout->addWidget(this->Internals->FuncList);//, 0, 0,1,1);
  //leftLayout->addWidget(this->Internals->AddButton);
  leftLayout->addWidget(this->Internals->EditorGroup);//, 1, 0,1,1);
  //leftLayout->addLayout(editorLayout);
  leftLayout->addLayout(copyLayout);//, 2, 0,1,1);
  rightLayout->addWidget(this->Internals->FuncTable);//, 0, 1, 2, 1);
  rightLayout->addLayout(rowButtonLayout);//, 2, 1,1,1);

  frame->addWidget(leftFrame);
  frame->addWidget(rightFrame);
  QObject::connect(this->Internals->FuncList, 
    SIGNAL(currentItemChanged (QListWidgetItem * , QListWidgetItem * )), 
    this, SLOT(onFuncSelectionChanged(QListWidgetItem * , QListWidgetItem * )));
  QObject::connect(this->Internals->FuncList, 
    SIGNAL(itemChanged (QListWidgetItem *)), 
    this, SLOT(onFuncNameChanged(QListWidgetItem * )));

  QObject::connect(this->Internals->AddButton, 
    SIGNAL(clicked()), this, SLOT(onCreateNew()));
  QObject::connect(this->Internals->CopyButton, 
    SIGNAL(clicked()), this, SLOT(onCopySelected()));
  QObject::connect(this->Internals->DeleteButton, 
    SIGNAL(clicked()), this, SLOT(onDeleteSelected()));
  QObject::connect(this->Internals->AddValueButton, 
    SIGNAL(clicked()), this, SLOT(onAddValue()));
  QObject::connect(this->Internals->RemoveValueButton, 
    SIGNAL(clicked()), this, SLOT(onRemoveSelectedValues()));

  QObject::connect(this->Internals->FuncTable, 
    SIGNAL(itemChanged (QTableWidgetItem *)), 
    this, SLOT(onFuncValueChanged(QTableWidgetItem * )));
  QObject::connect(this->Internals->FuncTable, 
    SIGNAL(keyPressed (QKeyEvent *)), 
    this, SLOT(onFuncTableKeyPress(QKeyEvent * )));
  this->Internals->FuncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->FuncList->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Widget = frame;
  this->parentWidget()->layout()->setAlignment(Qt::AlignJustify);
  this->parentWidget()->layout()->addWidget(frame);
  
  this->initFunctionList();
  
  if(this->Internals->FuncList->count())
    {
    this->Internals->FuncList->setCurrentRow(0);
    }
}

//-----------------------------------------------------------------------------
slctk::GroupItemPtr qtSimpleExpressionSection::getArrayDataFromItem(QListWidgetItem * item)
{
  return this->getFunctionArrayData(this->getFunctionFromItem(item));
}
//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtSimpleExpressionSection::getStringDataFromItem(QListWidgetItem * item)
{
  return this->getFunctionStringData(this->getFunctionFromItem(item));
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtSimpleExpressionSection::getFunctionFromItem(
  QListWidgetItem * item)
{ 
  Attribute* rawPtr = item ? 
    static_cast<Attribute*>(item->data(Qt::UserRole).value<void *>()) : NULL;
  return rawPtr ? rawPtr->pointer() : slctk::AttributePtr();
}
//-----------------------------------------------------------------------------
slctk::GroupItemPtr qtSimpleExpressionSection::getSelectedArrayData()
{
  return this->getFunctionArrayData(this->getSelectedFunction());
}
//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtSimpleExpressionSection::getSelectedStringData()
{
  return this->getFunctionStringData(this->getSelectedFunction());
}
//-----------------------------------------------------------------------------
slctk::AttributePtr qtSimpleExpressionSection::getSelectedFunction()
{
  return this->getFunctionFromItem(this->getSelectedItem());
}
//-----------------------------------------------------------------------------
QListWidgetItem *qtSimpleExpressionSection::getSelectedItem()
{
  return this->Internals->FuncList->selectedItems().count()>0 ?
    this->Internals->FuncList->selectedItems().value(0) : NULL;
}

//-----------------------------------------------------------------------------
slctk::GroupItemPtr qtSimpleExpressionSection::getFunctionArrayData(
  slctk::AttributePtr func)
{
  return func ? dynamicCastPointer<GroupItem>(func->item(0)) :
    slctk::GroupItemPtr();
}

//-----------------------------------------------------------------------------
slctk::ValueItemPtr qtSimpleExpressionSection::getFunctionStringData(
  slctk::AttributePtr func)
{
  if(func && func->numberOfItems()==2)// Kind of Hack
    {
    return dynamicCastPointer<ValueItem>(func->item(1));
    }
  return slctk::ValueItemPtr();
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onFuncSelectionChanged(
  QListWidgetItem * current, QListWidgetItem * previous)
{
  slctk::GroupItemPtr dataItem = this->getArrayDataFromItem(current);
  this->Internals->FuncTable->blockSignals(true);
  if(dataItem)
    {
    qtUIManager::instance()->updateArrayTableWidget(dataItem, 
      this->Internals->FuncTable);
    this->Internals->FuncTable->resizeColumnsToContents();
    if(this->Internals->FuncTable->columnCount()>=2)
      {
      this->Internals->FuncTable->setHorizontalHeaderLabels(
          QStringList() << tr("x") << tr("f(x)") );
      }
    }
  else
    {
    this->Internals->FuncTable->clear();
    this->Internals->FuncTable->setRowCount(0);
    this->Internals->FuncTable->setColumnCount(0);
    }
  this->Internals->FuncTable->blockSignals(false);
  
  // Now set up the function editor UI
  
  slctk::ValueItemPtr expressionItem = this->getStringDataFromItem(current);
  this->updateFunctionEditorUI(expressionItem, dataItem);
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::updateFunctionEditorUI(
  slctk::ValueItemPtr expressionItem, slctk::GroupItemPtr arrayItem)
{
  this->Internals->ExpressionInput->setText("");
  this->Internals->NumberBox->setValue(10);
  this->Internals->DeltaInput->setText("0.0");
  this->Internals->InitValueInput->setText("0.0");
  
  if(!expressionItem)
    {
    return;
    }
  this->Internals->ExpressionInput->setText(
    expressionItem->valueAsString().c_str());
  if(arrayItem)
    {
    int n = (int)(arrayItem->numberOfGroups());
    int m = (int)(arrayItem->numberOfItemsPerGroup()); // expecting 1
    if(!m  || !n)
      {
      return;
      }
    slctk::ValueItemPtr valueItem =dynamicCastPointer<ValueItem>(arrayItem->item(0,0));
    slctk::DoubleItemPtr dItem =dynamicCastPointer<DoubleItem>(arrayItem->item(0,0));
    slctk::IntItemPtr iItem =dynamicCastPointer<IntItem>(arrayItem->item(0,0));

    if(valueItem && valueItem->numberOfValues())
      {
      int numValues = (int)(valueItem->numberOfValues());
      this->Internals->InitValueInput->setText(valueItem->valueAsString(0).c_str());
      this->Internals->DeltaInput->setText(valueItem->valueAsString(0).c_str());
      this->Internals->NumberBox->setValue(numValues);
      if(numValues>1 && n==2 && (dItem || iItem))
        {
        double deltaVal = dItem ? (dItem->value(1)-dItem->value(0)) :
          (iItem->value(1)-iItem->value(0));
        this->Internals->DeltaInput->setText(QString::number(deltaVal));
        }
      }
    }
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onFuncNameChanged(QListWidgetItem* item)
{
  slctk::AttributePtr func = this->getFunctionFromItem(item);
  if(func)
    {
    Manager *attManager = func->definition()->manager();
    attManager->rename(func, item->text().toAscii().constData());
    //func->definition()->setLabel(item->text().toAscii().constData());

    // Lets see what attributes are being referenced
    std::vector<slctk::AttributeItemPtr> refs;
    std::size_t i;
    func->references(refs);
    for (i = 0; i < refs.size(); i++)
      {
      std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
        << "\n";
      } 
    } 
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onFuncValueChanged(QTableWidgetItem* item)
{
  slctk::GroupItemPtr dataItem = this->getSelectedArrayData();
  if(!dataItem)
    {
    return;
    }
  qtUIManager::instance()->updateArrayDataValue(dataItem, item);
  this->clearFuncExpression();
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onCreateNew()
{
  if(this->Internals->EditorGroup->isChecked())
    {
    this->createFunctionWithExpression();
    }
  else
    {
/*
    int numRows = this->Internals->NumberBox->value();
    vtkDoubleArray* newArray = vtkDoubleArray::New();
    newArray->SetNumberOfComponents(2);
    for(int i=0; i<numRows; i++)
      {
      double initV[2]={0.0, 0.0};
      newArray->InsertNextTuple(initV);
      }
*/
    slctk::SimpleExpressionSectionPtr sec =
      slctk::dynamicCastPointer<SimpleExpressionSection>(this->getObject());
    if(!sec || !sec->definition())
      {
      return;
      }

    this->createNewFunction(sec->definition());
    }
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::createFunctionWithExpression()
{
  QString funcExpr = this->Internals->ExpressionInput->text();
  if(funcExpr.isEmpty())
    {
    funcExpr = "X";
    }
  this->Internals->ExpressionInput->setText(funcExpr);
/*
  int errorPos = -1;
  std::string errorMsg;
  this->getFunctionContainer()->GetFunctionParser()->SetFunction(funcExpr.toStdString());
  this->getFunctionContainer()->GetFunctionParser()->CheckExpression(errorPos, errorMsg);
  QString strMessage = QString(errorMsg.c_str()) + 
    "\nThe function expression has some syntax error at cursor position: " + 
    QString::number(errorPos);
    
  if(errorPos != -1 && !errorMsg.empty())
    {
    QMessageBox::warning(this->parentWidget(), tr("SimBuilder Functions"),strMessage);
    this->Internals->ExpressionInput->setFocus();
    this->Internals->ExpressionInput->setCursorPosition(errorPos);
    return;
    }
*/
  double initVal = this->Internals->InitValueInput->text().toDouble();
  double deltaVal = this->Internals->DeltaInput->text().toDouble();
  int numValues = this->Internals->NumberBox->value();
  emit this->onCreateFunctionWithExpression(
    funcExpr, initVal, deltaVal, numValues);
/*
  // Need "Delete" after done.
  slctk::AttributePtr resultContainer = this->getFunctionContainer()->BuildFunction(
    "Function1DLinear", "New Func with Expr", 
    funcExpr.toAscii().constData(), initVal, deltaVal, numValues);
  if(resultContainer)
    {
    QListWidgetItem* item = this->addFunctionListItem(resultContainer);
    if(item)
      {
      this->Internals->FuncList->setCurrentItem(item);
      }
    }
*/
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::createNewFunction(
  slctk::AttributeDefinitionPtr attDef)
{
  if(!attDef)
    {
    return;
    }

  Manager *attManager = attDef->manager();

  slctk::AttributePtr newFunc = attManager->createAttribute(attDef->type());
  QListWidgetItem* item = this->addFunctionListItem(newFunc);
  if(item)
    {
    this->Internals->FuncList->setCurrentItem(item);
    }
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onCopySelected()
{
  slctk::AttributePtr dataItem = this->getSelectedFunction();
  if(dataItem)
    {
    this->createNewFunction(dataItem->definition());
    }
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onDeleteSelected()
{
  QListWidgetItem* selItem = this->getSelectedItem();
  if(selItem)
    {
    slctk::SimpleExpressionSectionPtr sec =
      slctk::dynamicCastPointer<SimpleExpressionSection>(this->getObject());
    if(!sec || !sec->definition())
      {
      return;
      }

    AttributeDefinitionPtr attDef = sec->definition();
    Manager *attManager = attDef->manager();
    attManager->removeAttribute(this->getFunctionFromItem(selItem));

    this->Internals->FuncList->takeItem(this->Internals->FuncList->row(selItem));
    }
}
//----------------------------------------------------------------------------
QListWidgetItem* qtSimpleExpressionSection::addFunctionListItem(
  slctk::AttributePtr childData)
{
  if(!qtUIManager::instance()->passAttributeAdvancedCheck(
    childData->definition()->advanceLevel()))
    {
    return NULL;
    }
    
  QListWidgetItem* item = NULL;
  slctk::GroupItemPtr dataItem = this->getFunctionArrayData(childData);
  if(dataItem)
    {
    item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()),
      this->Internals->FuncList, slctk_USER_DATA_TYPE);
    QVariant vdata;
    vdata.setValue((void*)(childData.get()));
    item->setData(Qt::UserRole, vdata);
    if(childData->definition()->advanceLevel())
      {
      item->setFont(qtUIManager::instance()->advancedFont());
      }
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    this->Internals->FuncList->addItem(item);
    }
  return item;
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onFuncTableKeyPress(QKeyEvent* e)
{

  // Allow paste
  if(e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
    {
    QString values = qtUIManager::instance()->clipBoardText();
    this->pasteFunctionValues(values);
    e->accept();
    return;
    }
  // Allow copying 
  if(e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
    {
    QStringList list ;
    int numSelRows = this->Internals->FuncTable->getSelectedIndexes().count()/this->Internals->FuncTable->columnCount();
    for(int r=0; r<this->Internals->FuncTable->rowCount(); r++)
      {
      if(this->Internals->FuncTable->item(r, 0)->isSelected())
        {
        for (int c=0; c<this->Internals->FuncTable->columnCount(); c++)
          {
          list << this->Internals->FuncTable->item(r, c)->text();
          if(c<this->Internals->FuncTable->columnCount()-1) 
            {
            list << "\t";
            }
          }
#ifdef WIN32
        list << "\n";
#else
        list << "\r";
#endif
        }
      }

    QString tempText = list.join(" ");
    qtUIManager::instance()->setClipBoardText( tempText ) ;

    e->accept();
    return;
    }

  // Allow delete
  if(e->key() == Qt::Key_Delete)
    {
    this->onRemoveSelectedValues();
    e->accept();
    return;

    }
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::pasteFunctionValues(QString& str)
{
  if(str.isNull())
    {
    return;
    }
  QStringList rows;
#ifdef WIN32
   rows = str.split('\n');
#else
   rows = str.split('\r');
#endif

  int numRows = rows.count()-1;
  int numColumns = rows.first().count('\t') + 1;
  QTableWidget* table = this->Internals->FuncTable;
  if (table->columnCount() != numColumns)
    {
    QMessageBox::warning(this->parentWidget(), tr("SimBuilder Functions"),
      tr("The information cannot be pasted because the copy "
      "and paste columns aren’t the same size."));
    return;
    }

  // add all the pasted rows  
  for (int i = 0; i < numRows; ++i) 
    {
    QStringList columns = rows[i].split('\t');
    if(columns.count() != numColumns)
      {
      continue;
      }
    double* vals = new double[numColumns];
    for (int j = 0; j < numColumns; ++j) 
      {
      vals[j] = columns[j].toDouble();
      }
    this->addNewValue(vals, numColumns);
    delete vals;
    }
  this->Internals->FuncTable->resizeColumnsToContents();
  this->clearFuncExpression();
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onAddValue()
{
  int numVals = this->Internals->FuncTable->columnCount();
  double zero = 0.;
  std::vector<double> vals(numVals, zero);
  //  qtUIManager::instance()->updateArrayDataValue(dataItem, item);
  this->addNewValue(&vals[0], numVals);
  this->clearFuncExpression();
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::addNewValue(double* vals, int numVals)
{
  slctk::GroupItemPtr dataItem = this->getSelectedArrayData();
  if(!dataItem)
    {
    return;
    }
  this->Internals->FuncTable->blockSignals(true);
  qtUIManager::instance()->addNewTableValues(dataItem,
    this->Internals->FuncTable, vals, numVals);
  this->Internals->FuncTable->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::onRemoveSelectedValues()
{
  slctk::GroupItemPtr dataItem = this->getSelectedArrayData();
  if(!dataItem)
    {
    return;
    }
  this->Internals->FuncTable->blockSignals(true);
  qtUIManager::instance()->removeSelectedTableValues(dataItem, 
    this->Internals->FuncTable);
  this->Internals->FuncTable->blockSignals(false);
  this->clearFuncExpression();
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::initFunctionList()
{
  slctk::SimpleExpressionSectionPtr sec =
    slctk::dynamicCastPointer<SimpleExpressionSection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }

  AttributeDefinitionPtr attDef = sec->definition();

  std::vector<slctk::AttributePtr> result;
  Manager *attManager = attDef->manager();
  attManager->findAttributes(attDef, result);
  std::vector<slctk::AttributePtr>::iterator it;
  this->Internals->FuncList->blockSignals(true);    
  for (it=result.begin(); it!=result.end(); ++it)
    {
    this->addFunctionListItem(*it);
    }
  this->Internals->FuncList->blockSignals(false);
}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::clearFuncExpression()
{
  slctk::ValueItemPtr strItem = this->getSelectedStringData();
  if(strItem)
    {
    strItem->unset();
    }
  this->Internals->ExpressionInput->setText("");
}

//----------------------------------------------------------------------------
void qtSimpleExpressionSection::showAdvanced(int checked)
{

}
//----------------------------------------------------------------------------
void qtSimpleExpressionSection::getAllDefinitions(
  std::vector<slctk::AttributeDefinitionPtr>& defs)
{
  slctk::SimpleExpressionSectionPtr sec =
    slctk::dynamicCastPointer<SimpleExpressionSection>(this->getObject());
  if(!sec || !sec->definition())
    {
    return;
    }

  AttributeDefinitionPtr attDef = sec->definition();
  this->qtSection::getDefinitions(attDef, defs);
}

//AttributeDefinitionPtr attDef = sec->definition();
//Manager *attManager = attDef->manager();
//std::vector<slctk::AttributeDefinitionPtr> defs;
//this->getAllDefinitions(defs);
//std::vector<slctk::AttributeDefinitionPtr>::iterator itDef;
//for (itDef=defs.begin(); itDef!=defs.end(); ++itDef)
//  {
//  std::vector<slctk::AttributePtr> result;
//  attManager->findAttributes(*itDef, result);
//  std::vector<slctk::AttributePtr>::iterator itAtt;
//  for (itAtt=result.begin(); itAtt!=result.end(); ++itAtt)
//    {
//    this->addAttributeListItem(*itAtt);
//    }
//  }
