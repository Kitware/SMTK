#ifndef __smtk_qt_qtEntityItemModel_h
#define __smtk_qt_qtEntityItemModel_h

#include "QAbstractItemModel"
#include "QIcon"

#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/QtSMTKExports.h" // For EXPORT macro.
#include "smtk/model/DescriptivePhrase.h"

namespace smtk {
  namespace model {

/**\brief Adapt an smtk::model::Storage instance into a hierarchical Qt model.
  *
  * This is done by generating instances of smtk::model::DescriptivePhrase
  * subclasses to portray the entities in storage both in terms of their
  * inherent attributes and their relations to other entities.
  *
  * By calling setPhrases() on the model, you identify the toplevel
  * description you wish to present; it may cover any subset of
  * an underlying storage model and may even describe entities from different
  * storage models.
  *
  * You may also call setPhraseFilter() on the model with a filter.
  * The filter is used to alter the available subphrases of each
  * descriptive phrase for presentation. For instance, you may write a
  * filter that omits descriptions of attributes on model items.
  */
class QTSMTK_EXPORT QEntityItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QEntityItemModel(QObject* parent = 0);
  virtual ~QEntityItemModel();

  /// Enumeration of model-specific data roles.
  enum DataRoles {
    TitleTextRole       = Qt::UserRole + 100, //!< Entity name (user-editable)
    SubtitleTextRole    = Qt::UserRole + 101, //!< Entity type description
    EntityIconRole      = Qt::UserRole + 102  //!< Entity type icon
  };

  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  //virtual bool hasChildren(const QModelIndex& parent) const;

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& inParent = QModelIndex()) const { (void)inParent; return 1; }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  //bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  //bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex());
  //bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  //virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void setRoot(DescriptivePhrasePtr root)
    {
    this->m_root = root;
    }

  void setDeleteOnRemoval(bool del)
    {
    this->m_deleteOnRemoval = del;
    }

  static QIcon lookupIconForEntityFlags(unsigned long flags);

protected:
  smtk::model::DescriptivePhrasePtr m_root;
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?

  DescriptivePhrase* getItem(const QModelIndex& idx) const;

  //template<typename T>
  //void sortDataWithContainer(T& sorter, Qt::SortOrder order);
private:
};

  } // namespace model
} // namespace smtk

#endif // __smtk_qt_qtEntityItemModel_h
