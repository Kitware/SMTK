//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelView - UI components for the attribute Model View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtModelView_h
#define __smtk_attribute_qtModelView_h

#include "smtk/extension/qt/qtBaseView.h"

#include <vector>

class qtModelViewInternals;
class QListWidgetItem;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtModelView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtModelView(smtk::view::BasePtr, QWidget* p, qtUIManager* uiman);
      virtual ~qtModelView();
      QListWidgetItem* getSelectedItem();
      const std::vector<smtk::attribute::DefinitionPtr> &attDefinitions() const;

    public slots:
      void updateModelEntityItems();
      void onShowCategory();
      void onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      virtual void updateModelAssociation();

    protected:
      virtual void createWidget( );
      smtk::model::ItemPtr getSelectedModelEntityItem();
      smtk::model::ItemPtr getModelEntityItem(QListWidgetItem * item);
      QListWidgetItem* addModelEntityItem(smtk::model::ItemPtr childData);
      bool isRegionDomain();

    private:

      qtModelViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
