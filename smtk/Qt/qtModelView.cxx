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
  smtk::util::UUIDs ids;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    this->recursiveSelect(qmodel, sel, ids);
    }

  emit this->entitiesSelected(ids);
}

void qtModelView::recursiveSelect (
   smtk::model::QEntityItemModel* qmodel, const QModelIndex& sel,
    smtk::util::UUIDs& ids)
{
  DescriptivePhrase* dPhrase = qmodel->getItem(sel);
  if(dPhrase && ids.find(dPhrase->relatedEntityId()) == ids.end())
    ids.insert(dPhrase->relatedEntityId());

  for (int row=0; row < qmodel->rowCount(sel); ++row)
    {
    this->recursiveSelect(qmodel, qmodel->index(row, 0, sel), ids);
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectEntities(const QList<std::string>& selIds)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
  // Convert selection to UUIDs for faster membership checks:
  smtk::util::UUIDs selEntities;
  foreach(std::string strId, selIds)
    {
    smtk::util::UUID uid = smtk::util::UUID(strId);
    if (!uid.isNull())
      selEntities.insert(uid);
    }
  // Now recursively check which model indices should be selected:
  QItemSelection selItems;
  this->selectionHelper(qmodel, this->rootIndex(), selEntities, selItems);
  // If we have any items selected, show them
  if(selItems.count())
    {
    this->blockSignals(true);
    this->selectionModel()->select(selItems, QItemSelectionModel::ClearAndSelect);
    this->blockSignals(false);
    this->scrollTo(selItems.value(0).topLeft());
    }
}

void qtModelView::expandToRoot(QEntityItemModel* qmodel, const QModelIndex& idx)
{
  if(0)
  {
    std::cout << "idx isValid " << idx.isValid() << std::endl;
    DescriptivePhrase* dPhrase = qmodel->getItem(idx);
    if(dPhrase)
    {
      std::cout << "title " << dPhrase->title() << std::endl;
    }
  }
  if(idx.isValid())
    {
    this->setExpanded(idx, true);
    this->expandToRoot(qmodel, qmodel->parent(idx));
    }
}

void qtModelView::selectionHelper(
  QEntityItemModel* qmodel,
  const QModelIndex& parent,
  const smtk::util::UUIDs& selEntities,
  QItemSelection& selItems)
{
  // For all the children of this index, see if
  // each child should be selected and then queue its children.
  for (int row=0; row < qmodel->rowCount(parent); ++row)
    {
    QModelIndex idx(qmodel->index(row, 0, parent));
    DescriptivePhrase* dPhrase = qmodel->getItem(idx);
    if (dPhrase && selEntities.find(dPhrase->relatedEntityId()) != selEntities.end())
      {
      this->expandToRoot(qmodel, parent);
      QItemSelectionRange sr(idx);
      selItems.append(sr);
      }
    this->selectionHelper(qmodel, idx, selEntities, selItems);
    }
}
  } // namespace model
} // namespace smtk
