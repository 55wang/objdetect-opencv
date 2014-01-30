#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/video/background_segm.hpp>
#include <time.h>
#include <stdio.h>
using namespace cv;
using namespace std;

Mat frame, frame_thresholded, roi, roi2;
int Y_MIN = 0, Y_MAX = 0, Cr_MIN = 0, Cr_MAX = 0, Cb_MIN = 0, Cb_MAX = 0	;
int a=4, b=55, c=0, d=62, e=255, f=145;
const int a_max=255, b_max=255, c_max=255, d_max=255, e_max=255, f_max=255;
int thresh = 100;
void thresh_callback(int, void* );

void on_trackbar(int, void*) {
    Y_MIN=a;
    //cout << Y_MIN << " " << Y_MAX << " "<< Cr_MIN << " "<< Cr_MAX << " " << Cb_MIN << " " << Cb_MAX << endl;
}
void on_trackbar2(int, void*) {
    Cr_MIN=b;
}
void on_trackbar3(int, void*) {
    Cb_MIN=c;
}
void on_trackbar4(int, void*) {
    Y_MAX=d;
}
void on_trackbar5(int, void*) {
    Cr_MAX=e;
}
void on_trackbar6(int, void*) {
    Cb_MAX=f;
}

Mat getSkin(Mat input) {
    Mat Skin;
    cvtColor(input,Skin,COLOR_BGR2YCrCb);
    inRange(Skin, Scalar(Y_MIN,Cr_MIN,Cb_MIN), Scalar(Y_MAX, Cr_MAX, Cb_MAX), Skin);
    
    return Skin;
}

void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    
    cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
    cv::Rect r = cv::boundingRect(contour);
    
    cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
    cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
    cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}


int main() {
    // Create a VideoCapture object to read from video file
    // 0 is the ID of the built-in laptop camera, change if you want to use other camera
    VideoCapture cap(0);
    cap.set(3,640);
    cap.set(4,480);
    //check if the file was opened properly
    if(!cap.isOpened())
    {
        cout << "Capture could not be opened succesfully" << endl;
        return -1; }
    namedWindow("Video");
    namedWindow("Segmentation");
    //namedWindow("top");
    //namedWindow("bottom");
    
    waitKey(200);
    
    while(char(waitKey(1)) != 'q' && cap.isOpened())
    {
        cap >> frame;
        // Check if the video is over
        if(frame.empty())
        {
            cout << "Video over" << endl;
            break;
        }
        
        GaussianBlur(frame, frame, Size(3, 3), 0, 0);
        
        createTrackbar("Y_MIN", "Segmentation", &a, a_max, on_trackbar);
        createTrackbar("CR_MIN", "Segmentation", &b, b_max, on_trackbar2);
        
        createTrackbar("CB_MIN", "Segmentation", &c, c_max, on_trackbar3);
        createTrackbar("Y_MAX", "Segmentation", &d, d_max, on_trackbar4);
        
        createTrackbar("CR_MAX", "Segmentation", &e, e_max, on_trackbar5);
        createTrackbar("CB_MAX", "Segmentation", &f, f_max, on_trackbar6);
        
        on_trackbar(a,0);
        on_trackbar2(b,0);
        on_trackbar3(c,0);
        on_trackbar4(d,0);
        on_trackbar5(e,0);
        on_trackbar6(f,0);
        
        flip(frame, frame, 1);
        frame_thresholded=getSkin(frame);
        dilate(frame_thresholded,frame_thresholded, Mat());
        
        imshow("Video", frame);
        imshow("Segmentation", frame_thresholded);
        Mat roi(frame_thresholded, Rect(0,0,640,80));
        
        Mat roi2(frame_thresholded, Rect(0,400,640,80));
        
        vector<std::vector<cv::Point> > contours;
        findContours(roi.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        
        vector<cv::Point> approx;
        Mat dst = frame_thresholded.clone();
        for (int i = 0; i < contours.size(); i++)
        {
            // Approximate contour with accuracy proportional
            // to the contour perimeter
            cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
            
            // Skip small or non-convex objects
            if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
                continue;
            
            setLabel(roi, "Detected", contours[i]);
            //cout << contours[i] <<" "<<contours.size()<< endl;
            
            
            
        }
        
        Mat threshold_output;
        vector<Vec4i> hierarchy;
        
        /// Detect edges using Threshold
        threshold(roi, threshold_output, thresh, 255, THRESH_BINARY );
        /// Find contours
        findContours( threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        
        /// Approximate contours to polygons + get bounding rects and circles
        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );
        vector<Point2f>center( contours.size() );
        
        vector<float>radius( contours.size() );
        
        for( int i = 0; i < contours.size(); i++ )
        { approxPolyDP( Mat(contours[i]), contours_poly[i], 5, true );
            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
            minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
        }
        
        
        /// Draw polygonal contour + bonding rects + circles
        Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
        for(int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar(255,0,0);
            drawContours(drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
            circle(drawing, center[i], (int)radius[i], color, 2, 8, 0 );
            setLabel(drawing, "Detected", contours[i]);
            //cout << boundRect[i].tl()<< endl;
            
            
            /// Get the moments
            vector<Moments> mu(contours.size() );
            for( int i = 0; i < contours.size(); i++ )
            { mu[i] = moments( contours[i], false ); }
            
            ///  Get the mass centers:
            vector<Point2f> mc( contours.size() );
            for( int i = 0; i < contours.size(); i++ )
            { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

            cout << mc[i].x <<" " << mc[i].y<< endl;
            
            
            if (mc[i].x <200){
                cout << "Robot move RIGHT" << endl;
            } else if (mc[i].x >400) {
                cout << "Robot move LEFT" << endl;
            } else if (mc[i].x>200 && mc[i].x <400) {
                cout << "Robot move FORWARD" << endl;
            }
            
            cout<<endl<<endl;
            
        }
        
        
        /// Show in a window
        namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
        imshow( "Contours", drawing );
        
        //imshow("top",roi);
        //imshow("bottom",roi2);
        
    }
    
    cap.release();
    destroyWindow("Video");
    destroyWindow("Segmentation");
    destroyWindow("top");
    destroyWindow("bottom");
    
    return 0;
}



