// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

#pragma once
#ifndef REQUEST_EVENT_KAFKA_PRODUCER_H
#define REQUEST_EVENT_KAFKA_PRODUCER_H

#include <atscppapi/AsyncHttpFetch.h>
#include <atscppapi/Mutex.h>
#include <ctime>
#include <string>
#include <sstream>
#include <li_atscppapi/li_utils.h>
#include <vector>

class KafkaProducer : public atscppapi::AsyncReceiver<atscppapi::AsyncHttpFetch> {
public:
  KafkaProducer(const std::string &server_url, const std::string &logFileName,
                const std::string &topic, const std::string &schema_id, int max_event_queue_size) :
  max_event_queue_size (max_event_queue_size) {
    url = server_url + "/" + topic + "/?schemaId=" + schema_id;
    logger.init(logFileName);
    event_queue.reserve(max_event_queue_size);
  }

  // Send a http post request to the kafka url.
  void sendEvent(const std::string &data) {
    std::string post_body("--");
    { // For scoped locks.
      atscppapi::ScopedMutexLock scopedMutexLock(event_queue_mutex);
      event_queue.push_back(data);
      if (event_queue.size() < max_event_queue_size)
        return;

      timespec tp;
      clock_gettime(CLOCK_REALTIME, &tp);
      std::ostringstream oss;
      oss << (tp.tv_sec * 1000 + tp.tv_nsec / 1000000.0) + 0.5;
      std::string boundary = oss.str();
      for (size_t i = 0; i < max_event_queue_size; ++i) {
        post_body += boundary;
        post_body += "\r\n";
        post_body += "Content-Disposition: form-data; name=\"message"; // content disposition bit
        post_body += "1"; // message count.
        post_body += "\"\r\nContent-Type: application/json\r\ncharset=utf-8\r\n\r\n"; // content type bit
        post_body += event_queue[i];
        post_body += "\r\n";
        post_body += "--";
      }
      post_body += boundary;
      post_body += "--\r\n";
      event_queue.clear();
    } // Close scoped locks.
    atscppapi::AsyncHttpFetch *http_fetch = new atscppapi::AsyncHttpFetch(url, post_body);
    atscppapi::Headers &headers = http_fetch->getRequestHeaders();
    headers.set("Content-Type", "multipart/form-data");
    headers.set("charsets", "utf-8");

    std::cout << url << "\n";
    std::cout << post_body;
    atscppapi::Async::execute(this, http_fetch, atscppapi::shared_ptr<atscppapi::Mutex>());
  }

  virtual void handleAsyncComplete(atscppapi::AsyncHttpFetch &http_request) {
    if (http_request.getResult() == atscppapi::AsyncHttpFetch::RESULT_SUCCESS ||
        http_request.getResponse().getStatusCode() != 200) {
      std::cerr << "POST request to Kafka failed with status code: "
                << http_request.getResponse().getStatusCode() << "\n";
    }
  }

private:
  std::string url;
  unsigned max_event_queue_size;
  std::vector<std::string> event_queue;
  atscppapi::Mutex event_queue_mutex;
  atscppapi::Logger logger;
};

#endif

