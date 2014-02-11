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
//    std::cerr << "sel row " << sel.row() << "sel col " << sel.column() << "\n";
    ids.append(dPhrase->relatedEntityId());
    std::cout << "sel uuid " << dPhrase->relatedEntityId().toString() << " title "
      << dPhrase->title() << "\n";
    }

  emit this->entitiesSelected(ids);
}


//----------------------------------------------------------------------------
void qtModelView::selectEntities(const QList<std::string>& selIds)
{
//  QModelIndex root(this->rootIndex());
  QEntityItemModel* qmodel = this->getModel();
  QItemSelection selitems;

//std::cerr << "sel UUID " << selIds.value(0) <<"\n";
//std::cerr << "root rowcount " << qmodel->rowCount(root) <<"\n";
    DescriptivePhrase* rPhrase = qmodel->getItem(this->rootIndex());
//std::cerr << "root title " << rPhrase->title() <<"\n";
//std::cerr << "root UUID " << rPhrase->relatedEntityId().toString() <<"\n";

  for(int row=0; row<qmodel->rowCount(this->rootIndex()); ++row)
    {
    DescriptivePhrase* dPhrase = qmodel->getItem(
     qmodel->index(row, 0, this->rootIndex()));
//std::cout << "row UUID " << dPhrase->relatedEntityId().toString() <<"\n";
//std::cerr << "title " << dPhrase->title() <<"\n";

    if(selIds.contains(dPhrase->relatedEntityId().toString()))
      {
      std::cout << "sel UUID " << dPhrase->relatedEntityId().toString() << " title "
      << dPhrase->title() << "\n";
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
      std::cerr << "select row " << selitems.value(0).top() <<"\n";
    }
}

  } // namespace model
} // namespace smtk
