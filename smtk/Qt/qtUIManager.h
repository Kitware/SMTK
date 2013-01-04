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

#ifndef __smtk_attribute_qtUIManager_h
#define __smtk_attribute_qtUIManager_h

#include "smtk/Qt/qtItem.h"
#include "smtk/attribute/Manager.h"
#include <QFont>
#include <QColor>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;

namespace smtk
{
  namespace attribute
  {
    class qtItem;
    class qtFileItem;
    class qtRootSection;
    class qtAttributeSection;
    class qtInstancedSection;
    class qtModelEntitySection;
    class qtSimpleExpressionSection;
    class qtGroupSection;
    class qtSection;
   
    class SMTKCORE_EXPORT qtUIManager : public QObject
    {

    Q_OBJECT

    friend class qtRootSection;

    public:
      // Get the global instace for the pqApplicationCore.
      static qtUIManager* instance();
      
      qtUIManager(smtk::attribute::Manager &manager);
      virtual ~qtUIManager();  

      void initializeUI(QWidget* pWidget);
      smtk::attribute::Manager* attManager() const
        {return &this->m_AttManager;}

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

      bool passItemAdvancedCheck(bool advancedItem);
      bool passAttributeAdvancedCheck(bool advancedAtt);
      const QFont& advancedFont()
        {return this->advFont;}
      bool showAdvanced()
      {  return this->ShowAdvanced;  }
      void setWidgetToDefaultValueColor(QWidget *widget,
        bool setToDefault);
      bool getExpressionArrayString(
        smtk::GroupItemPtr dataItem, QString& strValues);

    static void updateArrayTableWidget(smtk::GroupItemPtr dataItem, QTableWidget* widget);
    static void updateTableColRows(smtk::AttributeItemPtr dataItem,
      int col, QTableWidget* widget);
    
    static void updateArrayDataValue(smtk::GroupItemPtr dataItem, QTableWidgetItem* item);
    static void addNewTableValues(smtk::GroupItemPtr dataItem, 
      QTableWidget* table, double* vals, int numVals);
    static void removeSelectedTableValues(
      smtk::GroupItemPtr dataItem, QTableWidget* table);

    std::string getValueItemCommonLabel(smtk::ValueItemPtr vitem) const;
    std::string getGroupItemCommonLabel(smtk::GroupItemPtr groupitem) const;
    std::string getItemCommonLabel(smtk::AttributeItemPtr attItem);
    bool updateTableItemCheckState(
      QTableWidgetItem* labelitem, smtk::AttributeItemPtr attItem);

    virtual QWidget* createInputWidget(smtk::AttributeItemPtr,int elementIdx, QWidget* pWidget);
    virtual QWidget* createEditBox(smtk::AttributeItemPtr,int elementIdx, QWidget* pWidget);
    virtual QWidget* createComboBox(smtk::AttributeItemPtr,int elementIdx, QWidget* pWidget);
    virtual QWidget* createExpressionRefWidget(smtk::AttributeItemPtr,int elementIdx,QWidget* pWidget);


#ifdef WIN32
    #define LINE_BREAKER_STRING "\n";
#else
    #define LINE_BREAKER_STRING "\r";
#endif

    public slots:
      void onFileItemCreated(smtk::attribute::qtFileItem*);
      void onInputValueChanged();
      void onComboIndexChanged();
      void onExpressionReferenceChanged();

    signals:
      void fileItemCreated(smtk::attribute::qtFileItem* fileItem);

    protected:
      static void processAttributeSection(qtAttributeSection* sec);
      static void processInstancedSection(qtInstancedSection* sec);
      static void processModelEntitySection(qtModelEntitySection* sec);
      static void processSimpleExpressionSection(qtSimpleExpressionSection* sec);
      static void processGroupSection(qtGroupSection* sec);
      static void processBasicSection(qtSection* sec);

      void setShowAdvanced(bool val)
      {  this->ShowAdvanced = val;  }
      bool ShowAdvanced;
      
    private:
      static qtUIManager* Instance;
      qtRootSection* RootSection;
      QFont advFont;
      QColor DefaultValueColor;
      //smtk::attribute::Manager &m_AttManager;
      smtk::attribute::Manager &m_AttManager;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
