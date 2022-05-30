//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtSimpleExpressionView.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/view/Configuration.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVariant>

#include <sstream>

#define MAX_NUMBEWR_FUNC_POINTS 10000

using namespace smtk::attribute;
using namespace smtk::extension;

qtSimpleExpressionView::qtSimpleExpressionViewInternals::~qtSimpleExpressionViewInternals()
{
  if (this->FunctionParserDescription)
  {
    delete[] this->FunctionParserDescription;
    this->FunctionParserDescription = nullptr;
  }
}

const char* qtSimpleExpressionView::qtSimpleExpressionViewInternals::getFunctionParserDescription()
{
  if (!this->FunctionParserDescription)
  {
    std::stringstream ss;
    ss << "Enable VTK in the build configurations\n";
    ss << "to allow custom lambda function\n";
    this->FunctionParserDescription = new char[ss.str().length() + 1];
    strcpy(this->FunctionParserDescription, ss.str().c_str());
  }

  return this->FunctionParserDescription;
}

qtBaseView* qtSimpleExpressionView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new qtSimpleExpressionView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

qtSimpleExpressionView::qtSimpleExpressionView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new qtSimpleExpressionViewInternals;
}

qtSimpleExpressionView::~qtSimpleExpressionView()
{
  delete this->Internals;
}

void qtSimpleExpressionView::createWidget()
{
  if (!this->configuration())
  {
    return;
  }
  // Create a frame to contain all gui components for this object
  // Create a list box for the group entries
  // Create a table widget
  // Add link from the listbox selection to the table widget
  // A common add/delete/(copy/paste ??) widget

  QSplitter* frame = new QSplitter(this->parentWidget());
  frame->setObjectName(this->configuration()->name().c_str());
  QFrame* leftFrame = new QFrame(frame);
  leftFrame->setObjectName("left");
  QFrame* rightFrame = new QFrame(frame);
  rightFrame->setObjectName("right");
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
  QDoubleValidator* validator = new QDoubleValidator(frame);
  this->Internals->ExpressionInput = new QLineEdit(frame);
  this->Internals->DeltaInput = new QLineEdit(frame);
  this->Internals->DeltaInput->setValidator(validator);
  this->Internals->InitValueInput = new QLineEdit(frame);
  this->Internals->InitValueInput->setValidator(validator);

  QGridLayout* editorLayout = new QGridLayout();
  editorLayout->addWidget(new QLabel("Initial Value", frame), 0, 0);
  editorLayout->addWidget(new QLabel("Delta", frame), 0, 1);
  editorLayout->addWidget(new QLabel("Number of Values", frame), 0, 2);
  editorLayout->addWidget(this->Internals->InitValueInput, 1, 0);
  editorLayout->addWidget(this->Internals->DeltaInput, 1, 1);
  editorLayout->addWidget(this->Internals->NumberBox, 1, 2);
  this->Internals->InitValueInput->setText("0.0");
  this->Internals->DeltaInput->setText("1.0");

  this->Internals->EditorGroup = new QGroupBox("Use Function Expression", frame);
  this->Internals->EditorGroup->setCheckable(true);
  this->Internals->EditorGroup->setChecked(false);
  // Without vtk enabled, the parser cannot do anything here
  this->Internals->EditorGroup->setDisabled(true);
  this->Internals->EditorGroup->setToolTip(this->Internals->getFunctionParserDescription());

  QVBoxLayout* addLayout = new QVBoxLayout(this->Internals->EditorGroup);
  QHBoxLayout* exprLayout = new QHBoxLayout();
  exprLayout->addWidget(new QLabel("f(X)=", frame));
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
  this->Internals->LoadCSVButton = new QPushButton("Load CSV", frame);
  this->Internals->LoadCSVButton->setSizePolicy(sizeFixedPolicy);

  rowButtonLayout->addWidget(this->Internals->LoadCSVButton);
  rowButtonLayout->addWidget(this->Internals->AddValueButton);
  rowButtonLayout->addWidget(this->Internals->RemoveValueButton);

  leftLayout->addLayout(copyLayout);                //, 2, 0,1,1);
  leftLayout->addWidget(this->Internals->FuncList); //, 0, 0,1,1);
  //leftLayout->addWidget(this->Internals->AddButton);
  leftLayout->addWidget(this->Internals->EditorGroup); //, 1, 0,1,1);
  //leftLayout->addLayout(editorLayout);
  rightLayout->addLayout(rowButtonLayout);            //, 2, 1,1,1);
  rightLayout->addWidget(this->Internals->FuncTable); //, 0, 1, 2, 1);

  frame->addWidget(leftFrame);
  frame->addWidget(rightFrame);
  QObject::connect(
    this->Internals->FuncList,
    SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
    this,
    SLOT(onFuncSelectionChanged(QListWidgetItem*, QListWidgetItem*)));
  QObject::connect(
    this->Internals->FuncList,
    SIGNAL(itemChanged(QListWidgetItem*)),
    this,
    SLOT(onFuncNameChanged(QListWidgetItem*)));

  QObject::connect(this->Internals->AddButton, SIGNAL(clicked()), this, SLOT(onCreateNew()));
  QObject::connect(this->Internals->CopyButton, SIGNAL(clicked()), this, SLOT(onCopySelected()));
  QObject::connect(
    this->Internals->DeleteButton, SIGNAL(clicked()), this, SLOT(onDeleteSelected()));
  QObject::connect(this->Internals->AddValueButton, SIGNAL(clicked()), this, SLOT(onAddValue()));
  QObject::connect(
    this->Internals->RemoveValueButton, SIGNAL(clicked()), this, SLOT(onRemoveSelectedValues()));
  QObject::connect(this->Internals->LoadCSVButton, SIGNAL(clicked()), this, SLOT(onCSVLoad()));

  QObject::connect(
    this->Internals->FuncTable,
    SIGNAL(itemChanged(QTableWidgetItem*)),
    this,
    SLOT(onFuncValueChanged(QTableWidgetItem*)));
  QObject::connect(
    this->Internals->FuncTable,
    SIGNAL(keyPressed(QKeyEvent*)),
    this,
    SLOT(onFuncTableKeyPress(QKeyEvent*)));
  this->Internals->FuncTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internals->FuncList->setSelectionMode(QAbstractItemView::SingleSelection);

  this->Widget = frame;

  this->updateUI();
}

