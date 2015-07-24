/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void SheetDiagramPopup::init()
{
  if (sheetDiagram == NULL) sheetDiagram = new QVTKWidget(0, "sheetDiagram");
}


QVTKWidget* SheetDiagramPopup::sheet_diagram()
{
  return sheetDiagram;
}
