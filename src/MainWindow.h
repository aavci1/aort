#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <OGRE/OgrePrerequisites.h>

#include "ui_MainWindow.h"

class TranslationManager;

class MainWindow : public QMainWindow, public Ui::MainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void changeEvent(QEvent *e);

public slots:
  void mousePressed(QMouseEvent *e);
  void mouseMoved(QMouseEvent *e);
  void wheelMoved(QWheelEvent *e);

private slots:
  void open();
  void translate(QAction *action);
  void render();
  void help();
  void about();
  void windowCreated();

private:
  TranslationManager *mTranslationManager;
  Ogre::SceneNode *objectNode;
  Ogre::Camera *camera;
  Ogre::Viewport *viewport;
  QPoint panStart;
  QPoint mousePosition;
};

#endif // MAINWINDOW_H
