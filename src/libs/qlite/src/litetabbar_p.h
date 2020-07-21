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
#ifndef LITETABBAR_P_H
#define LITETABBAR_P_H

#include <QToolButton>
#include <QMap>
#include <QBoxLayout>
#include <QAction>
// #include <QResizeEvent>

namespace QLite
{
	class LiteTabBarPrivate : public QObject
	{
		Q_OBJECT
	public:
		LiteTabBarPrivate();
		~LiteTabBarPrivate();

		void init();

		QToolButton *at(int index);

		inline bool validIndex(int index) const { return index >= 0 && index < m_tabs.count(); }
		void setIconSize(QSize size);

	public:
		QList<QToolButton *> m_tabs;
		QList<QToolButton *> m_actionButtons;
		QMap<QAction*, QToolButton*> m_actionTabMap;

		int m_currentIndex;
		LiteTabBar::Direction m_direction;
		int m_spacing;
		int m_totalHeight;
		QSize m_iconSize;

		QBoxLayout *m_frontActionLayout;
		QBoxLayout *m_middleActionLayout;
		QBoxLayout *m_backActionLayout;
		QBoxLayout *m_tabLayout;
		QBoxLayout *m_mainLayout;
		LiteTabBar *q;

		public slots:
		void switchTab(bool checked);
		void pressTab();
	};
}
#endif // LITETABBAR_P_H
