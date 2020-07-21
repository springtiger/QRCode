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


#include "qrcodewidget.h"
#include "moc/moc_qrcodewidget.cpp"

#include <QComboBox>
#include <QLineEdit>
#include <QAbstractButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>

QRCodeWidget::QRCodeWidget(QWidget *parent)
	: LiteWidget(parent)
{
}

QRCodeWidget::~QRCodeWidget()
{
}

void
QRCodeWidget::connectModifiedSignals()
{
	foreach(QComboBox *widget, findChildren<QComboBox *>()) {
		connect(widget, SIGNAL(activated(int)), this, SIGNAL(modified()));
	}
	foreach(QLineEdit *widget, findChildren<QLineEdit *>()) {
		connect(widget, SIGNAL(textEdited(const QString &)), this, SIGNAL(modified()));
	}
	foreach(QAbstractButton *widget, findChildren<QAbstractButton *>()) {
		connect(widget, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
	}
	foreach(QSpinBox *widget, findChildren<QSpinBox *>()) {
		connect(widget, SIGNAL(valueChanged(int)), this, SIGNAL(modified()));
	}
	foreach(QDoubleSpinBox *widget, findChildren<QDoubleSpinBox *>()) {
		connect(widget, SIGNAL(valueChanged(double)), this, SIGNAL(modified()));
	}
	foreach(QGroupBox *widget, findChildren<QGroupBox *>()) {
		if (widget->isCheckable())
			connect(widget, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
	}
}