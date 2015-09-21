/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright 2004 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**

** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/






void CropToolPopup::radius1_valueChanged( int  num)
{
  cropTool->radius_value_changed(0, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Xval1_valueChanged( int num)
{
  int vals[3];
  vals[0] = num;
  vals[1] = Yval1->value();
  vals[2] = Zval1->value();
  cropTool->xyz_value_changed(0, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Yval1_valueChanged( int num)
{
  int vals[3];
  vals[0] = Xval1->value();
  vals[1] = num;
  vals[2] = Zval1->value();
  cropTool->xyz_value_changed(0, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Zval1_valueChanged( int num)
{
  int vals[3];
  vals[0] = Xval1->value();
  vals[1] = Yval1->value();
  vals[2] = num;
  cropTool->xyz_value_changed(0, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot1a_valueChanged( int num )
{
  int vals[2];
  vals[0] = num;
  vals[1] = Rot1b->value();
  cropTool->rotate_value_changed(0, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot1b_valueChanged( int num )
{
  int vals[2];
  vals[0] = Rot1a->value();
  vals[1] = num;
  cropTool->rotate_value_changed(0, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::radius2_valueChanged( int num )
{
  cropTool->radius_value_changed(1, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Xval2_valueChanged( int num )
{
  int vals[3];
  vals[0] = num;
  vals[1] = Yval2->value();
  vals[2] = Zval2->value();
  cropTool->xyz_value_changed(1, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Yval2_valueChanged( int num )
{
  int vals[3];
  vals[0] = Xval2->value();
  vals[1] = num;
  vals[2] = Zval2->value();
  cropTool->xyz_value_changed(1, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Zval2_valueChanged( int num )
{
  int vals[3];
  vals[0] = Xval2->value();
  vals[1] = Yval2->value();
  vals[2] = num;
  cropTool->xyz_value_changed(1, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot2a_valueChanged( int num )
{
  int vals[2];
  vals[0] = num;
  vals[1] = Rot2b->value();
  cropTool->rotate_value_changed(1, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot2b_valueChanged( int num )
{
  int vals[2];
  vals[0] = Rot1a->value();
  vals[1] = num;
  cropTool->rotate_value_changed(1, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::radius3_valueChanged( int num )
{
  cropTool->radius_value_changed(2, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Xval3_valueChanged( int num )
{
  int vals[3];
  vals[0] = num;
  vals[1] = Yval3->value();
  vals[2] = Zval3->value();
  cropTool->xyz_value_changed(2, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Yval3_valueChanged( int num )
{
  int vals[3];
  vals[0] = Xval3->value();
  vals[1] = num;
  vals[2] = Zval3->value();
  cropTool->xyz_value_changed(2, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Zval3_valueChanged( int num )
{
  int vals[3];
  vals[0] = Xval3->value();
  vals[1] = Yval3->value();
  vals[2] = num;
  cropTool->xyz_value_changed(2, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot3a_valueChanged( int num )
{
  int vals[2];
  vals[0] = num;
  vals[1] = Rot1b->value();
  cropTool->rotate_value_changed(2, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rot3b_valueChanged( int num )
{
  int vals[2];
  vals[0] = Rot1a->value();
  vals[1] = num;
  cropTool->rotate_value_changed(2, vals);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rev1_toggled( bool togg)
{
  cropTool->toggle_changed(0, togg);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rev2_toggled( bool togg)
{
  cropTool->toggle_changed(1, togg);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::Rev3_toggled( bool togg)
{
  cropTool->toggle_changed(2, togg);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::CropToolType1_highlighted( int num )
{
  cropTool->type_activated(0, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::CropToolType2_highlighted( int num )
{
  cropTool->type_activated(1, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::init()
{
  cropTool = new CropTool();
  
    //cropTool->type_activated(0, CropTool::PLANEX);
}


void CropToolPopup::destroy()
{
  if (NULL != cropTool) {
    delete cropTool;
    cropTool = NULL;
  }
}


void CropToolPopup::CropToolType3_highlighted( int num )
{
  cropTool->type_activated(2, num);
  vtkWidget->GetRenderWindow()->Render();
}


void CropToolPopup::vtk_widget( QVTKWidget *vtkw )
{
  vtkWidget = vtkw;
}
