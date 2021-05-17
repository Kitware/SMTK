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

#include <QWidget>

class QTreeView;
class QModelIndex;

namespace smtk
{
namespace extension
{
class qtDescriptivePhraseModel;
class qtDescriptivePhraseDelegate;
} // namespace extension
} // namespace smtk

class ModelBrowser : public QWidget
{
  Q_OBJECT
public:
  ModelBrowser(QWidget* parent = nullptr);
  virtual ~ModelBrowser();

  QTreeView* tree() const;

  void setup(
    smtk::resource::ManagerPtr mgr,
    smtk::extension::qtDescriptivePhraseModel* qm,
    smtk::extension::qtDescriptivePhraseDelegate* qd);

public slots:
  virtual void addGroup();
  virtual void addToGroup();
  virtual void removeFromGroup();
  virtual void updateButtonStates(const QModelIndex& curr, const QModelIndex& prev);

protected:
  class Internals;
  Internals* m_p;
  smtk::resource::ManagerPtr m_manager;

  smtk::model::Group groupParentOfIndex(const QModelIndex& qidx);
};

#endif // __smtk_extension_qt_testing_ModelBrowser_h
