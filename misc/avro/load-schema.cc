// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

// A simple example to demonstrate how to load avro schema.
#include <exception>
#include <fstream>
#include "avro/ValidSchema.hh"
#include "avro/Compiler.hh"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: load-schema filename\n";
  }
  std::ifstream in(argv[1]);
  if (!in.good()) {
    std::cerr << "Could not open file " << argv[1];
    return 1;
  }

  avro::ValidSchema cpxSchema;

  try {
    avro::compileJsonSchema(in, cpxSchema);
  } catch (const exception &e) {
    std::cerr << "Failed to parse the schema. Validate the json first.\n";
    return 1;
  }
  std::cout << "Valid schema.\n";
}
