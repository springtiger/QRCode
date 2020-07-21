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

#include "aboutform.h"
#include "uic/ui_about.h"
#include "moc/moc_aboutform.cpp"
#include "nativeapp.h"

class AboutFormUi : public Ui::aboutform {

};

AboutForm::AboutForm(QWidget *parent/*, Qt::WindowFlags flags*/) :
	QRCodeWidget(parent/*, flags*/)
{

	m_ui.reset(new AboutFormUi);
	m_ui->setupUi(this);

	//application version
	QString version = app().applicationVersion();
	version += QString(" (%1)").arg(app().buildNumber());
	QString app_name = app().applicationDisplayName();
	m_ui->version->setText(tr("%1 Version %2").arg(app_name).arg(version));

	//license
	QByteArray license;
	QFile f(":/images/license.png");
	if (f.open(QFile::ReadOnly)) {
		license = f.readAll();
		f.close();
	}
	m_ui->plainTE->setPlainText(QString::fromUtf8(license));

	//about Qt
	connect(m_ui->btnAboutQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
}

AboutForm::~AboutForm()
{
}

void AboutForm::setText(QString content) {
	m_ui->info->setText(content);
}

void AboutForm::applyChanges()
{
}

void AboutForm::reload()
{
}
