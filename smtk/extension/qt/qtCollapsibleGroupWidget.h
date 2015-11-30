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

#ifndef __qtCollapsibleGroupWidget_h
#define __qtCollapsibleGroupWidget_h

#include <QWidget>

class QFrame;
namespace smtk
{
  class qtCollapsibleGroupWidgetInternals;
  
  class qtCollapsibleGroupWidget: public QWidget
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


#endif /* __qtCollapsibleGroupWidget_h */
