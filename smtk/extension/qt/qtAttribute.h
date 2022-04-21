//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttribute - a class that encapsulates the UI of an Attribute
// .SECTION Description

#ifndef smtk_extension_qtAttribute_h
#define smtk_extension_qtAttribute_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"
#include <QObject>
#include <QPointer>
#include <QWidget>

class qtAttributeInternals;
class QWidget;

namespace smtk
{
namespace extension
{

class qtBaseView;
class qtItem;

class SMTKQTEXT_EXPORT qtAttribute : public QObject
{
  Q_OBJECT

public:
  qtAttribute(
    smtk::attribute::AttributePtr,
    const smtk::view::Configuration::Component& comp,
    QWidget* parent,
    qtBaseView* view,
    bool createWidgetWhenEmpty = false);
  ~qtAttribute() override;

  ///\brief Return the SMTK attribute referenced by the instance
  smtk::attribute::AttributePtr attribute() const;
  ///\brief Return the QT widget that visualizes the attribute
  QWidget* widget() { return m_widget; }
  ///\brief Return the QT parent widget
  QWidget* parentWidget();

  ///\brief Add a qtItem to the instance
  virtual void addItem(qtItem*);
  ///\brief Return all of the qtItems stored in the instance
  QList<qtItem*>& items() const;
  virtual void showAdvanceLevelOverlay(bool show);
  bool useSelectionManager() const { return m_useSelectionManager; }

  ///\brief A basic layout for an attribute
  void createBasicLayout(bool includeAssociations);

  ///\brief Returns true if it does not display any of its items
  bool isEmpty() const;

  ///\brief Returns true if the underlying attribute is valid
  bool isValid() const;

  ///\brief Remove all qtItems contained in the qtAttribute.  This allows
  /// createBasicLayout to be called multiple times.
  void removeItems();

Q_SIGNALS:
  ///\brief Signal indicates that the underlying item has been modified
  void modified();
  void itemModified(qtItem*);

protected:
  ///\briefMethod for creating the widget for the qtAttribute.
  ///
  /// If the underlying attribute would have been filtered out by
  /// categories or advanced level a nullptr is returned unless
  /// createWidgetWhenEmpty is true.
  virtual void createWidget(bool createWidgetWhenEmpty = false);

  QPointer<QWidget> m_widget;

protected Q_SLOTS:
  void onItemModified();

private:
  qtAttributeInternals* m_internals;
  bool m_useSelectionManager;
  bool m_isEmpty;
};

} // namespace extension
} // namespace smtk

#endif
