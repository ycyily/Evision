#include "StereoCamera.h"
#include <QMessageBox>
#include <QPalette>
#include <QMediaMetaData>
#include <QtWidgets>
#include <QVariant>
Q_DECLARE_METATYPE(QCameraInfo)

StereoCamera::StereoCamera(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();

	if(availableCameras.size()<=1)
	{
		this->close();
		QMessageBox::information(NULL, QStringLiteral("����"), QStringLiteral("��������ͷ����С��2,�޷������ù���!"));
		return;
	}

	int id = 0;
	for (const QCameraInfo &cameraInfo : availableCameras) {
		ui.comboBox_Lcam->addItem(QString::fromStdString(std::to_string(id) + "-") + cameraInfo.description(), QVariant::fromValue(cameraInfo));
		ui.comboBox_Rcam->addItem(QString::fromStdString(std::to_string(id) + "-") + cameraInfo.description(), QVariant::fromValue(cameraInfo));
		id++;
	}
	ui.comboBox_Lcam->setCurrentIndex(0);
	ui.comboBox_Lcam->setCurrentIndex(1);
	setLCamera(static_cast<QCameraInfo*>(ui.comboBox_Lcam->currentData().data()));
	setRCamera(static_cast<QCameraInfo*>(ui.comboBox_Rcam->currentData().data()));

}

StereoCamera::~StereoCamera()
{
}
//���������
void StereoCamera::setLCamera(QCameraInfo* cameraInfo)
{
	m_pLCamera.reset(new QCamera(*cameraInfo));
	m_pLImageCapture.reset(new QCameraImageCapture(m_pLCamera.data()));
	//���òɼ�Ŀ��
	//m_pCamera->setCaptureMode(QCameraImageCapture::CaptureToFile);
	refreshResAndCodecListL();
	//���òɼ�ģʽ
	m_pLCamera->setCaptureMode(QCamera::CaptureStillImage);//����ɼ�ΪͼƬ
	m_pLCamera->setCaptureMode(QCamera::CaptureMode::CaptureViewfinder);//����ɼ���ȡ������
	//����ȡ����
	m_pLCamera->setViewfinder(ui.viewfinderL);
	//�������
	m_pLCamera->start();
}
//���������
void StereoCamera::setRCamera(QCameraInfo* cameraInfo)
{
	m_pRCamera.reset(new QCamera(*cameraInfo));
	m_pRImageCapture.reset(new QCameraImageCapture(m_pRCamera.data()));
	//���òɼ�Ŀ��
	//m_pCamera->setCaptureMode(QCameraImageCapture::CaptureToFile);
	refreshResAndCodecListR();
	//���òɼ�ģʽ
	m_pRCamera->setCaptureMode(QCamera::CaptureStillImage);//����ɼ�ΪͼƬ
	m_pRCamera->setCaptureMode(QCamera::CaptureMode::CaptureViewfinder);//����ɼ���ȡ������
	//����ȡ����
	m_pRCamera->setViewfinder(ui.viewfinder_R);
	//�������
	m_pRCamera->start();
}

void StereoCamera::refreshResAndCodecListL()
{
	ui.comboBox_LCodec->clear();
	ui.comboBox_LCodec->addItem(tr("Default image format"), QVariant(QString()));
	const QStringList supportedImageCodecs = m_pLImageCapture->supportedImageCodecs();
	for (const QString &codecName : supportedImageCodecs) {
		QString description = m_pLImageCapture->imageCodecDescription(codecName);
		ui.comboBox_LCodec->addItem(codecName + ": " + description, QVariant(codecName));
	}
	ui.comboBox_Lresolution->clear();
	ui.comboBox_Lresolution->addItem(tr("Default Resolution"));
	const QList<QSize> supportedResolutions = m_pLImageCapture->supportedResolutions();
	for (const QSize &resolution : supportedResolutions) {
		ui.comboBox_Lresolution->addItem(QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
			QVariant(resolution));
	}
	ui.horizontalSlider_LQuality->setRange(0, int(QMultimedia::VeryHighQuality));
}

