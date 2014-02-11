#include "smtk/Qt/qtModelView.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"
#include <iomanip>
// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {


//-----------------------------------------------------------------------------
qtModelView::qtModelView(QWidget* p)
  : QTreeView(p)
{
  this->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

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


//----------------------------------------------------------------------------
void qtModelView::selectEntities(const QList<std::string>& selIds)
{
//  QModelIndex root(this->rootIndex());
  QEntityItemModel* qmodel = this->getModel();
  QItemSelection selitems;

    DescriptivePhrase* rPhrase = qmodel->getItem(this->rootIndex());
  for(int row=0; row<qmodel->rowCount(this->rootIndex()); ++row)
    {
    DescriptivePhrase* dPhrase = qmodel->getItem(
     qmodel->index(row, 0, this->rootIndex()));
    if(selIds.contains(dPhrase->relatedEntityId().toString()))
      {
      QItemSelectionRange sr(qmodel->index(row, 0, this->rootIndex()));
      selitems.append(sr);
      }
    }
  if(selitems.count())
    {
    this->blockSignals(true);
    this->selectionModel()->select(selitems, QItemSelectionModel::ClearAndSelect);
    this->blockSignals(false);
    this->scrollTo(selitems.value(0).topLeft());
    }
}

  } // namespace model
} // namespace smtk
