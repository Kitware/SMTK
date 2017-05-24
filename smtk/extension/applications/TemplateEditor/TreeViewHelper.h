#ifndef TreeViewHelper_h
#define TreeViewHelper_h

#include <QAbstractItemModel>
#include <QTreeView>

namespace TreeViewHelper
{
/**
   * Expand all items of the tree using match(). Do not use, instead
   * use QTreeView::expandAll().  This only serves as an example of the use
   * of match(), which could be used for searching the tree.
   */
inline void expandTree(QTreeView* view)
{
  QAbstractItemModel const* model = view->model();
  QModelIndex const firstNode = model->index(0, 0, QModelIndex());
  auto matchedIndices =
    model->match(firstNode, Qt::DisplayRole, "*", -1, Qt::MatchWildcard | Qt::MatchRecursive);

  for (const auto& index : matchedIndices)
  {
    view->expand(index);
  }
}
};

#endif // TreeViewHelper_h
