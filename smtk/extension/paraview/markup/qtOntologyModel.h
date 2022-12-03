//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_qt_qtOntologyModel_h
#define smtk_qt_qtOntologyModel_h

#include "smtk/SharedFromThis.h"
#include "smtk/extension/paraview/markup/smtkPQMarkupExtModule.h"

#include <QAbstractItemModel>

// Forward declarations.
namespace smtk
{
namespace markup
{
namespace ontology
{
class Source;
}
} // namespace markup
} // namespace smtk

/**\brief A model to pass a QCompleter when users should select an ontology ID.
  *
  * This model will use column
  * + 0 (Column::Name) to hold the name of the ontology class;
  * + 1 (Column::URL) to hold the ontology class URL;
  * + 2 (Column::Base) to hold the base class URL (if any); and
  * + 3 (Column::Description) to hold a description of the ontology class (if any).
  */
class SMTKPQMARKUPEXT_EXPORT qtOntologyModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  smtkTypeMacroBase(qtOntologyModel);
  smtkSuperclassMacro(QAbstractItemModel);

  qtOntologyModel(const smtk::markup::ontology::Source& source, QObject* parent = nullptr);
  ~qtOntologyModel() override;

  /// An enum used to index columns in the model.
  enum class Column : int
  {
    Name,
    URL,
    Base,
    Description,
    Count
  };

  /// Return data for the requested row, column, and role.
  QVariant data(const QModelIndex& index, int role) const override;

  /// Return the number of columns in the model (which is always Column::Count).
  int columnCount(const QModelIndex& parent) const override;
  /// Return the number of rows (ontology identifiers) in the model.
  int rowCount(const QModelIndex& parent) const override;
  /// Return a QModelIndex for the given row, column, and parent.
  QModelIndex index(int row, int column, const QModelIndex& parent) const override;
  /// Return the parent of the given \a index.
  QModelIndex parent(const QModelIndex& index) const override;

protected:
  class Internal;
  Internal* m_p{ nullptr };
};

#endif // smtk_qt_qtOntologyModel_h
