/***************************************************************************
**
**  Copyright (C) 2020 KuCAD <www.kucad.ltd>
**  Contact: springtiger@aliyun.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see http://www.gnu.org/licenses/.
**
****************************************************************************/


#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QtWidgets>


#include "litewindow.h"

using namespace QLite;

class QRCodeWidget;
class MainWindowUi;
class MainWindow : public LiteWindow
{
    Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:

public slots:
    void addItem(QWidget* frame, bool isSelected = false);
    void changeRow(int row);

private slots:
    void currentRowChanged(int row);
	void slotStateChanged(const QString &message);
	void slotChangeSkin();

protected:
	void setupSkinActions();
	void addSkinItem(QAction *action, const QString &qss);
	void readSettings();
	void writeSettings();
protected:
	QList<QRCodeWidget*> m_settings;
	QPushButton *m_btn_close;
	QActionGroup *m_pActionGroup;

private:
	QScopedPointer <MainWindowUi> m_ui;
};

#endif // __MAINWINDOW_H__
