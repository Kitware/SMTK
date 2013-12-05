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
#include <QDoubleValidator>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;

namespace smtk
{
  namespace attribute
  {
    class qtItem;
    class qtFileItem;
    class qtRootView;
    class qtAttributeView;
    class qtBaseView;
    class qtInstancedView;
    class qtModelEntityView;
    class qtSimpleExpressionView;
    class qtGroupView;

    class QTSMTK_EXPORT qtUIManager : public QObject
    {

    Q_OBJECT

    friend class qtRootView;

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
      void setInvalidValueColor(const QColor &color);
      QColor invalidValueColor() const
      {return this->InvalidValueColor;}

      // Description:
      // Set the advanced values font to be bold and/or italic
      void setAdvancedBold(bool b)
      {this->AdvancedBold = b;}
      bool advancedBold()
      {return this->AdvancedBold;}
      void setAdvancedItalic(bool i)
      {this->AdvancedItalic = i;}
      bool advancedItalic()
      {return this->AdvancedItalic;}

      qtRootView* rootView()
        {return this->RootView;}
      static QString clipBoardText();
      static void setClipBoardText(QString& text);

      void clearRoot();

      bool passItemAdvancedCheck(bool advancedItem);
      bool passAttributeAdvancedCheck(bool advancedAtt);
      const QFont& advancedFont()
        {return this->advFont;}

      bool passItemCategoryCheck(){ return true;}
      bool passAttributeCategoryCheck() {return true;}

      void setWidgetToDefaultValueColor(QWidget *widget,
        bool setToDefault);
      bool getExpressionArrayString(
        smtk::attribute::GroupItemPtr dataItem, QString& strValues);

    static void updateArrayTableWidget(smtk::attribute::GroupItemPtr dataItem,
                                       QTableWidget* widget);
    static void updateTableColRows(smtk::attribute::ItemPtr dataItem,
      int col, QTableWidget* widget);

    static void updateArrayDataValue(smtk::attribute::GroupItemPtr dataItem,
                                     QTableWidgetItem* item);
    static void addNewTableValues(smtk::attribute::GroupItemPtr dataItem,
      QTableWidget* table, double* vals, int numVals);
    static void removeSelectedTableValues(
      smtk::attribute::GroupItemPtr dataItem, QTableWidget* table);

    bool updateTableItemCheckState(
      QTableWidgetItem* labelitem, smtk::attribute::ItemPtr attItem);

    virtual QWidget* createInputWidget(smtk::attribute::ItemPtr,
                                       int elementIdx, QWidget* pWidget);
    virtual QWidget* createEditBox(smtk::attribute::ItemPtr,
                                   int elementIdx, QWidget* pWidget);
    virtual QWidget* createComboBox(smtk::attribute::ItemPtr,
                                    int elementIdx, QWidget* pWidget);
    virtual QWidget* createExpressionRefWidget(smtk::attribute::ItemPtr,
                                               int elementIdx,QWidget* pWidget);


#ifdef _WIN32
    #define LINE_BREAKER_STRING "\n";
#else
    #define LINE_BREAKER_STRING "\r";
#endif

    public slots:
      void onFileItemCreated(smtk::attribute::qtFileItem*);
      void onInputValueChanged();
      void onComboIndexChanged();
      void onExpressionReferenceChanged();
      void updateModelViews();

    signals:
      void fileItemCreated(smtk::attribute::qtFileItem* fileItem);

    protected:
      static void processAttributeView(qtAttributeView* v);
      static void processInstancedView(qtInstancedView* v);
      static void processModelEntityView(qtModelEntityView* v);
      static void processSimpleExpressionView(qtSimpleExpressionView* v);
      static void processGroupView(qtGroupView* v);
      static void processBasicView(qtBaseView* v);

    private:
      static qtUIManager* Instance;
      qtRootView* RootView;
      QFont advFont;
      QColor DefaultValueColor;
      QColor InvalidValueColor;
      bool AdvancedBold; // true by default
      bool AdvancedItalic; // false by default

      smtk::attribute::Manager &m_AttManager;

    }; // class

    //A sublcass of QDoubleValidator to fixup input outside of range
    class QTSMTK_EXPORT qtDoubleValidator : public QDoubleValidator
    {
      Q_OBJECT
    public:
        qtDoubleValidator(QObject * parent);
        virtual void fixup(QString &input) const;
    };

  }; // namespace attribute
}; // namespace smtk

#endif
