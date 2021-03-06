#include "Calibration.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <iostream>

using namespace std;
using namespace cv;

void Calibration::computeHomography()
{
	///////////////////////////////////////////////////////////////////////////
	//
	// To do:
	//
	// Compute the homography matrices here in order to transform points between
	// the different spaces. These variables will help you doing so:
	//
	// * m_projectorCoordinates: Vector of 4 points you have clicked to
	//                           calibrate the projector
	// * m_cameraCoordinates: Vector of 4 points you have clicked to calibrate
	//                        the camera.
	//
	// At the end, these matrices should be set:
	//
	// * m_physicalToProjector
	// * m_projectorToPhysical
	// * m_physicalToCamera
	// * m_cameraToPhysical
	//
	///////////////////////////////////////////////////////////////////////////
    vector<Point2f> physicalCoordinates = vector<Point2f>(); // these should be window coords...
    physicalCoordinates.push_back(Point2f(0,480));
    physicalCoordinates.push_back(Point2f(480,480));
    physicalCoordinates.push_back(Point2f(480,0));
    physicalCoordinates.push_back(Point2f(0,0));
    
    m_physicalToProjector = cv::getPerspectiveTransform(physicalCoordinates, m_projectorCoordinates);
    m_projectorToPhysical = cv::getPerspectiveTransform(m_projectorCoordinates, physicalCoordinates);
    m_physicalToCamera = cv::getPerspectiveTransform(physicalCoordinates, m_cameraCoordinates);
    m_cameraToPhysical = cv::getPerspectiveTransform(m_cameraCoordinates, physicalCoordinates);
    
    // compute projetor -> physical
//    m_projectorToPhysical = getPerspectiveTransform(physicalCoordinates, m_projectorCoordinates);
    
    // compute physical -> projector
//    invert(m_projectorToPhysical, m_physicalToProjector);
    
    // compute projector -> camera
//    Mat projectorToCamera = getPerspectiveTransform(m_cameraCoordinates, m_projectorCoordinates);
    
    // camera -> projector
//    Mat cameraToProjector;
//    invert(projectorToCamera, cameraToProjector);
    
    // compute camera -> physical
//    m_cameraToPhysical = getPerspectiveTransform(physicalCoordinates, m_cameraCoordinates); //cameraToProjector * m_projectorToPhysical;
    
    // physical -> camera
//    invert(m_cameraToPhysical, m_physicalToCamera);

}

void Calibration::getHomographyFromCalibration(Mat calibration, Mat &homography) {
////    Mat calibrationMat = Mat(1,8,CV_32FC1);
////    
////    
////    
////    SVD svd = SVD(calibrationMat, SVD::FULL_UV);
//    homography = Mat(3, 3, CV_32FC1);
////
////    for (int i = 0; i < 9; i++) {
////        homography.at<float>(i / 3, i % 3) =
////        svd.vt.at<float>(8, i);
////    }
//    findHomography(calibration, homography)
}

void mouseCallback(int event, int x, int y, int flags, void *pointer);

Calibration::Calibration()
{
	restart();

	m_circleNames.push_back("bottom-left (red)");
	m_circleNames.push_back("bottom-right (green)");
	m_circleNames.push_back("top-right (blue)");
	m_circleNames.push_back("top-left (black)");

	m_calibrationImage = cv::Mat(600, 800, CV_8UC3);
}

void Calibration::restart()
{
	m_hasTerminated = false;
	m_isProjectorCalibrated = false;
	m_isCameraCalibrated = false;

	m_numberOfProjectorCoordinates = 0;
	m_numberOfCameraCoordinates = 0;

	m_projectorCoordinates.clear();
	m_cameraCoordinates.clear();

	cv::destroyWindow("UIST game");
	cv::destroyWindow("output");
	cv::destroyWindow("depth");
	cv::destroyWindow("bgr");
	cv::namedWindow("calibration", CV_WINDOW_NORMAL);
	cv::setMouseCallback("calibration", mouseCallback, this);
}

bool Calibration::hasTerminated() const
{
	return m_hasTerminated;
}

