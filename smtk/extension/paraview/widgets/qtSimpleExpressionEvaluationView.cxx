//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/widgets/qtSimpleExpressionEvaluationView.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/extension/vtk/widgets/vtkSBFunctionParser.h"

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

#include "vtkDoubleArray.h"
#include "vtkNew.h"

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

qtBaseView* qtSimpleExpressionEvaluationView::createViewWidget(const smtk::view::Information& info)
{
  qtSimpleExpressionEvaluationView* view = new qtSimpleExpressionEvaluationView(info);
  view->buildUI();
  return view;
}

qtSimpleExpressionEvaluationView::qtSimpleExpressionEvaluationView(
  const smtk::view::Information& info)
  : qtSimpleExpressionView(info)
{
}

qtSimpleExpressionEvaluationView::~qtSimpleExpressionEvaluationView() = default;

void qtSimpleExpressionEvaluationView::createWidget()
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

void qtSimpleExpressionEvaluationView::createFunctionWithExpression()
{
  QString funcExpr = this->Internals->ExpressionInput->text();
  if (funcExpr.isEmpty())
  {
    funcExpr = "X";
  }
  this->Internals->ExpressionInput->setText(funcExpr);
  double initVal = this->Internals->InitValueInput->text().toDouble();
  double deltaVal = this->Internals->DeltaInput->text().toDouble();
  int numValues = this->Internals->NumberBox->value();

  int errorPos = -1;
  std::string errorMsg;
  vtkNew<vtkSBFunctionParser> FunctionParser;
  FunctionParser->SetFunction(funcExpr.toStdString());
  FunctionParser->CheckExpression(errorPos, errorMsg);
  if (errorPos != -1 && !errorMsg.empty())
  {
    this->displayExpressionError(errorMsg, errorPos);
    return;
  }

  FunctionParser->SetNumberOfValues(numValues);
  FunctionParser->SetInitialValue(initVal);
  FunctionParser->SetDelta(deltaVal);
  vtkDoubleArray* result = FunctionParser->GetResult();
  if (result)
  {
    int numberOfComponents = result->GetNumberOfComponents();
    std::vector<double> values(numberOfComponents);
    QStringList strVals;
    for (vtkIdType i = 0, j = 0; i < result->GetNumberOfTuples(); i++, j += numberOfComponents)
    {
      result->GetTypedTuple(i, values.data());
      for (int c = 0; c < numberOfComponents - 1; c++)
      {
        strVals << QString::number(values[c]) << "\t";
      }
      strVals << QString::number(values[numberOfComponents - 1]);
      strVals << LINE_BREAKER_STRING;
    }

    QString valuesText = strVals.join(" ");
    this->buildSimpleExpression(funcExpr, valuesText, numberOfComponents);
  }
}
