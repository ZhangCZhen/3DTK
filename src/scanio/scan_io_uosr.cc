/*
 * scan_io_uosr implementation
 *
 * Copyright (C) Billy Okal
 *
 * Released under the GPL version 3.
 *
 */


/**
 * @file scan_io_uosr.cc
 * @brief IO of a 3D scan in uos file format plus a
 *        reflectance/intensity/temperature value
 * @author Billy Okal. Jacobs University Bremen, Germany.
 */

#include "scanio/scan_io_uosr.h"
#include "scanio/helper.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

#include "slam6d/globals.icc"



#define DATA_PATH_PREFIX "scan"
#define DATA_PATH_SUFFIX ".3d"
#define POSE_PATH_PREFIX "scan"
#define POSE_PATH_SUFFIX ".pose"



std::list<std::string> ScanIO_uosr::readDirectory(const char* dir_path, unsigned int start, unsigned int end)
{
    const char* data_path_suffixes[2] = {DATA_PATH_SUFFIX, NULL};
    return readDirectoryHelper(dir_path, start, end, data_path_suffixes);
}

void ScanIO_uosr::readPose(const char* dir_path, const char* identifier, double* pose)
{
    readPoseHelper(dir_path, identifier, pose);
}

bool ScanIO_uosr::supports(IODataType type)
{
  return !!(type & ( DATA_REFLECTANCE | DATA_XYZ ));
}

std::function<bool (std::istream &data_file)> read_data(PointFilter& filter,
        std::vector<double>* xyz, std::vector<unsigned char>* rgb,
        std::vector<float>* reflectance, std::vector<float>* temperature,
        std::vector<float>* amplitude, std::vector<int>* type,
        std::vector<float>* deviation)
{
    return [=,&filter](std::istream &data_file) -> bool {
        // open data file
        data_file.exceptions(ifstream::eofbit|ifstream::failbit|ifstream::badbit);

        char *firstline;
        std::streamsize linelen;
        linelen = uosHeaderTest(data_file, &firstline);
        if (linelen < 0)
            throw std::runtime_error("unable to read uos header");

        IODataType spec[5] = { DATA_XYZ, DATA_XYZ, DATA_XYZ, DATA_REFLECTANCE, DATA_TERMINATOR };
        ScanDataTransform_identity transform;
        readASCII(data_file, firstline, linelen, spec, transform, filter, xyz, 0, reflectance);

        if (firstline != NULL)
            free(firstline);

        return true;
    };
}

void ScanIO_uosr::readScan(const char* dir_path, const char* identifier, PointFilter& filter, std::vector<double>* xyz, std::vector<unsigned char>* rgb, std::vector<float>* reflectance, std::vector<float>* temperature, std::vector<float>* amplitude, std::vector<int>* type, std::vector<float>* deviation)
{
    if(xyz == 0 || reflectance == 0)
        return;
    // error handling
    path data_path(dir_path);
    data_path /= path(std::string(DATA_PATH_PREFIX) + identifier + DATA_PATH_SUFFIX);
    if (!open_path(data_path, read_data(filter, xyz, rgb, reflectance, temperature, amplitude, type, deviation)))
        throw std::runtime_error(std::string("There is no scan file for [") + identifier + "] in [" + dir_path + "]");
}



/**
 * class factory for object construction
 *
 * @return Pointer to new object
 */
#ifdef _MSC_VER
extern "C" __declspec(dllexport) ScanIO* create()
#else
extern "C" ScanIO* create()
#endif
{
  return new ScanIO_uosr;
}


/**
 * class factory for object construction
 *
 * @return Pointer to new object
 */
#ifdef _MSC_VER
extern "C" __declspec(dllexport) void destroy(ScanIO *sio)
#else
extern "C" void destroy(ScanIO *sio)
#endif
{
  delete sio;
}

#ifdef _MSC_VER
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}
#endif

/* vim: set ts=4 sw=4 et: */
