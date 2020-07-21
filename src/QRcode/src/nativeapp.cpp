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

#include "stdafx.h"
#include "nativeapp.h"
#include <QtCore>

namespace {
	QString _usage_histories_file_name = "log";
	const char _CURRENT_DIR[] = "currentdir";
	const char _NOTICE_ERROR_AGGRESSIVE[] = "notice/error/aggressive";
	const char _NOTICE_WARNING_AGGRESSIVE[] = "notice/warning/aggressive";

	int getSelectedFilter(const QStringList &filters, const QString &fileName) {
		if (!fileName.isEmpty()) {
			QString ext = QFileInfo(fileName).suffix();
			if (!ext.isEmpty()) {
				QString search = "*." + ext;
				for (int i = 0; i < filters.size(); ++i)
					if (filters[i].contains(search, Qt::CaseInsensitive)) {
						return i;
					}
			}
		}
		return -1;
	}

	QDir currentDocPath() {
		QString path = QSettings().value(_CURRENT_DIR).toString();
		if (!path.isEmpty()) {
			QDir result(path);
			if (result.exists() && result.isReadable()) {
				return result;
			}
		}
		return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	}

	void setCurrentDocPath(const QDir &path) {
		QSettings pref;
		pref.setValue(_CURRENT_DIR, path.absolutePath());
	}


	QDate compilationDate(void) {
		const char *compilation_date = __DATE__;
		const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL };
		int i;
		QDate rc;
		for (i = 0; months[i] != NULL; i++) {
			if (memcmp(compilation_date, months[i], 3) == 0) {
				break;
			}
		}

		if (months[i] == NULL) {
			rc = QDate::currentDate();
		}
		else {
			char year[5], day[3];
			memcpy(year, compilation_date + 7, 4);
			year[4] = 0x00;

			memcpy(day, compilation_date + 4, 2);
			day[2] = 0x00;

			rc = QDate(atoi(year), i + 1, atoi(day));
		}
		return rc;
	}

}

// It's safe to call this function on any platform.
// It will only have an effect on the Mac.
void NativeApp::set_smaller_text_osx(QWidget *w)
{
	return; //not using the small fonts right now...
	if (w == 0)return;

	// By default, none of these size attributes are set.
	// If any has been set explicitly, we'll leave the widget alone.
	if (!w->testAttribute(Qt::WA_MacMiniSize) &&
		!w->testAttribute(Qt::WA_MacSmallSize) &&
		!w->testAttribute(Qt::WA_MacNormalSize) &&
		!w->testAttribute(Qt::WA_MacVariableSize))
	{
		// Is the widget is one of a number of types whose default
		// text size is too large?
		if (w->inherits("QLabel") ||
			w->inherits("QLineEdit") ||
			w->inherits("QTextEdit") ||
			w->inherits("QComboBox") ||
			w->inherits("QCheckBox") ||
			w->inherits("QRadioButton") ||
			w->inherits("QAbstractItemView"))
			// Others could be added here...
		{
			// make the text the 'normal' size
			w->setAttribute(Qt::WA_MacMiniSize);
		}
		else if (w->inherits("QPushButton"))
			// Others could be added here...
		{
			// make the text the 'normal' size
			w->setAttribute(Qt::WA_MacMiniSize);
		}
	}
}
NativeApp::NativeApp(int &argc, char **argv)
	: QApplication(argc, argv)
{
#ifdef Q_OS_WIN
	QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
}

NativeApp::~NativeApp()
{
}

void NativeApp::setMainWindow(QWidget *win) {
	m_mainWindow = win;
}

QWidget* NativeApp::mainWindow() const {
	return m_mainWindow;
}

bool NativeApp::notify(QObject *receiver, QEvent *event)
{
	if (event->type() == QEvent::Polish &&
		receiver &&
		receiver->isWidgetType())
	{
		set_smaller_text_osx(reinterpret_cast<QWidget *>(receiver));
	}

	return QApplication::notify(receiver, event);
}

void
NativeApp::displayError(const QString &msg, const QString &title, QWidget *parent) const
{
	bool silent = !QSettings().value(_NOTICE_ERROR_AGGRESSIVE, true).toBool();
	if (this->thread() == QThread::currentThread() && !silent) {
		QMessageBox msgBox(
			QMessageBox::Critical
			, title.isEmpty() ? tr("Error") : title
			, msg
			, QMessageBox::Close
			, parent != NULL ? parent : m_mainWindow
		);
		QCheckBox *check = new QCheckBox(tr("Don't Remind Me Again"), &msgBox);
		msgBox.setCheckBox(check);

		msgBox.exec();
		QSettings().setValue(_NOTICE_ERROR_AGGRESSIVE, !check->isChecked());
	}
	else {
		qDebug() << msg;
	}
}

