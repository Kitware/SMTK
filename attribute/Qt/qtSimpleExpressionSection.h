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

#ifndef __slctk_attribute_qtSimpleExpressionSection_h
#define __slctk_attribute_qtSimpleExpressionSection_h

#include "qtSection.h"

#include <vector>

class qtSimpleExpressionSectionInternals;
class QListWidgetItem;
class QTableWidgetItem;
class QKeyEvent;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtSimpleExpressionSection : public qtSection
    {
      Q_OBJECT

    public:         
      qtSimpleExpressionSection(slctk::SectionPtr, QWidget* parent);
      virtual ~qtSimpleExpressionSection();  

      void buildSimpleExpression(
        QString& funcExpr, QString& funcVals, int numberOfComponents);
      virtual void createNewFunction(slctk::AttributeDefinitionPtr attDef);
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
      slctk::GroupItemPtr getArrayDataFromItem(QListWidgetItem * item);
      slctk::ValueItemPtr getStringDataFromItem(QListWidgetItem * item);
      slctk::AttributePtr getFunctionFromItem(QListWidgetItem * item);
      slctk::GroupItemPtr getSelectedArrayData();
      slctk::ValueItemPtr getSelectedStringData();
      slctk::AttributePtr getSelectedFunction();
      slctk::GroupItemPtr getFunctionArrayData(slctk::AttributePtr func);
      slctk::ValueItemPtr getFunctionStringData(slctk::AttributePtr func);
      QListWidgetItem* addFunctionListItem(slctk::AttributePtr childData);
      void addNewValue(double* vals, int numVals);
      void updateFunctionEditorUI(
        slctk::ValueItemPtr expressionItem, slctk::GroupItemPtr arrayItem);
      void pasteFunctionValues(QString& values, bool clearExp=true);
      virtual void initFunctionList();
      virtual void clearFuncExpression();
      virtual void getAllDefinitions(
        std::vector<slctk::AttributeDefinitionPtr>& defs);

    private:

      qtSimpleExpressionSectionInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace slctk

#endif
