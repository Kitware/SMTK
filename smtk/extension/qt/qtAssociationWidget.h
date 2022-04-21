//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAssociationWidget_h
#define smtk_extension_qtAssociationWidget_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QPointer>
#include <QWidget>

namespace smtk
{
namespace extension
{
class qtBaseView;
///\brief Base class for widgets that deal with attribute associations.
class SMTKQTEXT_EXPORT qtAssociationWidget : public QWidget
{
  Q_OBJECT

public:
  qtAssociationWidget(QWidget* p, qtBaseView* view)
    : QWidget(p)
    , m_view(view)
  {
  }
  ~qtAssociationWidget() override = default;
  ///\brief Return true if there are persistent objects selected.
  ///
  /// This comes in handy when a parent group view changes tabs.  With this
  /// you can prevent tab switching from changing selection state when returning
  /// to the view owning this widget
  virtual bool hasSelectedItem() = 0;
  ///\brief return true if the association information is valid.
  ///
  /// Note that this not only can refer to the requirements related to the target
  /// attribute but also to requirements imposed by the View description such as
  /// requiring that all "appropriate" persistent objects be associated with an attribute
  /// of a specified type.
  virtual bool isValid() const = 0;

public Q_SLOTS:
  ///\brief Display/Modify the association information to a specific attribute
  virtual void showEntityAssociation(smtk::attribute::AttributePtr theAtt) = 0;

  ///\brief Display/Modify the association information to a specific attribute definition
  virtual void showEntityAssociation(smtk::attribute::DefinitionPtr theDef) = 0;

Q_SIGNALS:
  void attAssociationChanged();
  void availableChanged();

protected:
  QPointer<qtBaseView> m_view;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
