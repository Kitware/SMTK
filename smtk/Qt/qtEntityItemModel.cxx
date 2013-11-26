#include "smtk/Qt/qtEntityItemModel.h"

#include "smtk/model/Entity.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Storage.h"
#include "smtk/model/StringData.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <iomanip>
#include <sstream>

// -----------------------------------------------------------------------------
// The following is used to ensure that the QRC file
// containing the entity-type icons is registered.
// This is required when building SMTK with static libraries.
// Note that the inlined functions may *NOT* be placed inside a namespace
// according to the Qt docs here:
//   http://qt-project.org/doc/qt-5.0/qtcore/qdir.html#Q_INIT_RESOURCE
static int resourceCounter = 0;

inline void initIconResource()
{
  if (resourceCounter <= 0)
    {
    Q_INIT_RESOURCE(qtEntityItemModelIcons);
    }
  ++resourceCounter;
}

inline void cleanupIconResource()
{
  if (--resourceCounter == 0)
    {
    Q_CLEANUP_RESOURCE(qtEntityItemModelIcons);
    }
}

// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {

// -----------------------------------------------------------------------------
/// A functor for sorting entity UUIDs by their entity-type bit flags.
struct SortByEntityFlags
{
  SortByEntityFlags(smtk::model::StoragePtr storage)
    : m_storage(storage)
    {
    }
  bool operator () (const smtk::util::UUID& a, const smtk::util::UUID& b) const
    {
    smtk::model::Entity* ea = this->m_storage->findEntity(a);
    smtk::model::Entity* eb = this->m_storage->findEntity(b);
    if (!ea)
      {
      return true;
      }
    if (!eb)
      {
      return false;
      }
    return ea->entityFlags() < eb->entityFlags();
    }
  smtk::model::StoragePtr m_storage;
};

// -----------------------------------------------------------------------------
/**\brief A functor for sorting entity UUIDs by a given, named property.
  *
  * The property may be a string, double, or integer according to the
  * template parameters.
  *
  * Properties which have shorter vectors are always "less than" those
  * with longer vectors. This may change in the future. You have been
  * warned.
  */
template<
  class T,
  typename U,
  U&(T::*GetProperty)(const smtk::util::UUID&, const std::string&) = &T::stringProperty,
  bool (T::*HasProperty)(const smtk::util::UUID&, const std::string&) const = &T::hasStringProperty
  >
struct SortByEntityProperty
{
  SortByEntityProperty(smtk::model::StoragePtr storage, std::string propName)
    : m_storage(storage), m_propName(propName)
    {
    }
  bool operator () (const smtk::util::UUID& a, const smtk::util::UUID& b) const
    {
    if (a == b)
      {
      return false;
      }
    bool aHas = (this->m_storage.get()->*HasProperty)(a, this->m_propName);
    bool bHas = (this->m_storage.get()->*HasProperty)(b, this->m_propName);
    if (!aHas && !bHas)
      {
      return a < b;
      }
    if (!aHas)
      {
      return false;
      }
    if (!bHas)
      {
      return true;
      }
    U& sa((this->m_storage.get()->*GetProperty)(a, this->m_propName));
    U& sb((this->m_storage.get()->*GetProperty)(b, this->m_propName));
    typename U::size_type nCommon =
      sa.size() > sb.size() ?
      sb.size() : sa.size();
    for (typename U::size_type n = 0; n < nCommon; ++n)
      {
      if (sa[n] < sb[n])
        return true;
      if (sb[n] < sa[n])
        return false;
      }
    if (sb.size() > sa.size())
      return true;
    return a < b;
    }
  smtk::model::StoragePtr m_storage;
  std::string m_propName;
};

// -----------------------------------------------------------------------------
QEntityItemModel::QEntityItemModel(smtk::model::StoragePtr model, QObject* parent)
  : m_storage(model), QAbstractItemModel(parent)
{
  this->m_deleteOnRemoval = true;
  initIconResource();
}

QEntityItemModel::~QEntityItemModel()
{
  cleanupIconResource();
}

QModelIndex QEntityItemModel::index(int row, int column, const QModelIndex& parent) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
}

QModelIndex QEntityItemModel::parent(const QModelIndex& child) const
{
  return QModelIndex();
}

bool QEntityItemModel::hasChildren(const QModelIndex& parent) const
{
      return parent.isValid() ? false : (rowCount() > 0);
}

int QEntityItemModel::rowCount(const QModelIndex& parent) const
{
  return this->m_subset.size();
}

