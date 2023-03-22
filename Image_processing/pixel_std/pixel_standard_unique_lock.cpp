#include <iostream>
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>
#include <chrono>

using namespace cv;

void processImage(const Mat& inputImage, Mat& outputImage, int startRow, int endRow, std::mutex& outputImageMutex)
{
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < inputImage.cols; ++j) {
            Vec3b pixel = inputImage.at<Vec3b>(i, j);
            pixel[0] *= 3; // double blue channel
            pixel[1] *= 3; // double green channel
            pixel[2] *= 3; // double red channel

            std::unique_lock<std::mutex> lock(outputImageMutex);
            outputImage.at<Vec3b>(i, j) = pixel;
        }
    }
}

int main(int argc, char** argv)
{
    // Read input image
    Mat inputImage = imread("/Users/stephencao/Downloads/1.png", IMREAD_COLOR);
    //cv::resize(inputImage, inputImage, cv::Size(10000, 10000));
    // Create output image
    Mat outputImage = Mat::zeros(inputImage.size(), inputImage.type());

    // Partition the image into 4 regions
    int numRegions = 4;
    int rowsPerRegion = inputImage.rows / numRegions;

    // Create threads to process each region
    std::vector<std::thread> threads(numRegions);
    std::mutex outputImageMutex;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numRegions; ++i) {
        int startRow = i * rowsPerRegion;
        int endRow = (i + 1) * rowsPerRegion;
        if (i == numRegions - 1) {
            endRow = inputImage.rows;
        }
        threads[i] = std::thread(processImage, std::ref(inputImage), std::ref(outputImage), startRow, endRow, std::ref(outputImageMutex));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Display output image
    imshow("Output", outputImage);
    waitKey(0);

    std::cout << "Execution time: " << duration.count() << " ms" << std::endl;

    return 0;
}