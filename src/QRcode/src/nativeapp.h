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

#ifndef __NATIVEAPP_H__
#define __NATIVEAPP_H__
#include <QApplication>

class NativeApp : public QApplication
{
public:
	NativeApp(int &argc, char **argv);
	~NativeApp();

	static NativeApp &instance();

public:
	void setMainWindow(QWidget *win);
	QWidget* mainWindow() const;

	virtual bool notify(QObject *receiver, QEvent *event);


	virtual void displayError(const QString &msg, const QString &title = QString(), QWidget *parent = NULL) const;
	virtual void displayWarning(const QString &msg, const QString &title = QString(), QWidget *parent = NULL) const;
	virtual void displayInfo(const QString &msg, const QString &title = QString(), QWidget *parent = NULL) const;


	virtual QString getOpenFileName(QWidget *parent, const QStringList &filters, const QString &prompt = QString(), const QString &defaultPath = QString());
	virtual QString getSaveFileName(QWidget * parent, const QString & caption, const QString & directory, const QString & filter, const QStringList & saveAbleExtensions);

	QString buildNumber()const;

private:

	void set_smaller_text_osx(QWidget *w);


private:
	QWidget* m_mainWindow;
};


inline NativeApp & app() {
	return NativeApp::instance();
}

#endif // __NATIVEAPP_H__