QVariant QEntityItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    {
    return QVariant();
    }
  if (orientation == Qt::Horizontal)
    {
    switch (section)
      {
    case 0:
      return "Type";
    case 1:
      return "Dimension";
    case 2:
      return "Name";
      }
    }
  // ... could add "else" here to present per-row header.
  return QVariant();
}

QVariant QEntityItemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid())
    {
    int row = index.row();
    int col = index.column();
    smtk::util::UUID uid = this->m_subset[row];
    if (role == TitleTextRole)
      {
      if (this->m_storage->hasStringProperty(uid, "name"))
        {
        smtk::model::StringList& names(this->m_storage->stringProperty(uid, "name"));
        return QVariant(names.empty() ? "" : names[0].c_str());
        }
      return QVariant("Unnamed");
      }
    else if (role == SubtitleTextRole)
      {
      smtk::model::Entity* link = this->m_storage->findEntity(uid);
      if (link)
        {
        return QVariant(smtk::model::Entity::flagSummary(link->entityFlags()).c_str());
        }
      return QVariant("missing UUID");
      }
    else if (role == EntityIconRole)
      {
      smtk::model::Entity* link = this->m_storage->findEntity(uid);
      if (link)
        {
        return QVariant(
          this->lookupIconForEntityFlags(
            link->entityFlags()));
        }
      }
    else if (role == Qt::DisplayRole)
      {
      switch (col)
        {
      case 0:
          {
          smtk::model::Entity* link = this->m_storage->findEntity(uid);
          if (link)
            {
            return QVariant(""); // FIXME: Why is this being rendered when I set the itemdelegate on the view?
            //return QVariant(smtk::model::Entity::flagSummary(link->entityFlags()).c_str());
            }
          }
        break;
      case 1:
          {
          smtk::model::Entity* link = this->m_storage->findEntity(uid);
          if (link)
            {
            return QVariant(link->dimension());
            }
          }
        break;
      case 2:
          {
          if (this->m_storage->hasStringProperty(uid, "name"))
            {
            smtk::model::StringList& names(this->m_storage->stringProperty(uid, "name"));
            return QVariant(names.empty() ? "" : names[0].c_str());
            }
          return QVariant("");
          }
        break;
        }
      }
    }
  return QVariant();
}

bool QEntityItemModel::insertRows(int position, int rows, const QModelIndex& parent)
{
  beginInsertRows(QModelIndex(), position, position + rows - 1);

  int maxPos = position + rows;
  for (int row = position; row < maxPos; ++row)
    {
    smtk::util::UUID uid = this->m_storage->addEntityOfTypeAndDimension(smtk::model::INVALID, -1);
    this->m_subset.insert(this->m_subset.begin() + row, uid);
    this->m_reverse[uid] = row;
    }

  endInsertRows();
  return true;
}

bool QEntityItemModel::removeRows(int position, int rows, const QModelIndex& parent)
{
  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  smtk::util::UUIDArray uids(this->m_subset.begin() + position, this->m_subset.begin() + position + rows);
  this->m_subset.erase(this->m_subset.begin() + position, this->m_subset.begin() + position + rows);
  int maxPos = position + rows;
  for (smtk::util::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    this->m_reverse.erase(this->m_reverse.find(*it));
    if (this->m_deleteOnRemoval)
      {
      this->m_storage->removeEntity(*it);
      // FIXME: Should we keep a set of all it->second.relations()
      //        and emit "didChange" events for all the relations
      //        that were affected by removal of this entity?
      }
    }

  endRemoveRows();
  return true;
}

bool QEntityItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  bool didChange = false;
  if(index.isValid() && role == Qt::EditRole)
    {
    smtk::model::Entity* entity;
    int row = index.row();
    int col = index.column();
    switch (col)
      {
    case 0:
      entity = this->m_storage->findEntity(this->m_subset[row]);
      if (entity)
        {
        didChange = entity->setEntityFlags(value.value<int>());
        }
      break;
    case 1:
      // FIXME: No way to change dimension yet.
      break;
    case 2:
        {
        std::string sval = value.value<QString>().toStdString();
        if (sval.size())
          {
          this->m_storage->setStringProperty(
            this->m_subset[row], "name", sval);
          didChange = true;
          }
        else
          {
          didChange = this->m_storage->removeStringProperty(
            this->m_subset[row], "name");
          }
        }
      break;
      }
    if (didChange)
      {
      emit(dataChanged(index, index));
      }
    }
  return didChange;
}

