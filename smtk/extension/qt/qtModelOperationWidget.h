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

#ifndef __smtk_extension_qtModelOperationWidget_h
#define __smtk_extension_qtModelOperationWidget_h

#include <QWidget>
#include "smtk/extension/qt/Exports.h"
#include "smtk/PublicPointerDefs.h"

class qtModelOperationWidgetInternals;
class QAbstractButton;

namespace smtk {
 namespace extension {
  class qtFileItem;
  class qtModelEntityItem;
  class qtMeshSelectionItem;
  class qtModelView;

  class SMTKQTEXT_EXPORT qtModelOperationWidget : public QWidget
  {
    Q_OBJECT

  public:
    qtModelOperationWidget(QWidget* p = NULL);
    virtual ~qtModelOperationWidget();
    virtual void setSession(smtk::model::SessionPtr session);
    virtual QSize sizeHint() const;
    virtual qtModelView* modelView();
    virtual smtk::model::OperatorPtr existingOperator(const std::string& opname);

  public slots:
    virtual bool setCurrentOperator(
      const std::string& opName, smtk::model::SessionPtr session);
    virtual bool initOperatorUI(const smtk::model::OperatorPtr& brOp);
    virtual void expungeEntities(
      const smtk::model::EntityRefs& expungedEnts);
    virtual void onOperate();

  signals:
    void operationRequested(const smtk::model::OperatorPtr& brOp);
    void operationCancelled(const smtk::model::OperatorPtr& brOp);
    void fileItemCreated(smtk::attribute::qtFileItem* fileItem);
    void modelEntityItemCreated(smtk::attribute::qtModelEntityItem* entItem);
    void meshSelectionItemCreated(
        smtk::attribute::qtMeshSelectionItem* meshItem,
        const std::string& opName, const smtk::common::UUID& uuid);
    void entitiesSelected(const smtk::common::UUIDs&);

  friend class qtModelView;

  protected slots:
    virtual void onOperationSelected();
    virtual void cancelCurrentOperator();
    virtual void cancelOperator(const std::string& opName);
    virtual void onMeshSelectionItemCreated(smtk::attribute::qtMeshSelectionItem*);
    virtual bool checkExistingOperator(const std::string& opName);

  protected:
    virtual void initWidget( );
    void setModelView(qtModelView* mv);

  private:

    qtModelOperationWidgetInternals *Internals;

  }; // class
  }; // namespace extension
}; // namespace smtk


#endif
