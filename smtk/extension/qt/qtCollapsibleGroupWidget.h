//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCollapsibleGroupWidget -Widget that can be "collapsed/closed"
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_qtCollapsibleGroupWidget_h
#define __smtk_extension_qtCollapsibleGroupWidget_h

#include <QWidget>
#include "smtk/extension/qt/Exports.h"

class QFrame;
namespace smtk
{
  namespace extension
  {
    class qtCollapsibleGroupWidgetInternals;

    class SMTKQTEXT_EXPORT qtCollapsibleGroupWidget: public QWidget
    {
      Q_OBJECT

    public:
      qtCollapsibleGroupWidget(QWidget *parent);
      virtual ~qtCollapsibleGroupWidget();

      QLayout *contentsLayout() const;
      void setContentsLayout(QLayout *newLayout);
      QFrame *contents() const;
      void setName(const QString &newName);
      QString name() const;

      public slots:
	void open();
	void collapse();
    protected:
	qtCollapsibleGroupWidgetInternals *m_internals;

    private:
    };
  };
};

#endif /* __smtk_extension_qtCollapsibleGroupWidget_h */
