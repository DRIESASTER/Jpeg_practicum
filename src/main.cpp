#include <cstdio>
#include <cstdlib>
#include <string>

#include "psnr.hpp"
#include "ctpl_stl.h"  // ThreadPool
#include "jpeg_decoder.hpp"
#include "jpeg_encoder.hpp"
#include <iostream>
#include <fstream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>


struct ppm {
    int width;
    int height;
    int max_color;
    std::string comment;
    std::vector<unsigned char> data;
};


ppm ppmRead(std::string file) {
    ppm image{};
    std::ifstream fileStream(file, std::ios::binary);
    if (!fileStream) {
        exit(1);
    }
    std::string line;
    getline(fileStream, line);
    if (line != "P6") {
        exit(1);
    }
    if (fileStream.peek() == '#') {
        getline(fileStream, image.comment);
    }
    fileStream >> image.width >> image.height >> image.max_color;
    fileStream.ignore(256, '\n');
    image.data.resize(image.width * image.height * 3);
    fileStream.read(reinterpret_cast<char *>(image.data.data()), image.data.size());
    fileStream.close();
    return image;
}


struct compression_task {
    // Task description
    std::string input_file;
    int quality{0};
    bool chroma_subsampling{false};

    bool success{false};
    std::string output_file; ///< JPEG file
    uint64_t uncompressed_filesize{0};
    uint64_t jpeg_filesize{0};
    double psnr{0.0}; ///< PSNR in dB
    double rate{0.0}; ///< Bits per pixels (bpp)
};


void compress(compression_task *task) {
    const ppm &ppmImage = ppmRead(task->input_file);
    task->uncompressed_filesize = ppmImage.data.size();
    std::vector<unsigned char> endcodedFile;
    auto out = [&](uint8_t byte) { endcodedFile.push_back(byte);};
    JpegEncoder::writeJpeg(
            out,
            ppmImage.data.data(), ppmImage.width, ppmImage.height, true, task->quality,
            task->chroma_subsampling);

    JpegDecoder::Decoder decoder(&endcodedFile[0], endcodedFile.size());
    if (decoder.GetResult() == JpegDecoder::Decoder::OK) {
        double psnr = PSNR::PSNR(ppmImage.data.data(), decoder.GetImage(), decoder.GetImageSize());
        task->psnr = psnr;
        task->jpeg_filesize = endcodedFile.size();
        task->rate = (task->jpeg_filesize * 8.0) / (ppmImage.width * ppmImage.height);
    }
    task->success = true;
}



void opgave2() {
    ctpl::thread_pool pool(std::thread::hardware_concurrency());
    std::list<std::string> files = {"nightshot_iso_1600, nightshot_iso_100"};
    std::vector<std::tuple<std::string, int, bool, double, double>> rd_data;
    for (std::string& file : files) {
        for (int i = 100; i >= 5; i -= 5) {
            pool.push([file, i, &rd_data](int thread_id) {
                compression_task task;
                task.chroma_subsampling = true;
                task.input_file = "/Users/drieshuybens/Documents/JPEG/JpegEncoder/afbeeldingen/" + file + ".ppm";
                task.output_file = file + std::to_string(i) + "downsampled" + ".jpeg";
                task.quality = i;
                compress(&task);
                rd_data.push_back(std::make_tuple(file, i, true, task.rate, task.psnr));
            });
            pool.push([file, i, &rd_data](int thread_id) {
                compression_task task2;
                task2.chroma_subsampling = false;
                task2.input_file = "/Users/drieshuybens/Documents/JPEG/JpegEncoder/afbeeldingen/" + file + ".ppm";
                task2.output_file = file + std::to_string(i) + ".jpeg";
                task2.quality = i;
                compress(&task2);
                rd_data.push_back(std::make_tuple(file, i, false, task2.rate, task2.psnr));
            });
        }
    }
    pool.stop(true);
    FILE* graphFile = std::fopen("rd_data.csv", "w");
    for (const auto& point : rd_data) {
        std::fprintf(graphFile, "%s,%d,%d,%.4f,%.4f\n", std::get<0>(point).c_str(), std::get<1>(point), std::get<2>(point), std::get<3>(point), std::get<4>(point));
    }
    std::fclose(graphFile);
}



void opgave3() {
}

int main() {
    opgave2();

    return 0;
}

