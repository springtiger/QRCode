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


#ifndef	__QRCODE_WIDGET_H__
#define	__QRCODE_WIDGET_H__

#include "litewidget.h"

using namespace QLite;

class QRCodeWidget : public LiteWidget
{
	Q_OBJECT

public:
	QRCodeWidget(QWidget *parent);
	virtual ~QRCodeWidget();

	virtual void applyChanges() = 0;
	virtual void reload() = 0;

signals:
	void modified();
	void stateChanged(const QString &message);

protected:
	void connectModifiedSignals();
};

#endif	// __QRCODE_WIDGET_H__
