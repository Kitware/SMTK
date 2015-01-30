//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_qt_testing_ModelBrowser_h
#define __smtk_extension_qt_testing_ModelBrowser_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Group.h"

#include <QtGui/QWidget>

class QTreeView;
class QModelIndex;

namespace smtk {
  namespace model {
    class QEntityItemModel;
    class QEntityItemDelegate;
  }
}

class ModelBrowser : public QWidget
{
  Q_OBJECT
public:
  ModelBrowser(QWidget* parent = NULL);
  virtual ~ModelBrowser();

  QTreeView* tree() const;

  void setup(
    smtk::model::ManagerPtr s,
    smtk::model::QEntityItemModel* qm,
    smtk::model::QEntityItemDelegate* qd,
    smtk::model::DescriptivePhrasePtr root);

public slots:
  virtual void addGroup();
  virtual void addToGroup();
  virtual void removeFromGroup();
  virtual void updateButtonStates(const QModelIndex& curr, const QModelIndex& prev);

protected:
  class Internals;
  Internals* m_p;
  smtk::model::ManagerPtr m_manager;

  smtk::model::Group groupParentOfIndex(const QModelIndex& qidx);
};

#endif // __smtk_extension_qt_testing_ModelBrowser_h