smtk::attribute::GroupItemPtr qtSimpleExpressionView::getArrayDataFromItem(QListWidgetItem* item)
{
  return this->getFunctionArrayData(this->getFunctionFromItem(item));
}

smtk::attribute::ValueItemPtr qtSimpleExpressionView::getStringDataFromItem(QListWidgetItem* item)
{
  return this->getFunctionStringData(this->getFunctionFromItem(item));
}

smtk::attribute::AttributePtr qtSimpleExpressionView::getFunctionFromItem(QListWidgetItem* item)
{
  Attribute* rawPtr =
    item ? static_cast<Attribute*>(item->data(Qt::UserRole).value<void*>()) : nullptr;
  return rawPtr ? rawPtr->shared_from_this() : smtk::attribute::AttributePtr();
}

smtk::attribute::GroupItemPtr qtSimpleExpressionView::getSelectedArrayData()
{
  return this->getFunctionArrayData(this->getSelectedFunction());
}

smtk::attribute::ValueItemPtr qtSimpleExpressionView::getSelectedStringData()
{
  return this->getFunctionStringData(this->getSelectedFunction());
}

smtk::attribute::AttributePtr qtSimpleExpressionView::getSelectedFunction()
{
  return this->getFunctionFromItem(this->getSelectedItem());
}

QListWidgetItem* qtSimpleExpressionView::getSelectedItem()
{
  return this->Internals->FuncList->currentItem();
}

smtk::attribute::GroupItemPtr qtSimpleExpressionView::getFunctionArrayData(
  smtk::attribute::AttributePtr func)
{
  return func ? dynamic_pointer_cast<GroupItem>(func->item(0)) : smtk::attribute::GroupItemPtr();
}

smtk::attribute::ValueItemPtr qtSimpleExpressionView::getFunctionStringData(
  smtk::attribute::AttributePtr func)
{
  if (func && func->numberOfItems() == 2) // Kind of Hack
  {
    return dynamic_pointer_cast<ValueItem>(func->item(1));
  }
  return smtk::attribute::ValueItemPtr();
}

