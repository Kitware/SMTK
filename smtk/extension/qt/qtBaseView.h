//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtBaseView - a base class for all view types
// .SECTION Description

#ifndef __smtk_attribute_qtBaseView_h
#define __smtk_attribute_qtBaseView_h

#include <QObject>
#include "smtk/extension/qt/Exports.h"
#include "smtk/PublicPointerDefs.h"
#include <QList>

class qtBaseViewInternals;
class QScrollArea;

namespace smtk
{
  namespace attribute
  {
    class qtUIManager;
    class qtItem;

    class SMTKQTEXT_EXPORT qtBaseView : public QObject
    {
      Q_OBJECT

    public:
      qtBaseView(smtk::common::ViewPtr, QWidget* parent, qtUIManager* uiman);
      virtual ~qtBaseView();

      smtk::common::ViewPtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();
      qtUIManager* uiManager();
      // Description:
      // Determines if an item should be displayed
      virtual bool displayItem(smtk::attribute::ItemPtr);
      virtual void getDefinitions(smtk::attribute::DefinitionPtr attDef,
        QList<smtk::attribute::DefinitionPtr>& defs);
      int fixedLabelWidth();
      bool setFixedLabelWidth(int w);
      bool advanceLevelVisible()
        { return m_advOverlayVisible; }
      virtual int advanceLevel();
      virtual bool categoryEnabled()
      {return false;}
      virtual std::string currentCategory()
      {return "";}
      bool isTopLevel() const
      {return this->m_isTopLevel;}

    signals:
      void modified(smtk::attribute::ItemPtr);

    public slots:
      virtual void updateUI()
      {
      this->updateAttributeData();
      this->updateModelAssociation();
      this->showAdvanceLevelOverlay(m_advOverlayVisible);
      }
      virtual void updateModelAssociation() {;}
      virtual void valueChanged(smtk::attribute::ItemPtr);
      virtual void childrenResized(){;}
      virtual void showAdvanceLevelOverlay(bool val)
      { m_advOverlayVisible = val;}
      virtual void showAdvanceLevel(int i);
      virtual void updateViewUI(int /* currentTab */){}
      virtual void enableShowBy(int /* enable */){}
      virtual void onShowCategory(){}


      virtual void requestModelEntityAssociation() {;}

    protected slots:
      virtual void updateAttributeData() {;}
      virtual void onAdvanceLevelChanged(int levelIdx);

    protected:
      // Description:
      // Creates the UI related to the view and properly assigns it
      // to the parent widget.
      virtual void buildUI();
    protected:
      // Description:
      // Creates the main QT Widget that is associated with a View.  Typically this
      // is the only method a derived View needs to override.
      virtual void createWidget();

      // Description:
      // Adds properties associated with respects to a top level view
      virtual void makeTopLevel();

      QWidget* Widget;
      QScrollArea *m_ScrollArea;
      bool m_isTopLevel;
      bool m_topLevelInitialized;

    private:

      qtBaseViewInternals *Internals;
      bool m_advOverlayVisible;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
