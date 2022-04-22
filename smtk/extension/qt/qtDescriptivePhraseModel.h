//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtDescriptivePhraseModel_h
#define smtk_extension_qtDescriptivePhraseModel_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h" // For EXPORT macro.
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/PhraseModelObserver.h"

#include <QAbstractItemModel>
#include <QIcon>

#include <map>

namespace smtk
{
namespace extension
{

/**\brief Adapt an smtk::view::PhraseModel instance into a hierarchical Qt model.
  *
  * By calling setPhraseModel(), you identify the toplevel
  * description you wish to present; it may cover any set
  * of model, mesh, and attribute resources.
  *
  * The smtk::view::PhraseModel uses an smtk::view::SubphraseGenerator
  * instance to create a hierarchy of smtk::view::DescriptivePhrase
  * instances which portray the components in a set of resources.
  */
class SMTKQTEXT_EXPORT qtDescriptivePhraseModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  qtDescriptivePhraseModel(QObject* parent = nullptr);
  ~qtDescriptivePhraseModel() override;
  // Set and get default entity color
  static void setDefaultPhraseColor(const std::string& entityType, const QColor& color)
  {
    s_defaultColors[entityType] = color;
  }
  static QColor defaultPhraseColor(const std::string& entityType);

  /// Set and get the icons to be used when visibility is to be drawn
  std::string visibleIconURL() const { return m_visibleIconURL; }
  std::string invisibleIconURL() const { return m_invisibleIconURL; }
  void setVisibleIconURL(const std::string& url) { m_visibleIconURL = url; }
  void setInvisibleIconURL(const std::string& url) { m_invisibleIconURL = url; }

  /// Enumeration of model-specific data roles.
  enum DataRoles
  {
    TitleTextRole = Qt::UserRole + 100, //!< Phrase title (usu. current component name)
    EditableTitleTextRole =
      Qt::UserRole + 101,                  //!< Phrase title (usu. user-editable component name)
    SubtitleTextRole = Qt::UserRole + 102, //!< Phrase subtitle (usu. type of phrase)
    // PhraseIconRole_LightBG = Qt::UserRole + 103, //!< Phrase type icon on light background
    // PhraseIconRole_DarkBG = Qt::UserRole + 104,  //!< Phrase type icon on dark background
    // PhraseColorRole = Qt::UserRole + 105, //!< Phrase-specific color (e.g., component color)
    // PhraseVisibilityRole = Qt::UserRole + 106, //!< Visibility of phrase's subject
    PhraseCleanRole = Qt::UserRole + 107,      //!< Is resource clean (0), dirty (1), or N/A (-1)?
    PhraseLockRole = Qt::UserRole + 108,       //!< Is resource free (0) or locked (1)?
    ModelActiveRole = Qt::UserRole + 109,      //!< Is resource the active resource?
    TitleTextMutableRole = Qt::UserRole + 110, //!< Is the title editable?
    // ColorMutableRole = Qt::UserRole + 111,       //!< Is the subject's color editable?
    PhrasePtrRole = Qt::UserRole + 112, //!< Grab the whole descriptive phrase!
    BadgesRole = Qt::UserRole + 113     //!< Grab the set of badges for this phrase.
  };

  void setPhraseModel(smtk::view::PhraseModelPtr model);
  smtk::view::PhraseModelPtr phraseModel() const { return m_model; }

  QModelIndex index(int row, int column, const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  bool hasChildren(const QModelIndex& parent) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& inParent = QModelIndex()) const override
  {
    (void)inParent;
    return 1;
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  // bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  void setDeleteOnRemoval(bool del) { m_deleteOnRemoval = del; }

  view::DescriptivePhrasePtr getItem(const QModelIndex& idx) const;
  QModelIndex indexFromPath(const std::vector<int>& path) const;

  template<typename T, typename C>
  bool foreach_phrase(
    T& visitor,
    C& collector,
    const QModelIndex& top = QModelIndex(),
    bool onlyBuilt = true);
  template<typename T, typename C>
  bool foreach_phrase(
    T& visitor,
    C& collector,
    const QModelIndex& top = QModelIndex(),
    bool onlyBuilt = true) const;

  void rebuildSubphrases(const QModelIndex& qidx);

  Qt::DropActions supportedDropActions() const override;

  void setColumnName(const std::string& name) { m_columnName = name; }
  const std::string& columnName() const { return m_columnName; }

signals:
  void phraseTitleChanged(const QModelIndex&);

protected:
  void updateObserver(
    smtk::view::DescriptivePhrasePtr phrase,
    smtk::view::PhraseModelEvent event,
    const std::vector<int>& src,
    const std::vector<int>& dst,
    const std::vector<int>& range);

  smtk::view::PhraseModelPtr m_model;
  smtk::view::PhraseModelObservers::Key m_modelObserver;
  bool m_deleteOnRemoval; // remove UUIDs from mesh when they are removed from the list?
  std::string m_visibleIconURL;
  std::string m_invisibleIconURL;
  static std::map<std::string, QColor> s_defaultColors;
  std::string m_columnName;
  class Internal;
  Internal* P;
};

/**\brief Iterate over all expanded entries in the tree.
  *
  * Note that if you call this method, you must include "smtk/extension/qt/qtTypeDeclarations.h".
  */
template<typename T, typename C>
bool qtDescriptivePhraseModel::foreach_phrase(
  T& visitor,
  C& collector,
  const QModelIndex& top,
  bool onlyBuilt)
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase =
      top.data(PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some phrases
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
    {
      for (int row = 0; row < this->rowCount(top); ++row)
      {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
      }
    }
  }
  return false;
}

/// A const version of foreach_phrase. See the non-const version for documentation.
template<typename T, typename C>
bool qtDescriptivePhraseModel::foreach_phrase(
  T& visitor,
  C& collector,
  const QModelIndex& top,
  bool onlyBuilt) const
{
  // visit parent, then children if we aren't told to terminate:
  if (!visitor(this, top, collector))
  {
    view::DescriptivePhrasePtr phrase =
      top.data(PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
    // Do not descend if top's corresponding phrase would have to invoke
    // the subphrase generator to obtain the list of children... some models
    // are cyclic graphs. In these cases, only descend if "onlyBuilt" is false.
    if (phrase && (!onlyBuilt || phrase->areSubphrasesBuilt()))
    {
      for (int row = 0; row < this->rowCount(top); ++row)
      {
        if (this->foreach_phrase(visitor, collector, this->index(row, 0, top), onlyBuilt))
          return true; // early termination;
      }
    }
  }
  return false;
}

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDescriptivePhraseModel_h
