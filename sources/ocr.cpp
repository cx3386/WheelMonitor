#include "stdafx.h"

#include "common.h"
#include "confighelper.h"
#include "ocr.h"

//ocr_parameters OCR::p; //init static member of OCR

OCR::OCR(const ConfigHelper* _configHelper, int _deviceIndex, QObject* parent /*= Q_NULLPTR*/)
    : QObject(parent)
    , configHelper(_configHelper)
    , deviceIndex(_deviceIndex)
    , ocrProfile(&(configHelper->device[deviceIndex].ocrProfile))
    , deviceMark(getDeviceMark(deviceIndex))
{
    //加载样本
    for (int i = 0; i < 10; i++) {
        QString patternName = QString("%1/%2.jpg").arg(ocrPatternDirPath).arg(i);
        pattern[i] = imread(patternName.toStdString(), -1);

        if (pattern[i].empty()) {
            qWarning() << "OCR: Could not open or find pattern[" << i << "]";
        }
    }
}

void OCR::core_ocr(Mat src)
{
    vector<Mat> plates = detect_plate(src);
    if (isDbg)
        cout << "find plates " << plates.size();
    for (auto&& pl : plates) {
        recognize(pl);
    }
}

vector<Mat> OCR::detect_plate(Mat frame)
{
    Mat fullscreen;
    resize(frame, fullscreen, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);

    Rect re(1100, 100, 450, 380);
    Mat window(fullscreen, re);
    //Mat win_gray;
    //cvtColor(window, win_gray, CV_BGR2GRAY);//如果输入是bgr则需要这两行

    if (isDbg)
        imshow("window", window);

    //方法1  canny的两个阈值都比较低
    Mat gauss;
    GaussianBlur(window, gauss, Size(5, 5), 0, 0);
    Mat img_canny;
    Canny(gauss, img_canny, 40, 100, 3, true);

    ////方法2  canny的两个阈值都比较高
    //Mat img_canny;
    //Canny(win_gray, img_canny, 125, 350);
    //threshold(img_canny, img_canny, 128, 255, THRESH_BINARY_INV);
    if (isDbg)
        imshow("img_canny", img_canny);

    vector<vector<Point>> contours;
    findContours(img_canny,
        contours, // a vector of contours
        CV_RETR_EXTERNAL, // retrieve the external contours
        CV_CHAIN_APPROX_NONE); // all pixels of each contours

    //vector<RotatedRect> rects;

    //Start to iterate to each contour founded
    vector<Mat> output;
    for (auto&& ct : contours) {
        //Create bounding rect of object
        //RotatedRect mr = minAreaRect(Mat(*itc));
        Rect re = boundingRect(Mat(ct));
        //rectangle(result, re, Scalar(0, 255, 0));

        if ( //re.x > p.plate_x_min && re.x < p.plate_x_max &&
            re.y > ocrProfile->plate_y_min && re.y < ocrProfile->plate_y_max && re.width > ocrProfile->plate_width_min && re.width < ocrProfile->plate_width_max && re.height > ocrProfile->plate_height_min && re.height < ocrProfile->plate_height_max) {
            Mat hmp(window, re);
            output.push_back(hmp);
            //save hmp
            QString date = QDate::currentDate().toString("yyyyMMdd");
            QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
            QString fullFilePath = QStringLiteral("%1/%2/%3_%4hmp.jpg").arg(ocrDirPath).arg(date).arg(deviceMark).arg(dateTime);
            imwrite(fullFilePath.toStdString(), hmp);
        }
    }
    return output;
}

