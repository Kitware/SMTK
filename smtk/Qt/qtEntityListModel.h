#ifndef __smtk_qt_qtEntityListModel_h
#define __smtk_qt_qtEntityListModel_h

#include "QAbstractListModel"

#include "smtk/PublicPointerDefs.h"
#include "smtk/util/UUID.h"

class QEntityListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  QEntityListModel(smtk::model::StoragePtr model, QObject* parent = 0);
  virtual ~QEntityListModel();

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const { return 3; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

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

private:
};

#endif // __smtk_qt_qtEntityListModel_h
