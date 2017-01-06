//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME qtUIManager - The user interface manager for smtk
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_qtUIManager_h
#define __smtk_extension_qtUIManager_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h" // Needed for ViewInfo definition
#include "smtk/attribute/System.h"
#include <map>
#include <QFont>
#include <QColor>
#include <QTextEdit>
#include <QMap>
#include <QPointer>

class QTableWidget;
class QTableWidgetItem;
class QComboBox;

namespace smtk
{
  namespace extension
  {
    class qtItem;
    class qtFileItem;
    class qtMeshSelectionItem;
    class qtModelEntityItem;
    class qtBaseView;
    class qtModelView;

    typedef qtBaseView* (*widgetConstructor)(const ViewInfo &info);

    class SMTKQTEXT_EXPORT qtUIManager : public QObject
    {

    Q_OBJECT

    public:
    qtUIManager(smtk::attribute::System &system);
      virtual ~qtUIManager();

      void initializeUI(QWidget* pWidget, bool useInternalFileBrowser=false);
      void initializeUI(const smtk::extension::ViewInfo &v,
			bool useInternalFileBrowser=false);
      qtBaseView *setSMTKView(smtk::common::ViewPtr v);
      qtBaseView *setSMTKView(smtk::common::ViewPtr v, QWidget* pWidget,
                              bool useInternalFileBrowser=false);
      qtBaseView *setSMTKView(const smtk::extension::ViewInfo &v,
                              bool useInternalFileBrowser=false);
      smtk::common::ViewPtr smtkView() const
      {return this->m_smtkView;}

      smtk::attribute::System* attSystem() const
      {return &this->m_AttSystem;}

      void setActiveModelView(smtk::extension::qtModelView*);
      smtk::extension::qtModelView* activeModelView();

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
      void setAdvanceFontStyleBold(bool val);
      bool advanceFontStyleBold() const;
      void setAdvanceFontStyleItalic(bool val);
      bool advanceFontStyleItalic() const;

      void setAdvancedBold(bool b)
      {this->AdvancedBold = b;}
      bool advancedBold()
      {return this->AdvancedBold;}
      void setAdvancedItalic(bool i)
      {this->AdvancedItalic = i;}
      bool advancedItalic()
      {return this->AdvancedItalic;}

      //Description:
      // Set and Get Value Label Lengths
      int maxValueLabelLength() const
      {return this->m_maxValueLabelLength;}
      void setMaxValueLabelLength(int w)
      {this->m_maxValueLabelLength = w;}
      int minValueLabelLength() const
      {return this->m_minValueLabelLength;}
      void setMinValueLabelLength(int w)
      {this->m_minValueLabelLength = w;}

      //Description:
      // Registers a view construction function with a view type string
      void registerViewConstructor(const std::string &vtype, widgetConstructor f);
      //Description:
      // Check if view type string has a registered view construction function
      bool hasViewConstructor(const std::string &vtype)
      { return this->m_constructors.find(vtype) != this->m_constructors.end(); }

      qtBaseView* topView()
        {return this->m_topView;}
      static QString clipBoardText();
      static void setClipBoardText(QString& text);
      std::string currentCategory();
      bool categoryEnabled();
      void clearRoot();

      bool passAdvancedCheck(int level);
      bool passAttributeCategoryCheck(smtk::attribute::ConstDefinitionPtr AttDef);
      bool passItemCategoryCheck(smtk::attribute::ConstItemDefinitionPtr ItemDef);
      bool passCategoryCheck(const std::set<std::string> & categories);

      const QFont& advancedFont()
        {return this->advFont;}
      int advanceLevel() const
      {return this->m_currentAdvLevel;}
      void initAdvanceLevels(QComboBox* combo);

      void setWidgetColorToInvalid(QWidget *widget);
      void setWidgetColorToDefault(QWidget *widget);
      void setWidgetColorToNormal(QWidget *widget);
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


    virtual int getWidthOfAttributeMaxLabel(smtk::attribute::DefinitionPtr def,
                                     const QFont &font);
    virtual int getWidthOfItemsMaxLabel(
      const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
      const QFont &font);

    qtBaseView* createView(const ViewInfo &info);

#ifdef _WIN32
    #define LINE_BREAKER_STRING "\n";
#else
    #define LINE_BREAKER_STRING "\r";
#endif

    public slots:
      void onFileItemCreated(smtk::extension::qtFileItem*);
      void onModelEntityItemCreated(smtk::extension::qtModelEntityItem*);
      void onMeshSelectionItemCreated(smtk::extension::qtMeshSelectionItem*);
      void updateModelViews();
      void onViewUIModified(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
      void setAdvanceLevel(int b);

    signals:
      void fileItemCreated(smtk::extension::qtFileItem* fileItem);
      void modelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
      void meshSelectionItemCreated(smtk::extension::qtMeshSelectionItem*);
      void viewUIChanged(smtk::extension::qtBaseView*, smtk::attribute::ItemPtr);
      void entitiesSelected(const smtk::common::UUIDs&);

    friend class qtRootView;
    friend class qtAssociationWidget;

    protected slots:
      void invokeEntitiesSelected(const smtk::common::UUIDs& uuids)
        { emit this->entitiesSelected(uuids); }

    protected:
      virtual void internalInitialize();

   private:
      qtBaseView* m_topView;
      smtk::common::ViewPtr m_smtkView;
      QWidget *m_parentWidget;
      qtModelView* m_activeModelView;

      QFont advFont;
      QColor DefaultValueColor;
      QColor InvalidValueColor;
      bool AdvancedBold; // true by default
      bool AdvancedItalic; // false by default

      smtk::attribute::System &m_AttSystem;
      bool m_useInternalFileBrowser;

      int m_maxValueLabelLength;
      int m_minValueLabelLength;

      // current advance level to show advanced attributes/items
      int m_currentAdvLevel;

      // map for <Definition, its longest item label>
      // The longest label is used as a hint when aligning all item labels
      QMap<smtk::attribute::DefinitionPtr, std::string> Def2LongLabel;
      void findDefinitionsLongLabels();
      void findDefinitionLongLabel(smtk::attribute::DefinitionPtr def, std::string &labelText);
      void getItemsLongLabel(
        const QList<smtk::attribute::ItemDefinitionPtr>& itemDefs,
        std::string &labelText);
      std::map<std::string, widgetConstructor> m_constructors;

    }; // class

    //A sublcass of QTextEdit to give initial sizehint
    class SMTKQTEXT_EXPORT qtTextEdit : public QTextEdit
      {
      Q_OBJECT
      public:
        qtTextEdit(QWidget * parent);
        virtual QSize sizeHint() const;
      };

  }; // namespace extension
}; // namespace smtk

#endif
