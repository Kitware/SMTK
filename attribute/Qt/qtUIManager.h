/*=========================================================================

  Module:    qtUIManager.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME qtUIManager - a user interface manager.
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_qtUIManager_h
#define __slctk_attribute_qtUIManager_h

#include "qtItem.h"
#include "attribute/Manager.h"
#include <QFont>
#include <QColor>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;

namespace slctk
{
  namespace attribute
  {
    class qtItem;
    class qtRootSection;
    class qtAttributeSection;
    class qtInstancedSection;
    class qtModelEntitySection;
    class qtSimpleExpressionSection;
    class qtGroupSection;
    class qtSection;
    
    class SLCTKATTRIBUTE_EXPORT qtUIManager : public QObject
    {

    Q_OBJECT

    friend class qtRootSection;

    public:
      // Get the global instace for the pqApplicationCore.
      static qtUIManager* instance();
      
      qtUIManager(slctk::attribute::Manager &manager);
      virtual ~qtUIManager();  

      void initializeUI(QWidget* pWidget);
      
      // Description:
      // Set/Get the color used for indicating items with default values
      void setDefaultValueColor(const QColor &color);
      QColor defaultValueColor() const
      {return this->DefaultValueColor;}
        
      qtRootSection* rootSection()
        {return this->RootSection;}
      static QString clipBoardText();
      static void setClipBoardText(QString& text);
      
      void clearRoot();
      
      const QFont& advancedFont()
        {return this->advFont;}
      bool showAdvanced()
      {  return this->ShowAdvanced;  }
      void setWidgetToDefaultValueColor(QWidget *widget,
        bool setToDefault);

    static void updateArrayTableWidget(slctk::GroupItemPtr dataItem, QTableWidget* widget);
    static void updateTableColRows(slctk::AttributeItemPtr dataItem,
      int col, QTableWidget* widget);
    
    static void updateArrayDataValue(slctk::GroupItemPtr dataItem, QTableWidgetItem* item);
    static void addNewTableValues(slctk::GroupItemPtr dataItem, 
      QTableWidget* table, double* vals, int numVals);
    static void removeSelectedTableValues(
      slctk::GroupItemPtr dataItem, QTableWidget* table);
      
    protected:

      void processRootSection(qtRootSection* qRootSection);
      void processAttributeSection(qtAttributeSection* sec);
      void processInstancedSection(qtInstancedSection* sec);
      void processModelEntitySection(qtModelEntitySection* sec);
      void processSimpleExpressionSection(qtSimpleExpressionSection* sec);
      void processGroupSection(qtGroupSection* sec);
      void processBasicSection(qtSection* sec);

      void setShowAdvanced(bool val)
      {  this->ShowAdvanced = val;  }
      bool ShowAdvanced;
      
    private:
      static qtUIManager* Instance;
      qtRootSection* RootSection;
      QFont advFont;
      QColor DefaultValueColor;
      slctk::attribute::Manager &m_AttManager;

    }; // class
  }; // namespace attribute
}; // namespace slctk

#endif