void qtSimpleExpressionView::onFuncSelectionChanged(
  QListWidgetItem* current,
  QListWidgetItem* /*previous*/)
{
  smtk::attribute::GroupItemPtr dataItem = this->getArrayDataFromItem(current);
  this->Internals->FuncTable->blockSignals(true);
  if (dataItem)
  {
    qtUIManager::updateArrayTableWidget(dataItem, this->Internals->FuncTable);
    this->Internals->FuncTable->resizeColumnsToContents();
    this->updateTableHeader();
  }
  else
  {
    this->Internals->FuncTable->clear();
    this->Internals->FuncTable->setRowCount(0);
    this->Internals->FuncTable->setColumnCount(0);
  }
  this->Internals->FuncTable->blockSignals(false);

  // Now set up the function editor UI

  smtk::attribute::ValueItemPtr expressionItem = this->getStringDataFromItem(current);
  this->updateFunctionEditorUI(expressionItem, dataItem);
}

void qtSimpleExpressionView::updateFunctionEditorUI(
  smtk::attribute::ValueItemPtr expressionItem,
  smtk::attribute::GroupItemPtr arrayItem)
{
  this->Internals->ExpressionInput->setText("");
  this->Internals->NumberBox->setValue(10);
  this->Internals->DeltaInput->setText("1.0");
  this->Internals->InitValueInput->setText("0.0");

  if (!expressionItem)
  {
    return;
  }
  this->Internals->ExpressionInput->setText(expressionItem->valueAsString().c_str());
  if (arrayItem)
  {
    int n = static_cast<int>(arrayItem->numberOfGroups());
    int m = static_cast<int>(arrayItem->numberOfItemsPerGroup());
    if (!m || !n)
    {
      return;
    }
    smtk::attribute::ValueItemPtr valueItem =
      dynamic_pointer_cast<ValueItem>(arrayItem->item(0, 0));
    smtk::attribute::DoubleItemPtr dItem = dynamic_pointer_cast<DoubleItem>(arrayItem->item(0, 0));
    smtk::attribute::IntItemPtr iItem = dynamic_pointer_cast<IntItem>(arrayItem->item(0, 0));

    if (valueItem && valueItem->numberOfValues())
    {
      int numValues = static_cast<int>(valueItem->numberOfValues());
      this->Internals->InitValueInput->setText(valueItem->valueAsString(0).c_str());
      this->Internals->DeltaInput->setText(valueItem->valueAsString(0).c_str());
      this->Internals->NumberBox->setValue(numValues);
      if (numValues > 1 && n == 2 && (dItem || iItem))
      {
        double deltaVal =
          dItem ? (dItem->value(1) - dItem->value(0)) : (iItem->value(1) - iItem->value(0));
        this->Internals->DeltaInput->setText(QString::number(deltaVal));
      }
    }
  }
}

void qtSimpleExpressionView::onFuncNameChanged(QListWidgetItem* item)
{
  smtk::attribute::AttributePtr func = this->getFunctionFromItem(item);
  if (func)
  {
    ResourcePtr attResource = func->definition()->resource();
    attResource->rename(func, item->text().toLatin1().constData());
  }
}

void qtSimpleExpressionView::onFuncValueChanged(QTableWidgetItem* item)
{
  smtk::attribute::GroupItemPtr dataItem = this->getSelectedArrayData();
  if (!dataItem)
  {
    return;
  }
  qtUIManager::updateArrayDataValue(dataItem, item);
  this->clearFuncExpression();
}

int qtSimpleExpressionView::getNumberOfComponents()
{
  if (!this->Internals->m_attDefinition)
  {
    return -1;
  }

  if (!this->Internals->m_attDefinition->numberOfItemDefinitions())
  {
    return -1;
  }

  auto itemDefinition = dynamic_pointer_cast<const GroupItemDefinition>(
    this->Internals->m_attDefinition->itemDefinition(0));

  if (!itemDefinition)
  {
    return -1;
  }
  return static_cast<int>(itemDefinition->numberOfItemDefinitions());
}