void Calibration::loop(const cv::Mat &bgrImage, const cv::Mat &depthImage)
{
	// Reset the calibration wizard image
	m_calibrationImage = cv::Mat::zeros(480, 640, CV_8UC3);

	// Run the calibration wizard
	calibrate(bgrImage);

	// Show the calibration wizard
	if (!m_hasTerminated)
		cv::imshow("calibration", m_calibrationImage);
}

void Calibration::calibrate(const cv::Mat &bgrImage)
{
	// First, calibrate the projector
	if(!m_isProjectorCalibrated)
	{
		calibrateProjector();
		return;
	}

	// Then, calibrate the camera
	if (!m_isCameraCalibrated)
	{
		calibrateCamera(bgrImage);
		return;
	}

	// If both are calibrated, compute the homography
	computeHomography();

	// Finally hide the calibration wizard and show the UIST game instead
	cv::destroyWindow("calibration");
	cv::namedWindow("UIST game", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("output", CV_WINDOW_NORMAL);
	cv::namedWindow("depth", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("bgr", CV_WINDOW_AUTOSIZE);

	m_hasTerminated = true;
}

void Calibration::calibrateProjector()
{
	std::stringstream info;
	info << "Click on the " << m_circleNames[m_numberOfProjectorCoordinates]
		<< " circle.";

	cv::putText(m_calibrationImage, info.str(), cv::Point2i(16, 32),
		cv::FONT_HERSHEY_SIMPLEX, 0.7f, cv::Scalar(192, 192, 192), 1,
		CV_AA);
}

void Calibration::calibrateCamera(const cv::Mat &bgrImage)
{
	cv::Mat destinationRegionOfInterest
		= m_calibrationImage(cv::Rect(0, 0, bgrImage.cols, bgrImage.rows));

	// Show the BGR image in order to click on it
	bgrImage.copyTo(destinationRegionOfInterest);

	std::stringstream info;
	info << "Click on the " << m_circleNames[m_numberOfCameraCoordinates]
		<< " circle.";

	cv::putText(m_calibrationImage, info.str(), cv::Point2i(16, 32),
		cv::FONT_HERSHEY_SIMPLEX, 0.7f, cv::Scalar(192, 192, 192), 1,
		CV_AA);
}

void Calibration::handleMouseClick(int x, int y, int flags)
{
	// If the projector is not calibrated, save the clicked point as a
	// projector calibration point
	if (m_numberOfProjectorCoordinates < 4)
	{
		m_projectorCoordinates.push_back(cv::Point2f((float)x, (float)y));
		m_numberOfProjectorCoordinates++;

		if(m_numberOfProjectorCoordinates >= 4)
		{
			m_isProjectorCalibrated = true;
			std::cout << "Finished calibrating the projector." << std::endl;
		}

		return;
	}

	// If the projector is calibrated, but not the camera, save the
	// clicked point as a camera calibration point
	if (m_numberOfCameraCoordinates < 4)
	{
		m_cameraCoordinates.push_back(cv::Point2f((float)x, (float)y));
		m_numberOfCameraCoordinates++;

		if(m_numberOfCameraCoordinates >= 4)
		{
			m_isCameraCalibrated = true;
			std::cout << "Finished calibrating the camera." << std::endl;
		}

		return;
	}
}


const cv::Mat &Calibration::physicalToProjector() const
{
	return m_physicalToProjector;
}

const cv::Mat &Calibration::projectorToPhysical() const
{
	return m_projectorToPhysical;
}

const cv::Mat &Calibration::physicalToCamera() const
{
	return m_physicalToCamera;
}

const cv::Mat &Calibration::cameraToPhysical() const
{
	return m_cameraToPhysical;
}

void mouseCallback(int event, int x, int y, int flags, void *calib)
{
	if (event != CV_EVENT_LBUTTONDOWN)
		return;

	Calibration* calibration = static_cast<Calibration*>(calib);

	if (calibration)
		calibration->handleMouseClick(x, y, flags);
}

Calibration::~Calibration()
{
}
