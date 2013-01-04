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
// .NAME qtSimpleExpressionSection - an Expression Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtSimpleExpressionSection_h
#define __smtk_attribute_qtSimpleExpressionSection_h

#include "smtk/Qt/qtSection.h"

#include <vector>

class qtSimpleExpressionSectionInternals;
class QListWidgetItem;
class QTableWidgetItem;
class QKeyEvent;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtSimpleExpressionSection : public qtSection
    {
      Q_OBJECT

    public:         
      qtSimpleExpressionSection(smtk::SectionPtr, QWidget* parent);
      virtual ~qtSimpleExpressionSection();  

      void buildSimpleExpression(
        QString& funcExpr, QString& funcVals, int numberOfComponents);
      virtual void createNewFunction(smtk::AttributeDefinitionPtr attDef);
      QListWidgetItem* getSelectedItem();
      void displayExpressionError(std::string& errorMsg, int errorPos);

    public slots:
      void onFuncSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      void onFuncValueChanged(QTableWidgetItem*);
      void onFuncNameChanged(QListWidgetItem*);
      void onCreateNew();
      void onCopySelected();
      void onDeleteSelected();
      void onFuncTableKeyPress(QKeyEvent* );
      void onAddValue();
      void onRemoveSelectedValues();

      virtual void createFunctionWithExpression();
      void showAdvanced(int show);

    signals:
      void onCreateFunctionWithExpression(
        QString& expression, double initVal, double deltaVal, int numVals);

    protected slots:
      virtual void updateAttributeData() 
      {this->initFunctionList();}
    
    protected:
      virtual void createWidget();
      void updateTableHeader();
      smtk::GroupItemPtr getArrayDataFromItem(QListWidgetItem * item);
      smtk::ValueItemPtr getStringDataFromItem(QListWidgetItem * item);
      smtk::AttributePtr getFunctionFromItem(QListWidgetItem * item);
      smtk::GroupItemPtr getSelectedArrayData();
      smtk::ValueItemPtr getSelectedStringData();
      smtk::AttributePtr getSelectedFunction();
      smtk::GroupItemPtr getFunctionArrayData(smtk::AttributePtr func);
      smtk::ValueItemPtr getFunctionStringData(smtk::AttributePtr func);
      QListWidgetItem* addFunctionListItem(smtk::AttributePtr childData);
      void addNewValue(double* vals, int numVals);
      void updateFunctionEditorUI(
        smtk::ValueItemPtr expressionItem, smtk::GroupItemPtr arrayItem);
      void pasteFunctionValues(QString& values, bool clearExp=true);
      virtual void initFunctionList();
      virtual void clearFuncExpression();
      virtual void getAllDefinitions(
        QList<smtk::AttributeDefinitionPtr>& defs);

    private:

      qtSimpleExpressionSectionInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