void qtSimpleExpressionView::onCreateNew()
{
  QStringList strVals;
  int numRows = this->Internals->NumberBox->value();
  int numberOfComponents = this->getNumberOfComponents();
  if (numberOfComponents == -1)
  {
    return; //This does not support expressions!
  }
  // If the Function Parser checkbox is checked, just emit a signal to
  // give the consumer (e.g. cmb ModelBuilder app) a chance to use its
  // function parser to create a function with the a string description.
  // NOTE:: qtSimpleExpressionView currently does not offer a function parser.
  if (this->Internals->EditorGroup->isChecked())
  {
    this->createFunctionWithExpression();
  }
  else
  {
    for (int i = 0; i < numRows; i++)
    {
      for (int c = 0; c < numberOfComponents - 1; c++)
      {
        strVals << "0.0"
                << "\t";
      }
      strVals << "0.0" << LINE_BREAKER_STRING
    }
    QString valuesText = strVals.join(" ");
    smtk::attribute::ValueItemPtr expressionItem =
      this->getStringDataFromItem(this->Internals->FuncList->currentItem());
    QString funcExp = expressionItem ? expressionItem->valueAsString().c_str() : "";

    this->buildSimpleExpression(funcExp, valuesText, numberOfComponents);
  }
}

void qtSimpleExpressionView::displayExpressionError(std::string& errorMsg, int errorPos)
{
  QString strMessage = QString(errorMsg.c_str()) +
    "\nThe function expression has some syntax error at entityref position: " +
    QString::number(errorPos);
  QMessageBox::warning(this->parentWidget(), tr("SimBuilder Functions"), strMessage);
  this->Internals->ExpressionInput->setFocus();
  this->Internals->ExpressionInput->setCursorPosition(errorPos);
}

void qtSimpleExpressionView::createFunctionWithExpression() {}

void qtSimpleExpressionView::createNewFunction(smtk::attribute::DefinitionPtr attDef)
{
  if (!attDef)
  {
    return;
  }
  this->Internals->FuncList->blockSignals(true);
  ResourcePtr attResource = attDef->resource();

  smtk::attribute::AttributePtr newFunc = attResource->createAttribute(attDef->type());
  this->attributeCreated(newFunc);
  QListWidgetItem* item = this->addFunctionListItem(newFunc);
  if (item)
  {
    this->Internals->FuncList->setCurrentItem(item);
  }
  this->Internals->FuncList->blockSignals(false);
}

void qtSimpleExpressionView::buildSimpleExpression(
  QString& funcExpr,
  QString& funcVals,
  int numberOfComponents)
{
  if (!this->Internals->m_attDefinition)
  {
    return;
  }

  this->createNewFunction(this->Internals->m_attDefinition);
  smtk::attribute::ValueItemPtr expressionItem =
    this->getStringDataFromItem(this->Internals->FuncList->currentItem());
  if (expressionItem && !funcExpr.isEmpty())
  {
    smtk::attribute::StringItemPtr sItem = dynamic_pointer_cast<StringItem>(expressionItem);
    if (sItem)
    {
      sItem->setValue(funcExpr.toStdString());
      this->Internals->ExpressionInput->setText(sItem->valueAsString().c_str());
    }
  }

  this->Internals->FuncTable->blockSignals(true);
  this->Internals->FuncTable->clear();
  this->Internals->FuncTable->setRowCount(0);
  this->Internals->FuncTable->setColumnCount(numberOfComponents);
  this->pasteFunctionValues(funcVals, false);
  this->updateTableHeader();
  this->Internals->FuncTable->blockSignals(false);
}

void qtSimpleExpressionView::updateTableHeader()
{
  if (this->Internals->FuncTable->columnCount() >= 2)
  {
    this->Internals->FuncTable->setHorizontalHeaderLabels(QStringList() << tr("x") << tr("f(x)"));
  }
}

