#ifndef __smtk_qt_qtEntityItemModel_h
#define __smtk_qt_qtEntityItemModel_h

#include "QAbstractItemModel"

#include "smtk/PublicPointerDefs.h"
#include "smtk/QtSMTKExports.h" // For EXPORT macro.
#include "smtk/util/UUID.h"

class QTSMTK_EXPORT QEntityItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QEntityItemModel(smtk::model::StoragePtr model, QObject* parent = 0);
  virtual ~QEntityItemModel();

  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual bool hasChildren(const QModelIndex& parent) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const { return 3; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  Qt::ItemFlags flags(const QModelIndex& index) const;

  template<typename T>
  void setSubset(const T& subset)
    {
    m_subset.clear();
    m_reverse.clear();
    typename T::const_iterator it = subset.begin();
    for (int i = 0; it != subset.end(); ++it, ++i)
      {
      m_subset.push_back(*it);
      m_reverse[*it] = i;
      }
    }

  void setDeleteOnRemoval(bool del)
    {
    this->m_deleteOnRemoval = del;
    }

protected:
  smtk::model::StoragePtr m_storage;
  smtk::util::UUIDArray m_subset; // *ordered* subset of m_storage being presented
  std::map<smtk::util::UUID,int> m_reverse; // lookup from UUID into m_subset
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?

  template<typename T>
  void sortDataWithContainer(T& sorter, Qt::SortOrder order);
private:
};

#endif // __smtk_qt_qtEntityItemModel_h
