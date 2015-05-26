// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

// This is a simple program that I wrote to produce kafka events.
// It requires the atscppapi.

#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/PluginInit.h> // Needed so that TSPluginInit is not mangled.
#include <atscppapi/TransactionPlugin.h>
#include <atscppapi/Logger.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <li_atscppapi/StatusProviderService.h>
#include <string>

#include "li_kafka.h"
#include "KafkaProducer.h"
#include "KafkaTopicSchema.h"
#include "RequestEvent.h"

#define LOG_TAG "requesteventproducer"

namespace {
  atscppapi::Logger logger;
  KafkaProducer *kafka_rest = NULL;

  void kafka_logger(li_kafka_log_msg_level log_level, const char *err_msg) {
    switch(log_level) {
    case LI_KAFKA_LOG_MSG_ERROR:
      std::cerr << "Error: " << err_msg << std::endl;
      break;
    case LI_KAFKA_LOG_MSG_INFO:
      std::cerr << "Log: " << err_msg << std::endl;
      break;
    default:
      std::cout << "[kafka] " << err_msg << std::endl;
      break;
    }
  }

  // Register a kafka schema and get the schema_id.
  KafkaProducer *createKafkaProducer(const std::string &proxyUrl /* kafka rest proxy url */,
                                     const std::string &schemaRegistryUrl /* kafka schema registry url*/,
                                     void (*klogger)(li_kafka_log_msg_level, const char *),
                                     const std::string &topic /* kafka event topic */,
                                     const std::string &schema /* the schema for the event */,
                                     const std::string &logfile,
                                     const unsigned buffer_size /* buffer queue size */) {
    // In order to create a kafka producer, you need to create a kafka object.
    li_kafka lik = li_kafka_init_rest(proxyUrl.c_str(), schemaRegistryUrl.c_str(),
                                      klogger, LI_KAFKA_LOG_LEVEL_INFO);
    if (lik == NULL) {
      std::cerr << "Failed to initialize kafka\n";
      return NULL;
    }

    // Register the schema with the topic and schema.
    if (!li_kafka_register_schema(lik, topic.c_str(), schema.c_str())) {
      std::cerr << "Failed to register schema";
      return NULL;
    }

    // Get the schema ID. Try 10 times to see whether it still fails.
    char schema_id[256];
    int count = 10;
    while (count-- > 0) {
      if (li_kafka_get_schema_id(lik, topic.c_str(), schema_id, sizeof(schema_id)))
        break;
      usleep(100000/* micro seconds*/);
    }

    if (!schema_id[0]) {
      std::cerr << "Failed to get schema ID\n";
      return NULL;
    }
    return new KafkaProducer(proxyUrl, logfile, topic, schema_id, buffer_size);
  }
}

// Read a file into a string.
bool readFileIntoStr(const std::string &filename, std::string &contents) {
  std::ifstream input(filename.c_str());
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

class TxnProducer : public atscppapi::TransactionPlugin {
public:
  TxnProducer(atscppapi::Transaction &txn) : TransactionPlugin(txn), transaction(txn) {
    // event_str += "TxnProducer constructor";
  }

  ~TxnProducer() {
    // This is not in the right schema. Receive 400 error.
    // event_str += "TxnProducer destructor";

    // This is a legal event string.
    event_str = "{\"header\":null,\"request\":null,\"response\":null}";

    // Read an event string from a file that contains the events in json.
    // if (!readFileIntoStr(event_file, event_str)) {
    //   std::cerr << "Failed to read the file: " << event_file << "\n";
    //   event_str = EVENT_STR;
    // }
    kafka_rest->sendEvent(event_str);
  }
private:
  atscppapi::Transaction &transaction;
  std::string event_str;
};

// A global plugin.
class RequestEventProducer : public atscppapi::GlobalPlugin, atscppapi::StatusProvider {
public:
  RequestEventProducer() : atscppapi::GlobalPlugin(true /*ignore internal transactions */) {}
  bool initialize();
  virtual std::string getStatus() { return "GOOD STATUS."; }
  virtual void handleReadRequestHeadersPreRemap(atscppapi::Transaction &);
};

bool RequestEventProducer::initialize() {
  GlobalPlugin::registerHook(Plugin::HOOK_READ_REQUEST_HEADERS_PRE_REMAP);

  kafka_rest = createKafkaProducer(rest_proxy_url, schema_registry_url, kafka_logger,
                                   USER_REQUEST_EVENT_TOPIC, USER_REQUEST_EVENT_SCHEMA,
                                   "requestevent-kafka.log", 1);
  if (!kafka_rest) {
    std::cerr << "Failed to create a Kafka producer\n";
    return false;
  }

  atscppapi::StatusProviderService::registerProvider(LOG_TAG, this);
  return true;
}

void RequestEventProducer::handleReadRequestHeadersPreRemap(atscppapi::Transaction &transaction) {
  atscppapi::ClientRequest &clientRequest = transaction.getClientRequest();
  atscppapi::Headers &requestHeaders = clientRequest.getHeaders();
  // Create a transaction plugin here.
  if (requestHeaders.find("client_ip") != requestHeaders.end() || true)
    transaction.addPlugin(new TxnProducer(transaction));
  transaction.resume();
}

void TSPluginInit(int argc, const char *argv[]) {
  (void) argv;
  if (argc != 1) {
    std::cerr << "Error: too many arguments.";
    abort();
  }
  RequestEventProducer *REP = new RequestEventProducer();
  if (!REP->initialize()) {
    TSError("Could not initialize request event producer");
    abort();
  }

  // LOG_TAG has to be a string literal here, not even a const char *.
  TS_DEBUG(LOG_TAG, "Finished innitializing request event producer.");
}