void qtSimpleExpressionView::onCSVLoad()
{
  int numberOfComponents = this->getNumberOfComponents();
  if (numberOfComponents == -1)
  {
    return; //This does not support expressions!
  }

  QString fname = QFileDialog::getOpenFileName(
    this->Widget, tr("Open CSV File"), QString(), tr("CSV Files (*.csv);; All Files(*.*)"));
  if (fname == "")
  {
    return;
  }
  std::cout << "Got File\n";
  QFile f(fname);
  QString line;
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    // Should add error message!
    return;
  }
  QTextStream in(&f);
  int i;
  QStringList tableVals, rowVals;
  while (!in.atEnd())
  {
    line = in.readLine();
    QStringList vals = line.split(',');
    if (vals.size() != numberOfComponents)
    {
      continue;
    }
    // Clear the row of vals
    rowVals.clear();
    for (i = 0; i < numberOfComponents; i++)
    {
      // Is this a number - if not skip the row!
      bool ok;
      vals.at(i).toDouble(&ok);
      if (!ok)
      {
        rowVals.clear();
        break;
      }
      rowVals << vals.at(i);
      if (i < (numberOfComponents - 1))
      {
        rowVals << "\t";
      }
      else
      {
        rowVals << LINE_BREAKER_STRING;
      }
    }
    tableVals.append(rowVals);
  }
  if (!tableVals.empty())
  {
    QString tableString = tableVals.join(" ");
    QString dummy;
    this->buildSimpleExpression(dummy, tableString, numberOfComponents);
  }
}

void qtSimpleExpressionView::onCopySelected()
{
  smtk::attribute::AttributePtr dataItem = this->getSelectedFunction();
  if (dataItem && dataItem->numberOfItems())
  {
    smtk::attribute::GroupItemPtr groupItem = dynamic_pointer_cast<GroupItem>(dataItem->item(0));
    QString valuesText;
    if (groupItem && this->uiManager()->getExpressionArrayString(groupItem, valuesText))
    {
      smtk::attribute::ValueItemPtr expressionItem =
        this->getStringDataFromItem(this->Internals->FuncList->currentItem());
      QString funcExp = expressionItem ? expressionItem->valueAsString().c_str() : "";
      this->buildSimpleExpression(
        funcExp, valuesText, static_cast<int>(groupItem->numberOfItemsPerGroup()));
    }
  }
}

void qtSimpleExpressionView::onDeleteSelected()
{
  QListWidgetItem* selItem = this->getSelectedItem();
  if (selItem)
  {
    if (!this->Internals->m_attDefinition)
    {
      return;
    }

    smtk::attribute::ResourcePtr resource = this->attributeResource();
    resource->removeAttribute(this->getFunctionFromItem(selItem));

    this->Internals->FuncList->takeItem(this->Internals->FuncList->row(selItem));
  }
}

QListWidgetItem* qtSimpleExpressionView::addFunctionListItem(
  smtk::attribute::AttributePtr childData)
{
  if (!(this->uiManager()->passAdvancedCheck(childData->definition()->advanceLevel()) &&
        childData->isRelevant()))
  {
    return nullptr;
  }

  QListWidgetItem* item = nullptr;
  smtk::attribute::GroupItemPtr dataItem = this->getFunctionArrayData(childData);
  if (dataItem)
  {
    item = new QListWidgetItem(
      QString::fromUtf8(childData->name().c_str()), this->Internals->FuncList, smtk_USER_DATA_TYPE);
    QVariant vdata;
    vdata.setValue(static_cast<void*>(childData.get()));
    item->setData(Qt::UserRole, vdata);
    if (childData->definition()->advanceLevel())
    {
      item->setFont(this->uiManager()->advancedFont());
    }
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    this->Internals->FuncList->addItem(item);
  }
  return item;
}

void qtSimpleExpressionView::onFuncTableKeyPress(QKeyEvent* e)
{

  // Allow paste
  if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
  {
    QString values = qtUIManager::clipBoardText();
    this->pasteFunctionValues(values);
    e->accept();
    return;
  }
  // Allow copying
  if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
  {
    QStringList list;
    for (int r = 0; r < this->Internals->FuncTable->rowCount(); r++)
    {
      if (this->Internals->FuncTable->item(r, 0)->isSelected())
      {
        for (int c = 0; c < this->Internals->FuncTable->columnCount(); c++)
        {
          list << this->Internals->FuncTable->item(r, c)->text();
          if (c < this->Internals->FuncTable->columnCount() - 1)
          {
            list << "\t";
          }
        }
        list << LINE_BREAKER_STRING;
      }
    }

    QString tempText = list.join(" ");
    qtUIManager::setClipBoardText(tempText);

    e->accept();
    return;
  }

  // Allow delete
  if (e->key() == Qt::Key_Delete)
  {
    this->onRemoveSelectedValues();
    e->accept();
    return;
  }
}

