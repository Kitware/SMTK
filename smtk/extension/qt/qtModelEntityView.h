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
