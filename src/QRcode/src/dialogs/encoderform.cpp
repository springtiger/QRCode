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

#include "encoderform.h"
#include "uic/ui_encoder.h"
#include "moc/moc_encoderform.cpp"
#include "nativeapp.h"
#include <QProcess>
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QDebug>

//////////////////////////////////////////////////////////////////////////
void QRCodePara::save() {
	QSettings settings;
	settings.setValue("qrcode_parameter/colorized", m_c);
	settings.setValue("qrcode_parameter/correction_level", m_l);
	settings.setValue("qrcode_parameter/length", m_v);
	settings.setValue("qrcode_parameter/contrast", m_con);
	settings.setValue("qrcode_parameter/brightness", m_bri);
}

void QRCodePara::load() {
	QSettings settings;
	m_c = settings.value("qrcode_parameter/colorized", true).toBool();
	m_l = settings.value("qrcode_parameter/correction_level", 'L').toChar();
	m_v = settings.value("qrcode_parameter/length", 10).toInt();
	m_con = settings.value("qrcode_parameter/contrast", 1.).toReal();
	m_bri = settings.value("qrcode_parameter/brightness", 1.).toReal();
}

//////////////////////////////////////////////////////////////////////////
class EncoderFormUi : public Ui::frmEncoder{

public:
	QRCodePara m_para;

	QString m_imgSrc;
};

EncoderForm::EncoderForm(QWidget *parent) :
	QRCodeWidget(parent)
	, m_ui(new EncoderFormUi)
{
	m_ui->setupUi(this);

	reload();

	connectModifiedSignals();

	QDir dir(app().applicationDirPath());
	m_process.setProgram(dir.absoluteFilePath("qr.exe"));
	connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotGenFinished(int, QProcess::ExitStatus)));
	connect(&m_process, SIGNAL(readyReadStandardOutput()), SLOT(slotGenReadyReadStandardOutput()));
	m_ui->imgSrc->setImage(QString(":/images/wx.png"));
}

EncoderForm::~EncoderForm()
{
}

void EncoderForm::applyChanges()
{
	m_ui->m_para.m_v = m_ui->sbV->value();
	m_ui->m_para.m_con = m_ui->dsbCon->value();
	m_ui->m_para.m_l = m_ui->cmbL->currentText()[0];
	m_ui->m_para.m_bri = m_ui->dsbBri->value();
	m_ui->m_para.m_c = m_ui->cbC->isChecked();

	m_ui->m_para.save();
}

void EncoderForm::reload()
{
	m_ui->m_para.load();
	m_ui->sbV->setValue(m_ui->m_para.m_v);
	m_ui->dsbCon->setValue(m_ui->m_para.m_con);
	m_ui->cmbL->setCurrentText(m_ui->m_para.m_l);
	m_ui->dsbBri->setValue(m_ui->m_para.m_bri);
	m_ui->cbC->setChecked(m_ui->m_para.m_c);
}

void EncoderForm::on_btnLoad_clicked() {
	QStringList filter;
	filter << tr("PNG Files (*.png)");
	filter << tr("JPG Files (*.jpg)");

	QString img = app().getOpenFileName(this, filter);
	if (img.isEmpty()) {
		return;
	}
	m_ui->imgSrc->setImage(img);
	m_ui->m_imgSrc = img;

	on_btnGen_clicked();
}

void EncoderForm::on_btnGen_clicked() {
	QString program = m_process.program();
	if (program.isEmpty()) {
		QDir dir(app().applicationDirPath());
		program = dir.absoluteFilePath("qr.exe");
		m_process.setProgram(program);
	}
	if(!QFile::exists(program)){
		app().displayError(tr("The executable does not exist and cannot be generated QRCode."));
		return;
	}

	m_info_filepath = generatorArguments();
	if (m_info_filepath.isEmpty()) {
		return;
	}
	if (m_ui->m_imgSrc.isEmpty())
	{
		return;
	}

	m_process.start();

	return;
	QImage m_img, m_mask;
	QString filename;
	filename = QFileDialog::getOpenFileName(this,
		tr("选择图像"),
		"",
		tr("Images(*.png *.bmp *.jpg *.tif *.GIF)"));
	if (filename.isEmpty())
	{
		return;
	}
	else
	{
		if (!(m_img.load(filename)))//加载图像
		{
			QMessageBox::information(this,
				tr("打开图像失败"),
				tr("打开图像失败!"));

			return;
		}
		m_img.load(filename);
	}

	filename = QFileDialog::getOpenFileName(this,
		tr("选择mask"),
		"",
		tr("Images(*.png *.bmp *.jpg *.tif *.GIF)"));
	if (filename.isEmpty()){
		return;
	}else{
		if (!m_mask.load(filename))//加载图像
		{
			QMessageBox::information(this,
				tr("打开图像失败"),
				tr("打开图像失败!"));
			return;
		}
		m_mask.load(filename);
	}

	QImage *newImage = new QImage(m_img);
	QImage *mask = new QImage(m_mask);

	QPainter painter;
	painter.begin(mask);

	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	painter.drawImage(0, 0, *newImage);

	painter.end();

	m_ui->imgQRCode->setImage(*mask);

	delete mask;
	delete newImage;
}

void EncoderForm::on_pteContent_textChanged() {
	on_btnGen_clicked();
}
QString 
EncoderForm::generatorArguments() {
	QString words = m_ui->pteContent->toPlainText();
	if (words.isEmpty()) {
		app().displayError(tr("The generated content cannot be empty."));
		return QString();
	}
	//get information file folder
	QString program = m_process.program();
	QFileInfo fi(program);
	QString infos = fi.absoluteDir().absoluteFilePath("info.txt");
	if (QFile::exists(infos)) {
		QFile::remove(infos);
	}

	QStringList arguments;
	arguments << "words=" + words;
	arguments << "p=" + m_ui->m_imgSrc;

	arguments << "v=" + QString::number(m_ui->m_para.m_v);
	arguments << "l=" + QString(m_ui->m_para.m_l);
	if (m_ui->m_para.m_c) {
		arguments << "c";
	}
	arguments << "con=" + QString::number(m_ui->m_para.m_con);
	arguments << "bri=" + QString::number(m_ui->m_para.m_bri);

	//generator information file
	QFile file(infos);
	if (file.open(QFile::WriteOnly)) {
		for (const QString & arg : arguments) {
			file.write(arg.toLocal8Bit());
			file.write("\n");
		}
		file.close();
	}

	return infos;
}

void EncoderForm::slotGenFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	qDebug() << exitCode;
	qDebug() << exitStatus;

	QFile::remove(m_info_filepath);

	QFileInfo fi(m_ui->m_imgSrc);
	QString file_name = fi.completeBaseName() + "_qrcode.png";
	QString result = fi.absoluteDir().absoluteFilePath(file_name);
	m_ui->imgQRCode->setImage(result);
}

void EncoderForm::slotGenReadyReadStandardOutput() {
	QByteArray output = m_process.readAllStandardOutput();
	QString message = QString::fromLocal8Bit(output);
	message = message.remove("Enter any key to exit.");
	emit stateChanged(message);
	m_process.kill();
}