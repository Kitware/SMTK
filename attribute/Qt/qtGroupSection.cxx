/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "qtGroupSection.h"

#include "qtUIManager.h"

#include "attribute/Section.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSize>
#include <QTabWidget>
#include <QFile>
#include <QApplication>

using namespace slctk::attribute;

//----------------------------------------------------------------------------
class qtGroupSectionInternals
{
public:
  QList<slctk::attribute::qtSection*> ChildSections;

};

//----------------------------------------------------------------------------
qtGroupSection::qtGroupSection(
  slctk::SectionPtr dataObj, QWidget* p) : qtSection(dataObj, p)
{
  this->Internals = new qtGroupSectionInternals;
  this->createWidget( );

}

//----------------------------------------------------------------------------
qtGroupSection::~qtGroupSection()
{
  this->clearChildSections();
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtGroupSection::createWidget( )
{
  if(!this->getObject())
    {
    return;
    }
  this->clearChildSections();
  QTabWidget *tab = new QTabWidget(this->parentWidget());
  tab->setUsesScrollButtons( true );
  this->Widget = tab;

  this->parentWidget()->layout()->addWidget(this->Widget);    
}

//----------------------------------------------------------------------------
void qtGroupSection::addChildSection(qtSection* child)
{
  if(!this->Internals->ChildSections.contains(child))
    {
    this->Internals->ChildSections.append(child);
    QTabWidget *tab = dynamic_cast<QTabWidget*>(this->Widget);
    
    }
}
//----------------------------------------------------------------------------
QList<qtSection*>& qtGroupSection::childSections() const
{
  return this->Internals->ChildSections;
} 
//----------------------------------------------------------------------------
void qtGroupSection::clearChildSections()
{
  for(int i=0; i < this->Internals->ChildSections.count(); i++)
    {
    delete this->Internals->ChildSections.value(i);
    }
  this->Internals->ChildSections.clear();
}

//----------------------------------------------------------------------------
void qtGroupSection::showAdvanced(int checked)
{

}

//----------------------------------------------------------------------------
void qtGroupSection::isRootTabGroup( )
{
  //if we are the root tab group we want to be display differently than the other tab groups
  QTabWidget *tab = dynamic_cast<QTabWidget*>(this->Widget);
  if ( tab )
    {
    tab->setTabPosition(QTabWidget::East);

    //default icon size is style dependent. We will override this with a default
    //that icons can't be smaller than 32x32
    QSize ISize = tab->iconSize();
    if ( ISize.height() < 24 && ISize.width() < 24 )
      {
      tab->setIconSize( QSize(24,24) );
      }
    }
}

//----------------------------------------------------------------------------
void qtGroupSection::addTabEntry(qtSection* child)
{
  QTabWidget* tabWidget = static_cast<QTabWidget*>(this->Widget);
  if(!tabWidget || !child || !child->getObject())
    {
    return;
    }
  QWidget* tabPage = new QWidget();
  tabPage->setSizePolicy(QSizePolicy::Ignored,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(tabWidget);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  QString secTitle = child->getObject()->title().c_str();
  QString name = "tab" + QString(secTitle);
  s->setObjectName(name);
  s->setWidget(tabPage);

  QVBoxLayout* vLayout = new QVBoxLayout(tabPage);
  vLayout->setMargin(0);

  int index = tabWidget->addTab(s, secTitle);
  tabWidget->setTabToolTip( index, secTitle);

  //using the ui label name find if we have an icon resource
  QString resourceName = QApplication::applicationDirPath().append("/../Resources/Icons/");
  //QString resourceName = ":/SimBuilder/Icons/";
  resourceName.append(child->getObject()->iconName().c_str());
  resourceName.append(".png");
  
  // If user specified icons are not found, use default ones
  if ( !QFile::exists( resourceName ) )
    {
    resourceName = QString(":/SimBuilder/Icons/").
      append(secTitle).append(".png");
    }
  
  if ( QFile::exists( resourceName ) )
    {
    QPixmap image(resourceName);
    QIcon icon;
    QMatrix transform;
    if ( tabWidget->tabPosition() == QTabWidget::East )
      {
      transform.rotate(-90);
      }
    else if( tabWidget->tabPosition() == QTabWidget::West )
      {
      transform.rotate(90);
      }
    icon = image.transformed( transform );
    tabWidget->setTabIcon( index, icon );
    tabWidget->setTabText( index, "");
    }

  vLayout->setAlignment(Qt::AlignTop);
}
