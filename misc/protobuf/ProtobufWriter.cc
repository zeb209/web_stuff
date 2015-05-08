// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

// Test VersionedData here.

#include <trafficserver-protobuf/VersionedData.pb.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

// Write the VersionedData into a file.
bool writeToFile(const VersionedData &VD, const string &filename) {
  ofstream os(filename.c_str());
  if (!os.good())
    return false;
  if (!VD.SerializeToOstream(&os))
    return false;
  os.close();
  return true;
}

// Read a file into a string.
bool readFileIntoStr(const string &filename, string &contents) {
  ifstream input(filename.c_str());
  if (!input.good()) return false;
  // Get length of a file.
  input.seekg(0, input.end);
  size_t fileSize = input.tellg();
  input.seekg(0, input.beg);

  contents.resize(fileSize);
  input.read(&contents[0], fileSize);
  input.close();
  return true;
}

// Print the VersionedData in file filename,
bool printVersionedData(const string &filename) {
  string contents;
  if (!readFileIntoStr(filename, contents)) {
    std::cerr << "Failed to read the file " << filename << "\n";
    return false;
  }
  VersionedData downloadedData;
  if (!downloadedData.ParseFromString(contents))
    return false;
  std::cout << downloadedData.data();
  return true;
}

// Construct a VersionedData with a str as the data.
bool constructVersionedData(const string &str, int version, VersionedData &VD) {
  VD.set_data(str);
  VD.set_version(version);
  return true;
}

int main(int argc, char *argv[]) {
  if (argc == 6) {
    if (string(argv[2]).compare("-v") != 0 || string(argv[4]).compare("-o") != 0) {
      std::cerr << "Usage: ProtobufWriter input -v version -o output\n";
      return 1;
    }
  } else if (argc == 3) {
    if (string(argv[1]).compare("-p") != 0) {
      std::cerr << "Usage: ProtobufWriter -p versioned_data\n";
      return 1;
    } else {
      if (!printVersionedData(argv[2])) {
        std::cerr << "Failed to parse file " << argv[2] << "\n";
        return 1;
      }
      return 0;
    }
  } else {
    std::cerr << "Write a file in protobuf format: ProtobufWriter input -v version -o output\n"
              << "Parse a protobuf file: ProtobufWriter -p filename\n";
    return 1;
  }

  int version = atoi(argv[3]);
  if (version < 0) {
    std::cerr << "Usage: ProtobufWriter input -v version -o output\n";
    return 1;
  }

  // Read the input file into a string.
  string input;
  if (!readFileIntoStr(argv[1], input)) {
    std::cerr << "Failed to read the input file: " << argv[1] << "\n";
    return 1;
  }

  // Construct VersionedData from the string.
  VersionedData VD;
  if (!constructVersionedData(input, version, VD)) {
    std::cerr << "Failed to construct a VersionedData\n";
    return 1;
  }

  // Write the VersionedData to a file.
  if (!writeToFile(VD, argv[5])) {
    std::cerr << "Failed to write VersionedData to file " << argv[5] << "\n";
    return 1;
  }
  return 0;
}