void OCR::recognize(Mat in)
{
    auto unit_chars = find_ch(binaryProcess(in));

    int count = 0;
    QString nums;
    int i = 0;
    QString date = QDate::currentDate().toString("yyyyMMdd");
    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    for (auto&& unit_char : as_const(unit_chars)) {
        Mat stdChar;
        resize(unit_char.img, stdChar, Size(20, 20));
        // save pattern
        QString fullFilePath = QStringLiteral("%1/%2/%3_%4_%5.jpg").arg(ocrDirPath).arg(date).arg(deviceMark).arg(dateTime).arg(++i);
        imwrite(fullFilePath.toStdString(), stdChar);

        int num = sampleComparison(stdChar);
        if (num != -1)
            nums += QString("%1").arg(num);
    }
    if (nums.size() != 3) {
        return;
    }
    result.push_back(nums.toStdString());
}

cv::Mat OCR::binaryProcess(const Mat& in)
{
    Mat binary;
    threshold(in, binary, 180, 255, cv::THRESH_BINARY);
    return binary;
}

vector<CharSegment> OCR::find_ch(Mat binaryImg)
{
    //Find contours of possibles characters
    vector<vector<Point>> contours;
    findContours(binaryImg,
        contours, // a vector of contours
        CV_RETR_EXTERNAL, // retrieve the external contours
        CV_CHAIN_APPROX_NONE); // all pixels of each contours

    vector<CharSegment> unit_chars;
    for (auto&& ct : contours) { //Create bounding rect of object
        Rect mr = boundingRect(Mat(ct));
        if (mr.width >= ocrProfile->num_width_min && mr.width <= ocrProfile->num_width_max && mr.height >= ocrProfile->num_height_min && mr.height <= ocrProfile->num_height_max) { //筛选有效的框***   mr.width >= 20 && mr.width <= 35 && mr.height >= 20 && mr.height <= 50
            Mat num_cut(binaryImg, mr);
            unit_chars.emplace_back(num_cut, mr);
        }
    }

    // 从左往右对char排序
    sort(unit_chars.begin(), unit_chars.end(), [](auto X, auto Y) { return X.pos.x < Y.pos.x; });
    if (isDbg)
        cout << "find num " << unit_chars.size();
    return unit_chars;
}

int OCR::sampleComparison(Mat charImg)
{
    int minIndex = -1;
    // 如果所有匹配的相似度都未达到最低要求，则返回-1
    double minDis = 2400;
    for (int i = 0; i < 10; i++) {
        double dis = oudistance(pattern[i], charImg);
        // 如果相似度超过阈值，直接接受该匹配
        if (dis < 1000) {
            return i;
        }
        if (dis < minDis) {
            minDis = dis;
            minIndex = i;
        }
    }
    return minIndex;
}

float OCR::oudistance(Mat a, Mat b) const
{
    float distance = 0;
    for (int i = 0; i < a.cols; i++) {
        for (int j = 0; j < a.rows; j++) {
            distance += (a.at<uchar>(i, j) - b.at<uchar>(i, j)) * (a.at<uchar>(i, j) - b.at<uchar>(i, j));
        }
    }
    return sqrt(distance);
}

std::string OCR::get_final_result()
{
    map<string, int> keyMap; //take result value as key and the count as value
    for (auto&& str : result) {
        keyMap[str]++;
    }
    string num = "";
    int maxCount = 0; //遍历keyMap, 找到出现次数最多的num
    for (auto&& pair : keyMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            num = pair.first;
        }
    }

    if (num.empty()) { // no key is detected.
        num = predictAnNum(); // might return a miss
    } else { // a num is detected.
        nContinuousMissCount = 0;
        lastNum = QString::fromStdString(num).toUInt(); // record the num
    }
    ocrDetectCount = result.size();
    result.clear();
    return num;
}

std::string OCR::predictAnNum()
{
    string num;
    // loop
    if (lastNum == 82) {
        num = "001";
    } else {
        num = QString("%1").arg(lastNum + 1, 3, 10, QChar('0')).toStdString();
    }
    lastNum = QString::fromStdString(num).toUInt();

    //if miss for 5 times, not predict
    if (++nContinuousMissCount > 5) {
        num = OCR_MISS;
    }
    return num;
}