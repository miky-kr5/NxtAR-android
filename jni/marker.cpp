/*
 * Copyright (C) 2014 Miguel Angel Astor Romero
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <algorithm>
#include <utility>
#include <limits>
#include <iostream>

#include "marker.hpp"

#define MIN_CONTOUR_LENGTH 0.1

namespace nxtar{

    static const cv::Scalar COLOR = cv::Scalar(255, 255, 255);

    class Marker;

    typedef std::vector<std::vector<cv::Point> > contours_vector;
    typedef std::vector<cv::Point2f> points_vector;
    typedef std::vector<Marker> markers_vector;

    class Marker{
        public:
            ~Marker();
            points_vector points;
            int code;
    };

    float perimeter(points_vector &);
    int hammDistMarker(cv::Mat);
    cv::Mat rotate(cv::Mat);
    void binarize(cv::Mat &, cv::Mat &);
    void findContours(cv::Mat &, contours_vector &, int);
    void renderContours(contours_vector &, cv::Mat &);
    void renderMarkers(markers_vector &, cv::Mat &);
    void warpMarker(Marker &, cv::Mat &, cv::Mat &);
    int decodeMarker(cv::Mat &);
    void fixCorners(cv::Mat &, Marker &);
    void isolateMarkers(const contours_vector &, markers_vector &);

    void getAllMarkers(std::vector<int> & codes, cv::Mat & img){
        cv::Mat gray, thresh, cont, mark;
        contours_vector contours;
        markers_vector markers;
        markers_vector valid_markers;

        codes.clear();

        cv::cvtColor(img, gray, CV_BGR2GRAY);
        binarize(gray, thresh);
        findContours(thresh, contours, 40);
        isolateMarkers(contours, markers);

        for(int i = 0; i < markers.size(); i++){
            warpMarker(markers[i], gray, mark);

            int code = decodeMarker(mark);

            if(code != -1){
                markers[i].code = code;
                valid_markers.push_back(markers[i]);
            }
        }

        for(int i = 0; i < valid_markers.size(); i++)
            fixCorners(gray, valid_markers[i]);

        cont = cv::Mat::zeros(img.size(), CV_8UC3);
        renderMarkers(valid_markers, cont);

        img = img + cont;

        for(int i = 0; i < valid_markers.size(); i++){
            codes.push_back(valid_markers[i].code);
        }

        markers.clear();
        contours.clear();
        valid_markers.clear();
    }

    #ifdef DESKTOP
    void getAllMarkers_d(std::vector<int> & codes, cv::Mat & img){
        cv::Mat gray, thresh, cont, mark;
        contours_vector contours;
        markers_vector markers;
        markers_vector valid_markers;
        std::ostringstream oss;

        codes.clear();

        cv::cvtColor(img, gray, CV_BGR2GRAY);
        binarize(gray, thresh);
        findContours(thresh, contours, 40);
        isolateMarkers(contours, markers);

        for(int i = 0; i < markers.size(); i++){
            warpMarker(markers[i], gray, mark);

            int code = decodeMarker(mark);

            if(code != -1){
                markers[i].code = code;
                valid_markers.push_back(markers[i]);
            }
        }

        for(int i = 0; i < valid_markers.size(); i++)
            fixCorners(gray, valid_markers[i]);

        cont = cv::Mat::zeros(img.size(), CV_8UC3);
        renderMarkers(valid_markers, cont);

        img = img + cont;

        for(int i = 0; i < valid_markers.size(); i++){
            oss << valid_markers[i].code;

            cv::putText(mark, oss.str(), cv::Point(5, 250), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar::all(128), 3, 8);

            oss.str("");
            oss.clear();

            oss << "Marker[" << i << "]";

            cv::imshow(oss.str(), mark);

            oss.str("");
            oss.clear();

            codes.push_back(valid_markers[i].code);
        }

        markers.clear();
        contours.clear();
        valid_markers.clear();
    }
    #endif

    void binarize(cv::Mat & src, cv::Mat & dst){
        cv::adaptiveThreshold(src, dst, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 7, 7);
    }

    void findContours(cv::Mat & img, contours_vector & v, int minP){
        std::vector<std::vector<cv::Point> > c;
        cv::findContours(img, c, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

        v.clear();
        for(size_t i = 0; i < c.size(); i++){
            if(c[i].size() > minP){
                v.push_back(c[i]);
            }
        }
    }

    void renderContours(contours_vector & v, cv::Mat & dst){
        cv::drawContours(dst, v, -1, COLOR, 1, CV_AA);
    }

    void renderMarkers(markers_vector & v, cv::Mat & dst){
        contours_vector cv;

        for(size_t i = 0; i < v.size(); i++){
            std::vector<cv::Point> pv;
            for(size_t j = 0; j < v[i].points.size(); ++j)
                pv.push_back(cv::Point2f(v[i].points[j].x, v[i].points[j].y));
            cv.push_back(pv);
        }

        cv::drawContours(dst, cv, -1, COLOR, 1, CV_AA);
    }

    void isolateMarkers(const contours_vector & vc, markers_vector & vm){
        std::vector<cv::Point> appCurve;
        markers_vector posMarkers;

        for(size_t i = 0; i < vc.size(); ++i){
            double eps = vc[i].size() * 0.05;
            cv::approxPolyDP(vc[i], appCurve, eps, true);

            if(appCurve.size() != 4 || !cv::isContourConvex(appCurve)) continue;

            float minD = std::numeric_limits<float>::max();

            for(int i = 0; i < 4; i++){
                cv::Point side = appCurve[i] - appCurve[(i + 1) % 4];
                float sqSideLen = side.dot(side);
                minD = std::min(minD, sqSideLen);
            }

            if(minD < MIN_CONTOUR_LENGTH) continue;

            Marker m;

            for(int i = 0; i < 4; i++)
                m.points.push_back(cv::Point2f(appCurve[i].x, appCurve[i].y));

            cv::Point v1 = m.points[1] - m.points[0];
            cv::Point v2 = m.points[2] - m.points[0];

            double o = (v1.x * v2.y) - (v1.y * v2.x);
            if(o < 0.0) std::swap(m.points[1], m.points[3]);

            posMarkers.push_back(m);
        }

        std::vector<std::pair<int, int> > tooNear;
        for(size_t i = 0; i < posMarkers.size(); ++i){
            const Marker & m1 = posMarkers[i];

            for(size_t j = i + 1; j < posMarkers.size(); j++){
                const Marker & m2 = posMarkers[j];

                float dSq = 0.0f;

                for(int c = 0; c < 4; c++){
                    cv::Point v = m1.points[c] - m2.points[c];
                    dSq += v.dot(v);
                }

                dSq /= 4.0f;

                if(dSq < 100) tooNear.push_back(std::pair<int, int>(i, j));
            }
        }

        std::vector<bool> remMask(posMarkers.size(), false);

        for(size_t i = 0; i < tooNear.size(); ++i){
            float p1 = perimeter(posMarkers[tooNear[i].first].points);
            float p2 = perimeter(posMarkers[tooNear[i].second].points);

            size_t remInd;
            if(p1 > p2) remInd = tooNear[i].second;
            else remInd = tooNear[i].first;

            remMask[remInd] = true;
        }

        vm.clear();
        for(size_t i = 0; i < posMarkers.size(); ++i){
            if(remMask[i]) vm.push_back(posMarkers[i]);
        }
    }

    void warpMarker(Marker & m, cv::Mat & in, cv::Mat & out){
        cv::Mat bin;
        cv::Size markerSize(350, 350);
        points_vector v;
        v.push_back(cv::Point2f(0,0));
        v.push_back(cv::Point2f(markerSize.width-1,0));
        v.push_back(cv::Point2f(markerSize.width-1,markerSize.height-1));
        v.push_back(cv::Point2f(0,markerSize.height-1));

        cv::Mat M = cv::getPerspectiveTransform(m.points, v);
        cv::warpPerspective(in, bin, M, markerSize);

        cv::threshold(bin, out, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    }

    int hammDistMarker(cv::Mat bits){
        int ids[4][5] = {
            {1,0,0,0,0},
            {1,0,1,1,1},
            {0,1,0,0,1},
            {0,1,1,1,0}
        };

        int dist = 0;

        for (int y = 0; y < 5; y++){
            int minSum = 1e5;

            for (int p = 0; p < 4; p++){
                int sum = 0;

                for (int x = 0; x < 5; x++){
                    sum += bits.at<uchar>(y, x) == ids[p][x] ? 0 : 1;
                }

                if(minSum > sum)
                    minSum = sum;
            }

            dist += minSum;
        }

        return dist;
    }
    cv::Mat rotate(cv::Mat in){
        cv::Mat out;
        in.copyTo(out);
        for (int i=0;i<in.rows;i++){
            for (int j=0;j<in.cols;j++){
                out.at<uchar>(i,j)=in.at<uchar>(in.cols-j-1,i);
            }
        }

        return out;
    }

    int decodeMarker(cv::Mat & marker){
        bool found = false;
        int code = 0;
        cv::Mat bits;

        for(int y = 0; y < 7; y++){
            int inc = (y == 0 || y == 6) ? 1 : 6;

            for(int x = 0; x < 7; x += inc){
                int cX = x * 50;
                int cY = y * 50;

                cv::Mat cell = marker(cv::Rect(cX, cY, 50, 50));

                int nZ = cv::countNonZero(cell);

                // Not a valid marker.
                if(nZ > (50 * 50) / 2) return -1;
            }
        }

        bits = cv::Mat::zeros(5, 5, CV_8UC1);


        for(int y = 0; y < 5; y++){
            for(int x = 0; x < 5; x++){
                int cX = (x + 1) * 50;
                int cY = (y + 1) * 50;

                cv::Mat cell = marker(cv::Rect(cX, cY, 50, 50));

                int nZ = cv::countNonZero(cell);

                if(nZ > (50 * 50) / 2) bits.at<uchar>(y, x) = 1;
            }
        }

        if(hammDistMarker(bits) != 0){
            for(int r = 1; r < 4; r++){
                bits = rotate(bits);
                if(hammDistMarker(bits) != 0) continue;
                else{ found = true; break;}
            }
        }else found = true;


        if(found){
            for(int y = 0; y < 5; y++){
                code <<= 1;
                if(bits.at<uchar>(y, 1))
                    code |= 1;

                code <<= 1;
                if(bits.at<uchar>(y, 3))
                    code |= 1;
            }


            return code;
        }else
            return -1;
    }

    void fixCorners(cv::Mat & img, Marker & m){
        cv::cornerSubPix(img, m.points, cvSize(10, 10), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER, 30, 0.1));
    }

    float perimeter(points_vector & p){
        float per = 0.0f, dx, dy;

        for(size_t i; i < p.size(); ++i){
            dx = p[i].x - p[(i + 1) % p.size()].x;
            dy = p[i].y - p[(i + 1) % p.size()].y;
            per += sqrt((dx * dx) + (dy * dy));
        }

        return per;
    }

    Marker::~Marker(){
        points.clear();
    }

}
