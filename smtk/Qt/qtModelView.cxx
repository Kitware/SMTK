#include "smtk/Qt/qtModelView.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"

// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {


//-----------------------------------------------------------------------------
qtModelView::qtModelView(QWidget* p)
  : QTreeView(p)
{

}

//-----------------------------------------------------------------------------
qtModelView::~qtModelView()
{
}

//-----------------------------------------------------------------------------
smtk::model::QEntityItemModel* qtModelView::getModel()
{
  return qobject_cast<QEntityItemModel*>(this->model());
}

//-----------------------------------------------------------------------------
void qtModelView::selectionChanged (
    const QItemSelection & selected, const QItemSelection & deselected )
{
  smtk::model::QEntityItemModel* qmodel = this->getModel();
  if(!qmodel)
    {
    return;
    }
  QTreeView::selectionChanged(selected, deselected);
  QList<smtk::util::UUID> ids;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    DescriptivePhrase* dPhrase = qmodel->getItem(sel);
    ids.append(dPhrase->relatedEntityId());
    }

  emit this->entitiesSelected(ids);
}

  } // namespace model
} // namespace smtk