void qtSimpleExpressionView::pasteFunctionValues(QString& str, bool clearExp)
{
  if (str.isNull())
  {
    return;
  }
  QString strSep = LINE_BREAKER_STRING;
  QStringList rows = str.split(strSep);

  int numRows = rows.count() - 1;
  int numColumns = rows.first().count('\t') + 1;
  QTableWidget* table = this->Internals->FuncTable;
  if (table->columnCount() != numColumns)
  {
    QMessageBox::warning(
      this->parentWidget(),
      tr("SimBuilder Functions"),
      tr("The information cannot be pasted because the copy "
         "and paste columns aren't the same size."));
    return;
  }

  // add all the pasted rows
  for (int i = 0; i < numRows; ++i)
  {
    QStringList columns = rows[i].split('\t');
    if (columns.count() != numColumns)
    {
      continue;
    }
    double* vals = new double[numColumns];
    for (int j = 0; j < numColumns; ++j)
    {
      vals[j] = columns[j].toDouble();
    }
    this->addNewValue(vals, numColumns);
    delete[] vals;
  }
  this->Internals->FuncTable->resizeColumnsToContents();
  if (clearExp)
  {
    this->clearFuncExpression();
  }
}

void qtSimpleExpressionView::onAddValue()
{
  int numVals = this->Internals->FuncTable->columnCount();
  double zero = 0.;
  std::vector<double> vals(numVals, zero);
  //  this->uiManager()->updateArrayDataValue(dataItem, item);
  this->addNewValue(vals.data(), numVals);
  this->clearFuncExpression();
}

void qtSimpleExpressionView::addNewValue(double* vals, int numVals)
{
  smtk::attribute::GroupItemPtr dataItem = this->getSelectedArrayData();
  if (!dataItem)
  {
    return;
  }
  this->Internals->FuncTable->blockSignals(true);
  qtUIManager::addNewTableValues(dataItem, this->Internals->FuncTable, vals, numVals);
  this->Internals->FuncTable->blockSignals(false);
}

void qtSimpleExpressionView::onRemoveSelectedValues()
{
  smtk::attribute::GroupItemPtr dataItem = this->getSelectedArrayData();
  if (!dataItem)
  {
    return;
  }
  this->Internals->FuncTable->blockSignals(true);
  qtUIManager::removeSelectedTableValues(dataItem, this->Internals->FuncTable);
  this->Internals->FuncTable->blockSignals(false);
  this->clearFuncExpression();
}

void qtSimpleExpressionView::onShowCategory()
{
  this->updateUI();
}

void qtSimpleExpressionView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }
  smtk::attribute::ResourcePtr resource = this->attributeResource();
  // There should be only 1 child component called Type
  if ((view->details().numberOfChildren() != 1) || (view->details().child(0).name() != "Att"))
  {
    return;
  }

  std::string defType;
  if (!view->details().child(0).attribute("Type", defType))
  {
    return;
  }
  this->Internals->m_attDefinition = resource->findDefinition(defType);
  std::vector<smtk::attribute::AttributePtr> result;
  resource->findAttributes(this->Internals->m_attDefinition, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  this->Internals->FuncList->blockSignals(true);
  this->Internals->FuncList->clear();
  for (it = result.begin(); it != result.end(); ++it)
  {
    this->addFunctionListItem(*it);
  }
  this->Internals->FuncList->blockSignals(false);

  if (this->Internals->FuncList->count())
  {
    this->Internals->FuncList->setCurrentRow(0);
  }
}

void qtSimpleExpressionView::clearFuncExpression()
{
  smtk::attribute::ValueItemPtr strItem = this->getSelectedStringData();
  if (strItem)
  {
    strItem->unset();
  }
  this->Internals->ExpressionInput->setText("");
}

void qtSimpleExpressionView::getAllDefinitions(QList<smtk::attribute::DefinitionPtr>& defs)
{
  if (this->Internals->m_attDefinition)
  {
    this->qtBaseAttributeView::getDefinitions(this->Internals->m_attDefinition, defs);
  }
}
