///=========================================================///
/*
 * An Example to demonstrate how to generate json string using
 * the avro jsonEncoder.
 */
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <avro/Compiler.hh>
#include <avro/Encoder.hh>
#include <avro/Decoder.hh>
#include <avro/Stream.hh>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>

#include "cpx.hh"
using namespace std;

void testStreamWriter() {
  const static size_t CHUNK_SIZE = 8 * 1024;
  std::ifstream ifs("cpx.json");

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream(CHUNK_SIZE);
  avro::EncoderPtr e = avro::jsonEncoder(cpxSchema);

  e->init(*out);
  c::cpx c1;
  c1.re = 100.23;
  c1.im = 105.77;
  avro::encode(*e, c1);
  // Note that flush should be called on the encoder, not the output stream.
  // out->flush();
  e->flush();
  std::cout << "out->byteCount(): " << out->byteCount() << std::endl;

  // Try to use StreamWriter and see whether it works.
  avro::StreamWriter sw(*out);
  sw.flush();
  uint8_t json[CHUNK_SIZE];
  sw.writeBytes(json, CHUNK_SIZE);
  std::cout << "json: " << json << "\n";

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::StreamReader streamReader(*in);
  uint8_t jsonBytes[CHUNK_SIZE];
  size_t length = CHUNK_SIZE;
  try {
    // readBytes can throw exceptions.
    streamReader.readBytes(jsonBytes, length);
  } catch (avro::Exception &e) {
    // Handle the exception here.
  }
  std::cout << "in->byteCount(): " << in->byteCount() << std::endl;

  std::cout << "result: '" << jsonBytes << "'" << std::endl;
  for (size_t i = 0; i < 30; ++i) {
    printf("jsonBytes[%02zu]: 0x%02X (%c)\n", i, (unsigned char)jsonBytes[i], jsonBytes[i]);
  }
}

void fromAvroToJsonString() {
  const static size_t CHUNK_SIZE = 4 * 1024;
  std::ifstream ifs("cpx.json");

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream(CHUNK_SIZE);
  avro::EncoderPtr e = avro::jsonEncoder(cpxSchema);

  e->init(*out);
  c::cpx c1;
  c1.re = 100.23, c1.im = 105.77;
  avro::encode(*e, c1);
  // Note that flush should be called on the encoder, not the output stream.
  // out->flush();
  e->flush();
  std::cout << "out->byteCount(): " << out->byteCount() << std::endl;

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::StreamReader streamReader(*in);
  std::cout << out->byteCount() << "\n";
  uint8_t jsonBytes[CHUNK_SIZE];
  try {
    // readBytes can throw exceptions.
    streamReader.readBytes(&jsonBytes[0], CHUNK_SIZE);
  } catch (avro::Exception &e) {
    // Handle the exception here.
  }
  std::cout << "in->byteCount(): " << in->byteCount() << std::endl;

  std::cout << "result: '" << jsonBytes << "'" << std::endl;
  for (size_t i = 0; i < 30; ++i) {
    printf("jsonBytes[%02zu]: 0x%02X (%c)\n", i, (unsigned char)jsonBytes[i], jsonBytes[i]);
  }
}

void fromAvroToJsonString2() {
  std::ifstream ifs("cpx.json");

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::jsonEncoder(cpxSchema);

  e->init(*out);
  c::cpx c1;
  c1.re = 100.23, c1.im = 105.77;
  avro::encode(*e, c1);
  // Note that flush should be called on the encoder, not the output stream.
  // out->flush();
  e->flush();
  std::cout << "out->byteCount(): " << out->byteCount() << std::endl;

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);
  avro::StreamReader streamReader(*in);
  int count = out->byteCount();
  vector<uint8_t> jsonBytes(count);
  // readBytes can throw exceptions.
  try {
    streamReader.readBytes(&jsonBytes[0], jsonBytes.size());
  } catch (avro::Exception &e) {
    // Handle the exception here.
  }
  std::cout << "in->byteCount(): " << in->byteCount() << std::endl;

  for (size_t i = 0; i < jsonBytes.size(); ++i) {
    std::cout << jsonBytes[i];
  }
  std::cout << "\n";
  for (size_t i = 0; i < out->byteCount(); ++i) {
    printf("jsonBytes[%02zu]: 0x%02X (%c)\n", i, (unsigned char)jsonBytes[i], jsonBytes[i]);
  }
}

// Given a json string and avro schema, generate the avro object.
// For java version: http://stackoverflow.com/questions/27559543/json-string-to-java-object-avro
void fromJsonToAvro(string json, string schema) {
  std::auto_ptr<avro::InputStream> in =
    avro::memoryInputStream((const uint8_t*)&json[0], json.size());

  // Compile the schema string.
  avro::ValidSchema validSchema;
  std::istringstream ins(schema);

  try {
    avro::compileJsonSchema(ins, validSchema);
  } catch (const exception &e) {
    return;
  }

  avro::DecoderPtr d = avro::jsonDecoder(validSchema);
  avro::GenericDatum datum(validSchema);

  d->init(*in);
  avro::decode(*d, datum);

  // Now encrypt the data into avro.
  std::auto_ptr<avro::OutputStream> out =
    avro::fileOutputStream("json_encode_example.out", 4096);
  avro::EncoderPtr e = avro::jsonEncoder(validSchema);
  e->init(*out);
  avro::encode(*e, datum);
  out->flush();
}

int main() {
  // testStreamWriter();
  fromAvroToJsonString2();

  // Test the function to generate avro objects from json string.
  string json = "{\"username\":\"miguno\",\"tweet\":\"Rock: Nerf paper, scissors is fine.\",\"timestamp\": 1366150681 }";
  string schema ="{ \"type\" : \"record\", \"name\" : \"twitter_schema\", \"namespace\" : \"com.miguno.avro\", \"fields\" : [ { \"name\" : \"username\", \"type\" : \"string\", \"doc\"  : \"Name of the user account on Twitter.com\" }, { \"name\" : \"tweet\", \"type\" : \"string\", \"doc\"  : \"The content of the user's Twitter message\" }, { \"name\" : \"timestamp\", \"type\" : \"long\", \"doc\"  : \"Unix epoch time in seconds\" } ], \"doc:\" : \"A basic schema for storing Twitter messages\" }";
  fromJsonToAvro(json, schema);
}