void
NativeApp::displayWarning(const QString &msg, const QString &title, QWidget *parent) const
{
	bool silent = !QSettings().value(_NOTICE_WARNING_AGGRESSIVE, true).toBool();
	if (this->thread() == QThread::currentThread() && !silent) {
		QMessageBox msgBox(
			QMessageBox::Warning
			, title.isEmpty() ? tr("Warning") : title
			, msg
			, QMessageBox::Close
			, parent != NULL ? parent : m_mainWindow
		);
		QCheckBox *check = new QCheckBox(tr("Don't Remind Me Again"), &msgBox);
		msgBox.setCheckBox(check);

		msgBox.exec();
		QSettings().setValue(_NOTICE_WARNING_AGGRESSIVE, !check->isChecked());
	}
	else {
		qDebug() << msg;
	}
}


void
NativeApp::displayInfo(const QString &msg, const QString &title, QWidget *parent) const
{
	if (this->thread() == QThread::currentThread()) {
		QMessageBox msgBox(
			QMessageBox::Information
			, title.isEmpty() ? tr("Information") : title
			, msg
			, QMessageBox::Close
			, parent != NULL ? parent : m_mainWindow
		);

		msgBox.exec();
	}
	else {
		qDebug() << msg;
	}
}

QString NativeApp::getOpenFileName(QWidget *parent, const QStringList &filters, const QString &prompt, const QString &defaultPath/* = QString()*/) {
	int index = getSelectedFilter(filters, defaultPath);
	QString selected = index >= 0 ? filters[index] : QString();
	QString initDir = defaultPath.isNull() ? currentDocPath().absolutePath() : defaultPath;
	QString fileName = QFileDialog::getOpenFileName(parent, prompt, initDir, filters.join(";;"), &selected);

	if (defaultPath.isNull() && !fileName.isEmpty()) {
		setCurrentDocPath(QFileInfo(fileName).dir());
	}
	return fileName;
}

QString NativeApp::getSaveFileName(QWidget * parent, const QString & caption, const QString & directory, const QString & filter, const QStringList & saveAbleExtensions) {
	QString saveFileName;

	QFileDialog dialog(parent);

	saveFileName = dialog.getSaveFileName(parent, caption, directory, filter, 0, QFileDialog::DontConfirmOverwrite);

	if (saveFileName.isEmpty())
		return "";

	QStringList parts = saveFileName.split(".");
	QString extension = parts.last();

	if (saveAbleExtensions.size())
	{
		if (extension != saveAbleExtensions.first())
		{
			saveFileName += "." + saveAbleExtensions.first();
		}
	}


	//now we check if the file already exists, and if we want to replace it.
	if (QFile::exists(saveFileName))
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText(saveFileName + tr(" already exists"));
		msgBox.setInformativeText(tr("Do you want to overwrite it?"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::No);
		int choice = msgBox.exec();
		if (choice == QMessageBox::No)
		{
			return "";
		}
	}
	return saveFileName;
}

QString NativeApp::buildNumber()const {
	QDate buildDate = compilationDate();

	// 	//     Return   4   digits   build   number.
	// 	//     For   example,   the   number   of   build   made   in   March   8th,   2000   is   1508.
	// 	//     the   15   is   number   of   monthes   from   the   build   day   to   December   1st,   1998.
	// 	//     the   08   is   the   date   number   of   the   build   day.
	// 	int uYear = buildDate.year();
	// 	int uMonth = buildDate.month();
	// 	int uDay = buildDate.day();
	// 	int uBuildNum   =   ((uYear - 2000)   *   12   +   uMonth)   *   100   +   uDay;

	QString buildNum = buildDate.toString("yyMMdd");

	return  buildNum;
}

NativeApp &
NativeApp::instance() {
	QCoreApplication *a = QApplication::instance();
	if (a != NULL) {
		if (NativeApp *padapp = dynamic_cast<NativeApp *>(a)) {
			return *padapp;
		}
	}
	throw (tr("Application instance not created yet"));
}
