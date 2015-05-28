// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

// Test the custom encoding and decoding.

#include <complex>
#include <fstream>

#include "cpx.hh"
#include "imaginary.hh"
#include "avro/Compiler.hh"
#include "avro/DataFile.hh"
#include "avro/Decoder.hh"
#include "avro/Encoder.hh"
#include "avro/Generic.hh"
#include "avro/Specific.hh"

// custom encoding and decoding for std::complex type.
namespace avro {
  template<typename T>
  struct codec_traits<std::complex<T> > {
    static void encode(Encoder &e, const std::complex<T> &c) {
      avro::encode(e, std::real(c));
      avro::encode(e, std::imag(c));
    }

    static void decode(Decoder &d, std::complex<T> &c) {
      T re, im;
      avro::decode(d, re);
      avro::decode(d, im);
      c = std::complex<T>(re, im);
    }
  };
}


int encode_test() {
  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::binaryEncoder();
  e->init(*out);
  std::complex<double> c1(1.0, 2.0);
  avro::encode(*e, c1);

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::DecoderPtr d = avro::binaryDecoder();
  d->init(*in);

  std::complex<double> c2;
  avro::decode(*d, c2);
  std::cout << '(' << std::real(c2) << ", " << std::imag(c2) << ')' << std::endl;
  return 0;
}

int validating_encode(const char *schema_file) {
  std::ifstream ifs(schema_file);
  if (!ifs.good())
    return -1;

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::validatingEncoder(cpxSchema, avro::binaryEncoder());
  e->init(*out);

  std::complex<double> c1(1.0, 2.0);
  avro::encode(*e, c1);

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::DecoderPtr d = avro::validatingDecoder(cpxSchema, avro::binaryDecoder());
  d->init(*in);

  std::complex<double> c2;
  avro::decode(*d, c2);
  std::cout << '(' << std::real(c2) << ", " << std::imag(c2) << ')' << std::endl;
  return 0;
}

// Try to decode a file with generic datum.
int generic_data_decode(const char *schema_file) {
  std::ifstream ifs(schema_file);

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::binaryEncoder();
  e->init(*out);
  c::cpx c1;
  c1.re = 100.23;
  c1.im = 105.77;
  avro::encode(*e, c1);

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::DecoderPtr d = avro::binaryDecoder();
  d->init(*in);

  avro::GenericDatum datum(cpxSchema);
  avro::decode(*d, datum);
  std::cout << "Type: " << datum.type() << std::endl;
  if (datum.type() == avro::AVRO_RECORD) {
    const avro::GenericRecord &r = datum.value<avro::GenericRecord>();
    std::cout << "Field-count: " << r.fieldCount() << std::endl;
    if (r.fieldCount() == 2) {
      const avro::GenericDatum &f0 = r.fieldAt(0);
      if (f0.type() == avro::AVRO_DOUBLE)
        std::cout << "Real: " << f0.value<double>() << std::endl;
      const avro::GenericDatum &f1 = r.fieldAt(1);
      if (f1.type() == avro::AVRO_DOUBLE)
        std::cout << "Imaginary: " << f1.value<double>() << std::endl;
    }
  }
  return 0;
}

avro::ValidSchema load(const char *filename) {
  std::ifstream ifs(filename);
  avro::ValidSchema result;
  avro::compileJsonSchema(ifs, result);
  return result;
}

// It is possible to read data written with a different schema.
int resolving_different_schema(const char *write_schema, const char *read_schema) {
  avro::ValidSchema cpxSchema = load(write_schema);
  avro::ValidSchema imaginarySchema = load(read_schema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::binaryEncoder();
  e->init(*out);
  c::cpx c1;
  c1.re = 100.23;
  c1.im = 105.77;
  avro::encode(*e, c1);

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::DecoderPtr d = avro::resolvingDecoder(cpxSchema, imaginarySchema, avro::binaryDecoder());
  d->init(*in);

  i::cpx c2;
  avro::decode(*d, c2);
  std::cout << "Imaginary: " << c2.im << std::endl;
  return 0;
}

int data_file_test(const char *schema_file, const char *output) {
  avro::ValidSchema cpxSchema = load(schema_file);
  {
    avro::DataFileWriter<c::cpx> dfw(output, cpxSchema);
    c::cpx c1;
    for (int i = 0; i < 100; ++i) {
      c1.re = i * 100;
      c1.im = i + 100;
      dfw.write(c1);
    }
    dfw.close();
  }

  {
    avro::DataFileReader<c::cpx> dfr(output, cpxSchema);
    c::cpx c2;
    while (dfr.read(c2)) {
      std::cout << '(' << c2.re << ", " << c2.im << ')' << std::endl;
    }
  }
  return 0;
}

int main() {
  // encode_test();
  // validating_encode("cpx.json");
  // generic_data_decode("cpx.json");
  resolving_different_schema("cpx.json", "imaginary.json");
  data_file_test("cpx.json", "test.bin");
}
