// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

#include <exception>
#include <fstream>
#include <iostream>
#include "cpx.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "avro/Compiler.hh"
#include "avro/ValidSchema.hh"

using namespace std;

enum SERIALIZE_TYPE {
  MEMORY = 0,
  FILE_OUTPUT
};

enum ENCODE_TYPE {
  BINARY = 0,
  JSON
};

enum ERR_CODE {
  SUCCESS = 0,
  ILLEGAL_SERIALIZE_TYPE,
  ILLEGAL_ENCODE_TYPE,
  FILE_NONEXIST,
  ILLEGAL_JSON
};

// Parse avro schema from filename.
ERR_CODE parseSchema(const char *filename, avro::ValidSchema &schema) {
  std::ifstream in(filename);
  if (!in.good())
    return FILE_NONEXIST;
  try {
    avro::compileJsonSchema(in, schema);
  } catch (const exception &e) {
    return ILLEGAL_JSON;
  }
  return SUCCESS;
}

bool equals(const c::cpx &cp1, const c::cpx &cp2) {
  return (cp1.re == cp2.re && cp1.im == cp2.im);
}

ERR_CODE serialize(SERIALIZE_TYPE sTy, ENCODE_TYPE eTy, const char *schema_file, const char *output) {
  // Construct an instance of cpx.
  c::cpx c1;

  // if the value is 1.0, json encoder will encode it as integer 1.
  // When you decode again, it will throw exception, complaining that integer
  // is encountered while a double is expected.
  c1.re = 1.1;
  c1.im = 2.1;

  std::auto_ptr<avro::OutputStream> out;
  switch (sTy) {
  case MEMORY: out = avro::memoryOutputStream(); break;
  case FILE_OUTPUT: out = avro::fileOutputStream(output /*filename*/, 4096 /*buffer size*/); break;
  default: return ILLEGAL_SERIALIZE_TYPE;
  }

  avro::EncoderPtr e;
  avro::ValidSchema validSchema;
  switch (eTy) {
  case BINARY: e = avro::binaryEncoder(); break;
  case JSON: {
    ERR_CODE errCode = parseSchema(schema_file, validSchema);
    if (parseSchema(schema_file, validSchema) != SUCCESS)
      return errCode;
    e = avro::jsonEncoder(validSchema);
  }
  break;
  default: return ILLEGAL_ENCODE_TYPE;
  }

  // Do the encoding here.
  e->init(*out);
  avro::encode(*e, c1);

  // flush the file to disk.
  if (sTy == FILE_OUTPUT) out->flush();

  // Now, we try to decode the encoded objects.
  std::auto_ptr<avro::InputStream> in;
  switch(sTy) {
  case MEMORY: in = avro::memoryInputStream(*out); break;
  case FILE_OUTPUT: in = avro::fileInputStream(output, 4096); break;
  default: return ILLEGAL_SERIALIZE_TYPE;
  }
  avro::DecoderPtr d;
  switch(eTy) {
  case BINARY: d = avro::binaryDecoder(); break;
  case JSON: d = avro::jsonDecoder(validSchema); break;
  default: return ILLEGAL_ENCODE_TYPE;
  }

  d->init(*in);

  c::cpx c2;
  avro::decode(*d, c2);

  // Verify the values are equal.
  assert (equals(c1, c2));
  return SUCCESS;
}


int main() {
  if (/*serialize(MEMORY, BINARY, "cpx.json", "cpx.binmem") != SUCCESS ||
      serialize(MEMORY, JSON, "cpx.json", "cpx.jsonmem") != SUCCESS ||
      serialize(FILE_OUTPUT, BINARY, "cpx.json", "cpx.binfile") != SUCCESS ||*/
      serialize(FILE_OUTPUT, JSON,   "cpx.json", "cpx.jsonfile") != SUCCESS
      )
    std::cerr << "Error\n";
  return 0;
}

/*
static byte[] fromJasonToAvro(String json, String schemastr) throws Exception {

InputStream input = new ByteArrayInputStream(json.getBytes());
DataInputStream din = new DataInputStream(input);

Schema schema = Schema.parse(schemastr);

Decoder decoder = DecoderFactory.get().jsonDecoder(schema, din);

DatumReader<Object> reader = new GenericDatumReader<Object>(schema);
Object datum = reader.read(null, decoder);

GenericDatumWriter<Object> w = new GenericDatumWriter<Object>(schema);
ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

Encoder e = EncoderFactory.get().binaryEncoder(outputStream, null);

w.write(datum, e);
e.flush();

return outputStream.toByteArray();
}

*/
