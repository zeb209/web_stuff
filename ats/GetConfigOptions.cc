// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.


///=============================== GetConfigOptions.cc ======================///
/// Get the config options in an ATS plugin.

#include <trafficserver-plugin-utils/ClientRequest.h>
#include <records/I_RecHttp.h>
#include <records/I_RecCore.h>
#include <iostream>

using namespace AtsPluginUtils;

#define _countof(array) (sizeof(array)/sizeof(0[array]))

// Get the port of a request.
// This does not work!!!
static int getPort(TSCont contp, TSHttpTxn txnp) {
  TSMBuffer reqp;
  TSMLoc hdr_loc, url_loc, field_loc;

  const char *request_host;
  int request_host_length = 0;
  const char *request_scheme;
  int request_scheme_length = 0;
  int request_port = 80;
  char *query;

  if (TSHttpTxnClientReqGet(txnp, &reqp, &hdr_loc) != TS_SUCCESS) {
    return -1;
  }

  TSHttpHdrUrlGet(reqp, hdr_loc, &url_loc);
  if (!url_loc) {
    return -2;
  }

  field_loc = TSMimeHdrFieldFind(reqp, hdr_loc, TS_MIME_FIELD_HOST, TS_MIME_LEN_HOST);
  if (!field_loc) {
    return -3;
  }

  request_host = TSMimeHdrFieldValueStringGet(reqp, hdr_loc, field_loc, -1, &request_host_length);
  if (!request_host_length) {
    return -4;
  }

  request_scheme = TSUrlSchemeGet(reqp, url_loc, &request_scheme_length);
  request_port = TSUrlPortGet(reqp, url_loc);
  return request_port;
}

static int getPort(TSHttpTxn txnp) {
  sockaddr const *sock = TSHttpTxnServerAddrGet(txnp);
  if (sock->sa_family == AF_INET)
    return ntohs(((struct sockaddr_in *)sock)->sin_port);
  return ntohs(((sockaddr_in6 *)sock)->sin6_port);
}

static int handleReadRequestHeader(TSCont contp, TSEvent /*event*/, void *edata) {
  TSHttpTxn txn = static_cast<TSHttpTxn>(edata);

  if (TSHttpIsInternalRequest(txn) == TS_SUCCESS) {
    // if (TSHttpTxnIsInternal(txn) == TS_SUCCESS) {
    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    return 0;
  }

  int timeout;
  char server_ports[128];
  std::cout << _countof(server_ports) << "\n";
  REC_ReadConfigInteger(timeout, "proxy.config.http.keep_alive_no_activity_timeout_in");
  REC_ReadConfigString(server_ports, "proxy.config.http.server_ports", _countof(server_ports));
  std::cout << "timeout: " << timeout << " server_ports: " << server_ports << "\n";
  TSPortDescriptor desc = TSPortDescriptorParse(server_ports);
  HttpProxyPort *port = reinterpret_cast<HttpProxyPort *>(desc);
  std::cout << port->m_port << "\n";

  const char *ptr = server_ports;
  while (*ptr != '\0' && *ptr != ' ')
    ++ptr;
  while (*ptr == ' ') ++ptr;
  if (*ptr != '\0') {
    desc = TSPortDescriptorParse(ptr);
    port = reinterpret_cast<HttpProxyPort *>(desc);
    std::cout << port->m_port << "\n";
  }

  std::cout << getPort(contp, txn) << "\n";

  std::cout << getPort(txn) << "\n";

  std::cout << TSHttpClientIncomingPortGet(txn) << "\n";

  TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
  return 0;
}


// Register the plugin.
bool registerPlugin() {
  TSPluginRegistrationInfo info;
  info.plugin_name   = (char *)"StartCorpIpFetcher";
  info.vendor_name   = (char *)"LinkedIn";
  info.support_email = (char *)"bzeng@linkedin.com";

  // return (TSPluginRegister(TS_SDK_VERSION_2_0 , &info) == TS_SUCCESS);
  return (TSPluginRegister(&info) == TS_SUCCESS);
}

void TSPluginInit(int /*argc*/, const char * /*argv*/[]) {
  if (!registerPlugin()) {
    TSError ("Plugin registration failed. \n");
  }

  TSCont cont = TSContCreate(handleReadRequestHeader, 0);
  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);
  char server_ports[128];
  std::cout << _countof(server_ports) << "\n";
}


