#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <dndemask.hpp>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QMainWindow window;
  window.resize(200, 250);
  window.show();

  QPushButton show("Show picture", &window);
  show.setGeometry(50, 50, 100, 30);
  show.show();
  QObject::connect(&show, &QPushButton::clicked, []() { dndemask::viewer_main(); });
  QPushButton edit("Editor", &window);
  edit.setGeometry(50, 100, 100, 30);
  edit.show();
  QObject::connect(&edit, &QPushButton::clicked, []() { dndemask::editor_main(); });

  return app.exec();
}
