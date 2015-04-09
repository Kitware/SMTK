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
#include "smtk/extension/qt/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"
#include <QList>

class qtBaseViewInternals;

namespace smtk
{
  namespace attribute
  {
    class qtUIManager;
    class qtItem;

    class QTSMTK_EXPORT qtBaseView : public QObject
    {
      Q_OBJECT

    public:
      qtBaseView(smtk::view::BasePtr, QWidget* parent, qtUIManager* uiman);
      virtual ~qtBaseView();

      smtk::view::BasePtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();
      qtUIManager* uiManager();
      virtual void getDefinitions(smtk::attribute::DefinitionPtr attDef,
        QList<smtk::attribute::DefinitionPtr>& defs);
      int fixedLabelWidth();
      bool setFixedLabelWidth(int w);
      bool advanceLevelVisible()
        { return m_advOverlayVisible; }

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

      virtual void requestModelEntityAssociation() {;}

    protected slots:
      virtual void updateAttributeData() {;}

    protected:
      virtual void createWidget(){;}

      QWidget* Widget;
    private:

      qtBaseViewInternals *Internals;
      bool m_advOverlayVisible;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
