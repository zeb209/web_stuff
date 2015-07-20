// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

///============================== StartCorpIpFetch.cc =======================///
/// Start the corporate IP fetcher. You need to send a request through curl
/// and instantiate a client request inside the plugin.

#include <trafficserver-plugin-utils/ClientRequest.h>

using namespace AtsPluginUtils;

static int handleReadRequestHeader(TSCont /*cont*/, TSEvent /*event*/, void *edata) {
  TSHttpTxn txn = static_cast<TSHttpTxn>(edata);
  if (TSHttpTxnIsInternal(txn) == TS_SUCCESS)
  {
    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    return 0;
  }

  // Corp Ip Fetcher will be started after the first instantiation of ClientRequest.
  ClientRequest &clientRequest = ClientRequest::getInstance(txn);
  (void) clientRequest;
  TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
  return 0;
}

void TSPluginInit(int /*argc*/, const char */*argv*/[]) {
  TSPluginRegistrationInfo info;

  info.plugin_name   = (char *)"StartCorpIpFetcher";
  info.vendor_name   = (char *)"LinkedIn";
  info.support_email = (char *)"bzeng@linkedin.com";

  if (!TSPluginRegister (TS_SDK_VERSION_2_0 , &info)) {
    TSError ("Plugin registration failed. \n");
  }

  TSCont cont = TSContCreate(handleReadRequestHeader, 0);
  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);
}

