#include "TemplateEditorMain.h"

#include <QApplication>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  TemplateEditorMain mainWindow;

  if (argc == 2)
  {
    char* fileName = argv[1];
    mainWindow.load(fileName);
  }

  mainWindow.show();

  return app.exec();
}
