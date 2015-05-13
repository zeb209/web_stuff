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
#include <li_atscppapi/StatusProviderService.h>
#include <string>

#include "li_kafka.h"
#include "KafkaProducer.h"
#include "RequestEvent.h"

#define LOG_TAG "requesteventproducer"

namespace {
  const std::string rest_proxy_url      = "http://tracking-rest-proxy/tracking-rest/kafka/topics";
  const std::string schema_registry_url = "http://eat1-app110.stg.linkedin.com:10252/schemaRegistry/schemas";
  const std::string REQUEST_EVENT_TOPIC = "RequestEvent";
  const std::string REQUEST_EVENT_SCHEMA =
    "{"
    "  \"type\": \"record\","
    "  \"doc\": \"Represents a user's direct HTTP reuqest.\","
    "  \"name\": \"RequestEvent\","
    "  \"fields\": ["
    "    {"
    "      \"name\": \"header\","
    "      \"doc\": \"Request header\","
    "      \"type\": ["
    "        \"null\", {"
    "          \"type\": \"record\","
    "          \"name\": \"EventHeader\","
    "          \"namespace\": \"com.xxx.events\","
    "          \"doc\": \"The basic header for every tracking event.\","
    "          \"fields\": ["
    "            {\"name\": \"memberId\", \"type\": \"int\",  \"doc\": \"The member Id of a request.\"},"
    "            {\"name\": \"time\",     \"type\": \"long\", \"doc\": \"The time of the event.\"}"
    "          ]"
    "        }"
    "      ]"
    "    },"
    "    {"
    "      \"name\": \"request\","
    "      \"type\": [ \"null\", \"string\" ],"
    "      \"doc\": \"request body\""
    "    }"
    "  ]"
    "}";

  atscppapi::Logger logger;
  KafkaProducer *kafka_rest;

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
}

class TxnProducer : public atscppapi::TransactionPlugin {
public:
  TxnProducer(atscppapi::Transaction &txn) : TransactionPlugin(txn), transaction(txn) {
    event_str += "TxnProducer constructor";
  }

  ~TxnProducer() {
    event_str += "TxnProducer destructor";
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
  // To create a kafka producer, you need to create a kafka object.
  li_kafka lik = li_kafka_init_rest(rest_proxy_url.c_str(), schema_registry_url.c_str(),
                                    kafka_logger, LI_KAFKA_LOG_LEVEL_INFO);
  if (lik == NULL) {
    LOG_ERROR2(logger, LOG_TAG, "Failed to initialize kafka");
    return false;
  }

  // Register the schema with the topic and schema.
  if (!li_kafka_register_schema(lik, REQUEST_EVENT_TOPIC.c_str(), REQUEST_EVENT_SCHEMA.c_str())) {
    LOG_ERROR2(logger, LOG_TAG, "cannot register schema");
    return false;
  }

  // Get the schema ID. Try 10 times to see whether it still fails.
  char schema_id[256] = {0};
  int count = 10;
  while (count-- > 0) {
    if (li_kafka_get_schema_id(lik, REQUEST_EVENT_TOPIC.c_str(), schema_id, sizeof(schema_id)))
      break;
    usleep(10000/* micro seconds*/);
  }

  GlobalPlugin::registerHook(Plugin::HOOK_READ_REQUEST_HEADERS_PRE_REMAP);
  kafka_rest = new KafkaProducer(rest_proxy_url, "requestevent-kafka.log", REQUEST_EVENT_TOPIC,
                                 schema_id, 100 /* queue_size */);

  atscppapi::StatusProviderService::registerProvider(LOG_TAG, this);
  return true;
}

void RequestEventProducer::handleReadRequestHeadersPreRemap(atscppapi::Transaction &transaction) {
  atscppapi::ClientRequest &clientRequest = transaction.getClientRequest();
  atscppapi::Headers &requestHeaders = clientRequest.getHeaders();
  // Create a transaction plugin here.
  if (requestHeaders.find("client_ip") != requestHeaders.end() || true)
    transaction.addPlugin(new TxnProducer(transaction));
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