void QEntityItemModel::sort(int column, Qt::SortOrder order)
{
  switch (column)
    {
  case -1:
      { // Sort by UUID.
      std::multiset<smtk::util::UUID> sorter;
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 0:
  case 1:
      {
      smtk::model::SortByEntityFlags comparator(this->m_storage);
      std::multiset<smtk::util::UUID,smtk::model::SortByEntityFlags>
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  case 2:
      {
      smtk::model::SortByEntityProperty<
        smtk::model::BRepModel,
        smtk::model::StringList,
        &smtk::model::BRepModel::stringProperty,
        &smtk::model::BRepModel::hasStringProperty> comparator(
          this->m_storage, "name");
      std::multiset<
        smtk::util::UUID,
        smtk::model::SortByEntityProperty<
          smtk::model::BRepModel,
          smtk::model::StringList,
          &smtk::model::BRepModel::stringProperty,
          &smtk::model::BRepModel::hasStringProperty> >
        sorter(comparator);
      this->sortDataWithContainer(sorter, order);
      }
    break;
  default:
    std::cerr << "Bad column " << column << " for sorting\n";
    break;
    }
  emit dataChanged(
    this->index(0, 0, QModelIndex()),
    this->index(this->m_subset.size(), this->columnCount(), QModelIndex()));
}

Qt::ItemFlags QEntityItemModel::flags(const QModelIndex& index) const
{
  if(!index.isValid())
    return Qt::ItemIsEnabled;

  // TODO: Check to make sure column is not "information-only".
  //       We don't want to allow people to randomly edit an enum string.
  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

QIcon QEntityItemModel::lookupIconForEntityFlags(unsigned long flags)
{
  std::ostringstream resourceName;
  resourceName << ":/icons/entityTypes/";
  bool dimBits = true;
  switch (flags & ENTITY_MASK)
    {
  case CELL_ENTITY:
    resourceName << "cell";
    break;
  case USE_ENTITY:
    resourceName << "use";
    break;
  case SHELL_ENTITY:
    resourceName << "shell";
    break;
  case GROUP_ENTITY:
    resourceName << "group";
    break;
  case MODEL_ENTITY:
    resourceName << "model";
    break;
  case INSTANCE_ENTITY:
    resourceName << "instance";
    break;
  default:
    resourceName << "invalid";
    dimBits = false;
    break;
    }
  if (dimBits)
    {
    resourceName
      << "_"
      << std::setbase(16) << std::fixed << std::setw(2) << std::setfill('0')
      << (flags & ANY_DIMENSION)
      ;
    }
  resourceName << ".svg";
  QFile rsrc(resourceName.str().c_str());
  if (!rsrc.exists())
    { // FIXME: Replace with the path of a "generic entity" or "invalid" icon.
    return QIcon(":/icons/entityTypes/cell_08.svg");
    }
  return QIcon(resourceName.str().c_str());
}

/**\brief Sort the UUIDs being displayed using the given ordered container.
  *
  * The ordered container's comparator is used to insertion-sort the UUIDs
  * displayed. Then, the \a order is used to either forward- or reverse-iterator
  * over the container to obtain a new ordering for the UUIDs.
  */
template<typename T>
void QEntityItemModel::sortDataWithContainer(T& sorter, Qt::SortOrder order)
{
  smtk::util::UUIDArray::iterator ai;
  // Insertion into the set sorts the UUIDs.
  for (ai = this->m_subset.begin(); ai != this->m_subset.end(); ++ai)
    {
    sorter.insert(*ai);
    }
  // Now we reset m_subset and m_reverse and recreate based on the sorter's order.
  this->m_subset.clear();
  this->m_reverse.clear();
  int i;
  if (order == Qt::AscendingOrder)
    {
    typename T::iterator si;
    for (i = 0, si = sorter.begin(); si != sorter.end(); ++si, ++i)
      {
      this->m_subset.push_back(*si);
      this->m_reverse[*si] = i;
      /*
      std::cout << i << "  " << *si << "  " <<
        (this->m_storage->hasStringProperty(*si, "name") ?
         this->m_storage->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         */
      }
    }
  else
    {
    typename T::reverse_iterator si;
    for (i = 0, si = sorter.rbegin(); si != sorter.rend(); ++si, ++i)
      {
      this->m_subset.push_back(*si);
      this->m_reverse[*si] = i;
      /*
      std::cout << i << "  " << *si << "  " <<
        (this->m_storage->hasStringProperty(*si, "name") ?
         this->m_storage->stringProperty(*si, "name")[0].c_str() : "--") << "\n";
         */
      }
    }
}

  } // namespace model
} // namespace smtk
