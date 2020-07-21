/***************************************************************************
 **
 **  Copyright (C) 2019 MaMinJie <canpool@163.com>
 **  Contact: https://github.com/canpool
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
#ifndef LITEWINDOW_P_H
#define LITEWINDOW_P_H

#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>

namespace QLite
{
	class LiteBar;
	class LiteWindowPrivate : public QObject
	{
		Q_OBJECT
	public:
		LiteWindowPrivate();
		~LiteWindowPrivate();

		LiteBar *m_liteBar;
		QStatusBar *m_statusBar;
		QMenuBar *m_menuBar;
		QHBoxLayout *m_layout;

	public slots:
		void slotResizable(bool resizable);
	};
}

#endif // LITEWINDOW_P_H
