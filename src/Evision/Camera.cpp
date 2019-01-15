#include "Camera.h"
#include <QMessageBox>
#include <QPalette>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QVariant>
#include <QCameraInfo>
#include <QCameraImageCapture>

Q_DECLARE_METATYPE(QCameraInfo)

Camera::Camera(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	//Camera devices:

	const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
	int id = 0;
	for (const QCameraInfo &cameraInfo : availableCameras) {
		ui.comboBox_CameraDevice->addItem(QString::fromStdString(std::to_string(id)+"-")+cameraInfo.description(),QVariant::fromValue(cameraInfo));
		id++;
	}

	setCamera(static_cast<QCameraInfo*>(ui.comboBox_CameraDevice->currentData().data()));
	ui.lcdNumber->display(0);
	ui.pushButton_CameraOn->setEnabled(false);
	ui.pushButton_CameraOff->setEnabled(true);

	ui.lineEdit_SavePath->setText(QDir::currentPath());
}

Camera::~Camera()
{
	m_pCamera->stop();
}
/*
 *setCamera：设置相机
 *QCameraInfo* ：相机信息的指针
 *
 *return：void
 */
void Camera::setCamera(QCameraInfo* cameraInfo)
{	
	m_pCamera.reset(new QCamera(*cameraInfo));
	m_pImageCapture.reset(new QCameraImageCapture(m_pCamera.data()));
	//设置采集目标
	//m_pCamera->setCaptureMode(QCameraImageCapture::CaptureToFile);
	refreshResAndCodecList();
	//设置采集模式
	m_pCamera->setCaptureMode(QCamera::CaptureStillImage);//将其采集为图片
	m_pCamera->setCaptureMode(QCamera::CaptureMode::CaptureViewfinder);//将其采集到取景器中
	//设置取景器
	m_pCamera->setViewfinder(ui.viewfinder);
	//开启相机
	m_pCamera->start();

}

void Camera::refreshResAndCodecList()
{
	ui.comboBox_Codec->clear();
	ui.comboBox_Codec->addItem(tr("Default image format"), QVariant(QString()));
	const QStringList supportedImageCodecs = m_pImageCapture->supportedImageCodecs();
	for (const QString &codecName : supportedImageCodecs) {
		QString description = m_pImageCapture->imageCodecDescription(codecName);
		ui.comboBox_Codec->addItem(codecName + ": " + description, QVariant(codecName));
	}
	ui.comboBox_Resolution->clear();
	ui.comboBox_Resolution->addItem(tr("Default Resolution"));
	const QList<QSize> supportedResolutions = m_pImageCapture->supportedResolutions();
	for (const QSize &resolution : supportedResolutions) {
		ui.comboBox_Resolution->addItem(QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
			QVariant(resolution));
	}
	ui.horizontalSlider_Quality->setRange(0, int(QMultimedia::VeryHighQuality));
}

QVariant Camera::boxValue(const QComboBox* box) const
{
	int idx = box->currentIndex();
	if (idx == -1)
		return QVariant();

	return box->itemData(idx);
}

void Camera::closeEvent(QCloseEvent * e)
{
	m_pCamera->stop();
}

void Camera::OnFindSavePath()
{
	QFileDialog * fileDialog2 = new QFileDialog();
	fileDialog2->setWindowTitle(QStringLiteral("请选择保存位置"));
	//fileDialog2->setNameFilter(QStringLiteral("点云文件(*.xml *.yml *.yaml)"));
	fileDialog2->setFileMode(QFileDialog::DirectoryOnly);
	if (fileDialog2->exec() == QDialog::Accepted)
	{
		saveToHere = fileDialog2->selectedFiles().at(0);
		ui.lineEdit_SavePath->setText(saveToHere);
	}
}

void Camera::OnCameraPowerOn()
{
	setCamera(static_cast<QCameraInfo*>(ui.comboBox_CameraDevice->currentData().data()));
	ui.pushButton_CameraOn->setEnabled(false);
	ui.pushButton_CameraOff->setEnabled(true);
	ui.pushButton_Focus->setEnabled(true);
	ui.pushButton_Shot->setEnabled(true);
	ui.horizontalSlider_exposureCompensation->setEnabled(true);
	ui.horizontalSlider_Quality->setEnabled(true);
}

void Camera::OnCameraPowerOff()
{
	ui.pushButton_CameraOff->setEnabled(false);
	ui.pushButton_CameraOn->setEnabled(true);
	ui.pushButton_Focus->setEnabled(false);
	ui.pushButton_Shot->setEnabled(false);
	ui.horizontalSlider_exposureCompensation->setEnabled(false);
	ui.horizontalSlider_Quality->setEnabled(false);
	m_pCamera->stop();
}

void Camera::OnFocus()
{
	switch (m_pCamera->lockStatus()) {
	case QCamera::Searching:
	case QCamera::Locked:
		m_pCamera->unlock();
		break;
	case QCamera::Unlocked:
		m_pCamera->searchAndLock();
	}
}

void Camera::OnShot()
{
	QString idt = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
	m_pImageCapture->capture(ui.lineEdit_SavePath->text() + "/S" + idt + ".jpg");
	ui.lcdNumber->display(ui.lcdNumber->intValue()+1);
}

void Camera::OnValueChanged_ExposureCompensation(int value)
{
	m_pCamera->exposure()->setExposureCompensation(value*0.5);
}

void Camera::OnValueChanged_Quality(int value)
{
	QImageEncoderSettings settings = m_pImageCapture->encodingSettings();
	settings.setQuality(QMultimedia::EncodingQuality(ui.horizontalSlider_Quality->value()));
	m_pImageCapture->setEncodingSettings(settings);
}

void Camera::OnSelectedChanged_CameraDevice(QString value)
{
	setCamera(static_cast<QCameraInfo*>(ui.comboBox_CameraDevice->currentData().data()));
}

void Camera::OnSelectedChanged_Resolution(QString value)
{
	QImageEncoderSettings settings = m_pImageCapture->encodingSettings();
	settings.setResolution(boxValue(ui.comboBox_Resolution).toSize());
	m_pImageCapture->setEncodingSettings(settings);
}

void Camera::OnSelectedChanged_Codec(QString value)
{
	QImageEncoderSettings settings = m_pImageCapture->encodingSettings();
	settings.setCodec(boxValue(ui.comboBox_Codec).toString());
	m_pImageCapture->setEncodingSettings(settings);
}




