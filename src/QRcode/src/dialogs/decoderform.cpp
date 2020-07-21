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

#include "decoderform.h"
#include "uic/ui_decoder.h"
#include "moc/moc_decoderform.cpp"
#include "nativeapp.h"
#include "QZXing.h"
#include <QClipboard.h>

//////////////////////////////////////////////////////////////////////////
class DecoderFormUi : public Ui::frmDecoder{
};

DecoderForm::DecoderForm(QWidget *parent) :
	QRCodeWidget(parent)
	, m_ui(new DecoderFormUi)
{
	m_ui->setupUi(this);

	reload();

	connectModifiedSignals();

	m_zxing = new QZXing(QZXing::DecoderFormat_QR_CODE, this);
	m_ui->imgQRCode->setImage(QString(":/images/wx.png"));
}

DecoderForm::~DecoderForm()
{
	delete m_zxing;
}

void DecoderForm::applyChanges()
{
}

void DecoderForm::reload()
{
}

void DecoderForm::on_btnLoad_clicked() {
	QStringList filter;
	filter << tr("PNG Files (*.png)");
	filter << tr("JPG Files (*.jpg)");

	QString img = app().getOpenFileName(this, filter);
	if (img.isEmpty()) {
		return;
	}

	m_ui->imgQRCode->setImage(img);

	QImage image(img);
	if (image.isNull())//图片加载失败函数返回
	{
		QString message = tr("Failed to load picture, please confirm picture format!");
		emit stateChanged(message);
		return;
	}

	//解析图片，经测试发现解析能力挺强的
	QString parseText = m_zxing->decodeImage(image);
	if (parseText.isEmpty())
	{
		QString message = tr("No QR code was scanned from the picture.");
		emit stateChanged(message);
		return;
	}
	else
	{
		m_ui->plainTextEdit->setPlainText(parseText);
		m_ui->plainTextEdit->selectAll();
	}
}
void DecoderForm::on_btnCopy_clicked() {
	QString text = m_ui->plainTextEdit->toPlainText();
	QClipboard *cb = app().clipboard();
	if (NULL != cb) {
		cb->setText(text);
		app().displayInfo(tr("The contents have been copied to the clipboard"));
	}		
}