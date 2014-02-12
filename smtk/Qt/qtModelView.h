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
// .NAME qtModelView - a tree view of smtk model.
// .SECTION Description
// .SECTION Caveats

#ifndef _qtModelView_h
#define _qtModelView_h

#include <QtGui/QTreeView.h>
#include "smtk/QtSMTKExports.h"
#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

class QTSMTK_EXPORT qtModelView : public QTreeView
{
  Q_OBJECT

public:
  qtModelView(QWidget* p = NULL);
  ~qtModelView();

  smtk::model::QEntityItemModel* getModel();

public slots:
  void selectEntities(const QList<std::string>& selIds);

signals:
  void entitiesSelected(const smtk::util::UUIDs& ids);

protected:
  virtual void  selectionChanged (
    const QItemSelection & selected, const QItemSelection & deselected );
  virtual void selectionHelper(
  QEntityItemModel* qmodel,
    const QModelIndex& parent,
    const smtk::util::UUIDs& selEntities,
    QItemSelection& selItems);
  void expandToRoot(QEntityItemModel* qmodel, const QModelIndex& idx);
  void recursiveSelect (
   smtk::model::QEntityItemModel* qmodel, const QModelIndex& sel,
    smtk::util::UUIDs& ids);
};

  } // namespace model
} // namespace smtk

#endif // !_qtModelView_h
