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
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtSimpleExpressionView_h
#define __smtk_attribute_qtSimpleExpressionView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/Exports.h"

#include <vector>

class qtSimpleExpressionViewInternals;
class QListWidgetItem;
class QTableWidgetItem;
class QKeyEvent;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtSimpleExpressionView : public qtBaseView
    {
      Q_OBJECT

    public:
      static qtBaseView *createViewWidget(smtk::common::ViewPtr view, QWidget* p,
                                          qtUIManager* uiman);
      qtSimpleExpressionView(smtk::common::ViewPtr view, QWidget* parent,
                             qtUIManager* uiman);
      virtual ~qtSimpleExpressionView();

      void buildSimpleExpression(
        QString& funcExpr, QString& funcVals, int numberOfComponents);
      virtual void createNewFunction(smtk::attribute::DefinitionPtr attDef);
      QListWidgetItem* getSelectedItem();
      void displayExpressionError(std::string& errorMsg, int errorPos);
      smtk::attribute::AttributePtr getFunctionFromItem(QListWidgetItem * item);
      int getNumberOfComponents();

    public slots:
      void onFuncSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onFuncValueChanged(QTableWidgetItem*);
      void onFuncNameChanged(QListWidgetItem*);
      void onCreateNew();
      void onCSVLoad();
      void onCopySelected();
      void onDeleteSelected();
      void onFuncTableKeyPress(QKeyEvent* );
      void onAddValue();
      void onRemoveSelectedValues();

      virtual void createFunctionWithExpression();

    signals:
      void onCreateFunctionWithExpression(
        QString& expression, double initVal, double deltaVal, int numVals);

    protected slots:
      virtual void updateAttributeData()
      {this->initFunctionList();}

    protected:
      virtual void createWidget();
      void updateTableHeader();
      smtk::attribute::GroupItemPtr getArrayDataFromItem(QListWidgetItem * item);
      smtk::attribute::ValueItemPtr getStringDataFromItem(QListWidgetItem * item);
      smtk::attribute::GroupItemPtr getSelectedArrayData();
      smtk::attribute::ValueItemPtr getSelectedStringData();
      smtk::attribute::AttributePtr getSelectedFunction();
      smtk::attribute::GroupItemPtr getFunctionArrayData(smtk::attribute::AttributePtr func);
      smtk::attribute::ValueItemPtr getFunctionStringData(smtk::attribute::AttributePtr func);
      QListWidgetItem* addFunctionListItem(smtk::attribute::AttributePtr childData);
      void addNewValue(double* vals, int numVals);
      void updateFunctionEditorUI(
        smtk::attribute::ValueItemPtr expressionItem, smtk::attribute::GroupItemPtr arrayItem);
      void pasteFunctionValues(QString& values, bool clearExp=true);
      virtual void initFunctionList();
      virtual void clearFuncExpression();
      virtual void getAllDefinitions(
        QList<smtk::attribute::DefinitionPtr>& defs);

    private:

      qtSimpleExpressionViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
