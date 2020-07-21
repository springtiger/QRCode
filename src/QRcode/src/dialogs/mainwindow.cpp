
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

#include "MainWindow.h"
#include "moc/moc_MainWindow.cpp"
#include "uic/ui_mainwindow.h"
#include "encoderform.h"
#include "decoderform.h"
#include "aboutform.h"
#include "nativeapp.h"

#include "litebar.h"
#include "qlite.h"

const char _MAIN_WINDOW_POS[] = "main/pos";
const char _MAIN_WINDOW_SIZE[] = "main/size";
const char _MAIN_WINDOW_WINDOW_STATE[] = "main/window_state";

//////////////////////////////////////////////////////////////////////////
class MainWindowUi : public Ui::MainWindow {
public:
// 	void setupUi(QDialog *dialog) {
// 		Ui::MainWindow::setupUi(dialog);
// 	}
};

//////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent)
	: LiteWindow(parent)
	, m_ui(new MainWindowUi)
{
	m_ui->setupUi(this);
	app().setMainWindow(this);

// 	setMaximumHeight(600);

	m_btn_close = m_ui->buttonBox->button(QDialogButtonBox::Close);
	connect(m_btn_close, SIGNAL(clicked(bool)), this, SLOT(close()));

	connect(m_ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::currentRowChanged);
	connect(m_ui->listWidget, &QListWidget::itemClicked, [this](QListWidgetItem * item) { currentRowChanged(m_ui->listWidget->row(item)); });
	
	setupSkinActions();


	QRCodeWidget* encoder = new EncoderForm;
	addItem(encoder);

	QRCodeWidget* decoder = new DecoderForm;
	addItem(decoder);


	QRCodeWidget*about = new AboutForm;
	addItem(about);

	readSettings();
}

MainWindow::~MainWindow()
{
	writeSettings();
}

void
MainWindow::readSettings() {
	QSettings settings;

	QPoint defaultPos(200, 200);
	static QSize defaultSize(400, 400);
	if (QApplication::desktop()->screenCount() <= 1) {
		defaultSize = QApplication::desktop()->geometry().size();
		defaultPos = QPoint(0, 0);
	}

	static QPoint pos = settings.value(_MAIN_WINDOW_POS, defaultPos).toPoint();
	QSize size = settings.value(_MAIN_WINDOW_SIZE, defaultSize).toSize();
	Qt::WindowState state = static_cast<Qt::WindowState>(settings.value(_MAIN_WINDOW_WINDOW_STATE, Qt::WindowActive).toInt());
	if (state & Qt::WindowMinimized) {
		state = Qt::WindowNoState;
	}
	move(pos);
	resize(size);
	setWindowState(state);
}

void
MainWindow::writeSettings() {
	QSettings settings;
	settings.setValue(_MAIN_WINDOW_POS, pos());
	settings.setValue(_MAIN_WINDOW_SIZE, size());
	settings.setValue(_MAIN_WINDOW_WINDOW_STATE, static_cast<int>(windowState()));
}

void MainWindow::addItem(QWidget* widget, bool isSelected)
{
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QListWidgetItem* item = new QListWidgetItem(widget->windowIcon(), widget->windowTitle(), m_ui->listWidget);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	item->setSizeHint(QSize(item->sizeHint().width(), 55));
	m_ui->listWidget->adjustSize();

	m_ui->stackedWidget->addWidget(widget);

	if (isSelected) {
		m_ui->listWidget->setItemSelected(item, true);

		currentRowChanged(m_ui->listWidget->row(item));
	}

	QRCodeWidget* settings = qobject_cast<QRCodeWidget*>(widget);
	if (NULL != settings){
		m_settings << settings;
		connect(settings, SIGNAL(stateChanged(const QString&)), SLOT(slotStateChanged(const QString&)));
	}
}

void MainWindow::changeRow(int row)
{
	{
		QSignalBlocker b(m_ui->listWidget);
		m_ui->listWidget->setCurrentRow(-1);
	}
	m_ui->listWidget->setCurrentRow(row);
}

void MainWindow::currentRowChanged(int row){
	m_ui->stackedWidget->setCurrentIndex(row);
}
void
MainWindow::setupSkinActions() {
	if (liteBar()) {
		QAction *action = new QAction(QIcon(":/images/skin.png"), tr("skin"));
		QMenu *menu = new QMenu(tr("skin"));
		action->setMenu(menu);
		liteBar()->addAction(action);
		m_pActionGroup = new QActionGroup(this);
		addSkinItem(menu->addAction(tr("flatdark")), QString(":/images/qss/flatdark.css"));
		addSkinItem(menu->addAction(tr("blue")), QString(":/images/qss/blue.css"));
		addSkinItem(menu->addAction(tr("green")), QString(":/images/qss/green.css"));
		addSkinItem(menu->addAction(tr("darkgreen")), QString(":/images/qss/darkgreen.css"));
		addSkinItem(menu->addAction(tr("red")), QString(":/images/qss/red.css"));

		addSkinItem(menu->addAction(tr("normal")), QString(""));

		emit m_pActionGroup->actions().at(0)->trigger();
	}
}

void MainWindow::addSkinItem(QAction *action, const QString &qss)
{
	m_pActionGroup->addAction(action);
	action->setCheckable(true);
	action->setData(QVariant(qss));
	QObject::connect(action, SIGNAL(triggered()), this, SLOT(slotChangeSkin()));
}

void MainWindow::slotChangeSkin()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		QLite::setStyle(action->data().toString());
	}
}

void MainWindow::slotStateChanged(const QString &message) {
	QStatusBar * bar = statusBar();
	if (bar) {
		bar->showMessage(message);
	}
}
