#ifndef __smtk_Qt_testing_ModelBrowser_h
#define __smtk_Qt_testing_ModelBrowser_h

#include "smtk/PublicPointerDefs.h"

#include <QtGui/QWidget>

class QTreeView;

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
    smtk::model::StoragePtr s,
    smtk::model::QEntityItemModel* qm,
    smtk::model::QEntityItemDelegate* qd,
    smtk::model::DescriptivePhrasePtr root);

public slots:
  virtual void addGroup();

protected:
  class Internals;
  Internals* m_p;
  smtk::model::StoragePtr m_storage;
};

#endif // __smtk_Qt_testing_ModelBrowser_h
