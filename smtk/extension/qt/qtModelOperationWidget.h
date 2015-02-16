//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelOperationWidget - the Model Operations Widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtModelOperationWidget_h
#define __smtk_attribute_qtModelOperationWidget_h

#include <QWidget>
#include "smtk/extension/qt/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class qtModelOperationWidgetInternals;
class QAbstractButton;

namespace smtk {
 namespace attribute {
  class qtFileItem;
 }
}

namespace smtk
{
  namespace model
  {
    class QTSMTK_EXPORT qtModelOperationWidget : public QWidget
    {
      Q_OBJECT

    public:
      qtModelOperationWidget(QWidget* p = NULL);
      virtual ~qtModelOperationWidget();
      virtual void setSession(smtk::model::SessionPtr session);
      virtual QSize sizeHint() const;

    public slots:
      virtual bool setCurrentOperation(
        const std::string& opName, smtk::model::SessionPtr session);
      virtual bool setCurrentOperation(const smtk::model::OperatorPtr& brOp);
      virtual void expungeEntities(
        const smtk::model::EntityRefs& expungedEnts);

    signals:
      void operationRequested(const smtk::model::OperatorPtr& brOp);
      void fileItemCreated(smtk::attribute::qtFileItem* fileItem);

    protected slots:
      virtual void onOperationSelected();
      virtual void onOperate();

    protected:
      virtual void initWidget( );

    private:

      qtModelOperationWidgetInternals *Internals;

    }; // class
  }; // namespace model
}; // namespace smtk


#endif