void StereoCamera::refreshResAndCodecListR()
{
	ui.comboBox_RCodec->clear();
	ui.comboBox_RCodec->addItem(tr("Default image format"), QVariant(QString()));
	const QStringList supportedImageCodecs = m_pRImageCapture->supportedImageCodecs();
	for (const QString &codecName : supportedImageCodecs) {
		QString description = m_pRImageCapture->imageCodecDescription(codecName);
		ui.comboBox_RCodec->addItem(codecName + ": " + description, QVariant(codecName));
	}
	ui.comboBox_Rresolution->clear();
	ui.comboBox_Rresolution->addItem(tr("Default Resolution"));
	const QList<QSize> supportedResolutions = m_pRImageCapture->supportedResolutions();
	for (const QSize &resolution : supportedResolutions) {
		ui.comboBox_Rresolution->addItem(QString("%1x%2").arg(resolution.width()).arg(resolution.height()),
			QVariant(resolution));
	}
	ui.horizontalSlider_RQuality->setRange(0, int(QMultimedia::VeryHighQuality));
}

QVariant StereoCamera::boxValue(const QComboBox* box) const
{
	int idx = box->currentIndex();
	if (idx == -1)
		return QVariant();

	return box->itemData(idx);
}

void StereoCamera::OnFindSavePath()
{
	QFileDialog * fileDialog2 = new QFileDialog();
	fileDialog2->setWindowTitle(QStringLiteral("��ѡ�񱣴�λ��"));
	//fileDialog2->setNameFilter(QStringLiteral("�����ļ�(*.xml *.yml *.yaml)"));
	fileDialog2->setFileMode(QFileDialog::DirectoryOnly);
	if (fileDialog2->exec() == QDialog::Accepted)
	{
		saveToHere = fileDialog2->selectedFiles().at(0);
		ui.lineEdit_SaveTo->setText(saveToHere);
	}
}

//����
void StereoCamera::OnShot()
{
	m_pLImageCapture->capture();
	m_pRImageCapture->capture();
}

void StereoCamera::OnValueChanged_LExposureCompensation(int value)
{
	m_pLCamera->exposure()->setExposureCompensation(value*0.5);
}

void StereoCamera::OnValueChanged_RExposureCompensation(int value)
{
	m_pRCamera->exposure()->setExposureCompensation(value*0.5);
}

void StereoCamera::OnValueChanged_LQuality(int value)
{
	QImageEncoderSettings settings = m_pLImageCapture->encodingSettings();
	settings.setQuality(QMultimedia::EncodingQuality(ui.horizontalSlider_LQuality->value()));
	m_pLImageCapture->setEncodingSettings(settings);
}

void StereoCamera::OnValueChanged_RQuality(int value)
{
	QImageEncoderSettings settings = m_pRImageCapture->encodingSettings();
	settings.setQuality(QMultimedia::EncodingQuality(ui.horizontalSlider_RQuality->value()));
	m_pRImageCapture->setEncodingSettings(settings);
}

void StereoCamera::OnSelectedChanged_LCameraDevice(QString value)
{
	setLCamera(static_cast<QCameraInfo*>(ui.comboBox_Lcam->currentData().data()));
}

void StereoCamera::OnSelectedChanged_RCameraDevice(QString value)
{
	setRCamera(static_cast<QCameraInfo*>(ui.comboBox_Rcam->currentData().data()));
}

void StereoCamera::OnSelectedChanged_LResolution(QString value)
{
	QImageEncoderSettings settings = m_pLImageCapture->encodingSettings();
	settings.setResolution(boxValue(ui.comboBox_Lresolution).toSize());
	m_pLImageCapture->setEncodingSettings(settings);
}

void StereoCamera::OnSelectedChanged_RResolution(QString value)
{
	QImageEncoderSettings settings = m_pRImageCapture->encodingSettings();
	settings.setResolution(boxValue(ui.comboBox_Rresolution).toSize());
	m_pRImageCapture->setEncodingSettings(settings);
}

void StereoCamera::OnSelectedChanged_LCodec(QString value)
{
	QImageEncoderSettings settings = m_pLImageCapture->encodingSettings();
	settings.setCodec(boxValue(ui.comboBox_LCodec).toString());
	m_pLImageCapture->setEncodingSettings(settings);
}

void StereoCamera::OnSelectedChanged_RCodec(QString value)
{
	QImageEncoderSettings settings = m_pRImageCapture->encodingSettings();
	settings.setCodec(boxValue(ui.comboBox_RCodec).toString());
	m_pRImageCapture->setEncodingSettings(settings);
}
