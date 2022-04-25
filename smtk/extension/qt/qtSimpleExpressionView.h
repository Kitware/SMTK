//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtSimpleExpressionView - UI components for the attribute Expression View
// without function evaluation
// .SECTION Description
// .SECTION See Also
// qtBaseAttributeView

#ifndef smtk_extension_qtSimpleExpressionView_h
#define smtk_extension_qtSimpleExpressionView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <vector>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QGroupBox;
class QSpinBox;
class QTableWidgetItem;
class QKeyEvent;

namespace smtk
{
namespace extension
{
class qtTableWidget;
class SMTKQTEXT_EXPORT qtSimpleExpressionView : public qtBaseAttributeView
{
  class SMTKQTEXT_EXPORT qtSimpleExpressionViewInternals
  {
  public:
    qtSimpleExpressionViewInternals() { this->FunctionParserDescription = nullptr; }

    ~qtSimpleExpressionViewInternals();

    const char* getFunctionParserDescription();

    smtk::extension::qtTableWidget* FuncTable;
    QListWidget* FuncList;
    QPushButton* AddButton;
    QPushButton* DeleteButton;
    QPushButton* CopyButton;
    QPushButton* LoadCSVButton;

    QSpinBox* NumberBox;
    QLineEdit* ExpressionInput;
    QLineEdit* DeltaInput;
    QLineEdit* InitValueInput;
    QPushButton* AddValueButton;
    QPushButton* RemoveValueButton;
    QGroupBox* EditorGroup;

    char* FunctionParserDescription;
    smtk::attribute::DefinitionPtr m_attDefinition;
  };
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtSimpleExpressionView(const smtk::view::Information& info);
  ~qtSimpleExpressionView() override;

  void buildSimpleExpression(QString& funcExpr, QString& funcVals, int numberOfComponents);
  virtual void createNewFunction(smtk::attribute::DefinitionPtr attDef);
  QListWidgetItem* getSelectedItem();
  void displayExpressionError(std::string& errorMsg, int errorPos);
  smtk::attribute::AttributePtr getFunctionFromItem(QListWidgetItem* item);
  int getNumberOfComponents();

public Q_SLOTS:
  void onFuncSelectionChanged(QListWidgetItem*, QListWidgetItem*);
  void onFuncValueChanged(QTableWidgetItem*);
  void onFuncNameChanged(QListWidgetItem*);
  void onCreateNew();
  void onCSVLoad();
  void onCopySelected();
  void onDeleteSelected();
  void onFuncTableKeyPress(QKeyEvent*);
  void onAddValue();
  void onRemoveSelectedValues();

  virtual void createFunctionWithExpression();
  void updateUI() override;
  void onShowCategory() override;

protected Q_SLOTS:

protected:
  void createWidget() override;
  void updateTableHeader();
  smtk::attribute::GroupItemPtr getArrayDataFromItem(QListWidgetItem* item);
  smtk::attribute::ValueItemPtr getStringDataFromItem(QListWidgetItem* item);
  smtk::attribute::GroupItemPtr getSelectedArrayData();
  smtk::attribute::ValueItemPtr getSelectedStringData();
  smtk::attribute::AttributePtr getSelectedFunction();
  smtk::attribute::GroupItemPtr getFunctionArrayData(smtk::attribute::AttributePtr func);
  smtk::attribute::ValueItemPtr getFunctionStringData(smtk::attribute::AttributePtr func);
  QListWidgetItem* addFunctionListItem(smtk::attribute::AttributePtr childData);
  void addNewValue(double* vals, int numVals);
  void updateFunctionEditorUI(
    smtk::attribute::ValueItemPtr expressionItem,
    smtk::attribute::GroupItemPtr arrayItem);
  void pasteFunctionValues(QString& values, bool clearExp = true);
  virtual void clearFuncExpression();
  virtual void getAllDefinitions(QList<smtk::attribute::DefinitionPtr>& defs);

  qtSimpleExpressionViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
