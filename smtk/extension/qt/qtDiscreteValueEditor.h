//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtDiscreteValueEditor - an item for display discrete value item
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_qtDiscreteValueEditor_h
#define __smtk_attribute_qtDiscreteValueEditor_h

#include "smtk/extension/qt/qtItem.h"
#include "smtk/extension/qt/Exports.h"
#include <QtGui/QWidget>

class qtDiscreteValueEditorInternals;

namespace smtk
{
  namespace extension
  {
    class SMTKQTEXT_EXPORT qtDiscreteValueEditor : public QWidget
    {
      Q_OBJECT

    public:
      qtDiscreteValueEditor(smtk::attribute::ItemPtr, int elementIdx,
        QWidget* parent, qtBaseView* bview, QLayout* childLayout);
      virtual ~qtDiscreteValueEditor();
      virtual QSize sizeHint() const;

    public slots:
      void onInputValueChanged();

    protected slots:
      virtual void updateItemData();

    protected:
      virtual void createWidget();

    private:
      qtDiscreteValueEditorInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
