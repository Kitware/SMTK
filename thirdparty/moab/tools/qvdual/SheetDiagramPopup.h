/****************************************************************************
** Form interface generated from reading ui file 'SheetDiagramPopup.ui'
**
** Created: Wed Aug 8 03:15:32 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SHEETDIAGRAMPOPUP_H
#define SHEETDIAGRAMPOPUP_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QVTKWidget;

class SheetDiagramPopup : public QDialog
{
    Q_OBJECT

public:
    SheetDiagramPopup( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SheetDiagramPopup();

    QVTKWidget* sheetDiagram;

    virtual void init();
    virtual QVTKWidget * sheet_diagram();

protected:

protected slots:
    virtual void languageChange();

};

#endif // SHEETDIAGRAMPOPUP_H
