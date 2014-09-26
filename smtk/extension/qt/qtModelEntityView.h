//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelEntityView - the ModelEntity View
// .SECTION Description
// .SECTION See Also
// qtBaseView

#ifndef __smtk_attribute_qtModelEntityView_h
#define __smtk_attribute_qtModelEntityView_h

#include "smtk/extension/qt/qtBaseView.h"

#include <vector>

class qtModelEntityViewInternals;
class QListWidgetItem;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtModelEntityView : public qtBaseView
    {
      Q_OBJECT

    public:
      qtModelEntityView(smtk::view::BasePtr, QWidget* p, qtUIManager* uiman);
      virtual ~qtModelEntityView();
      QListWidgetItem* getSelectedItem();
      const std::vector<smtk::attribute::DefinitionPtr> &attDefinitions() const;

    public slots:
      void updateModelItems();
      void onShowCategory();
      void onListBoxSelectionChanged(QListWidgetItem * , QListWidgetItem * );
      virtual void updateModelAssociation();

    protected:
      virtual void createWidget( );
      smtk::model::ItemPtr getSelectedModelItem();
      smtk::model::ItemPtr getModelItem(QListWidgetItem * item);
      QListWidgetItem* addModelItem(smtk::model::ItemPtr childData);
      bool isRegionDomain();

    private:

      qtModelEntityViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
