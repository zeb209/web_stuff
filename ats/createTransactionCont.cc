// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

///================================= createTransactionCont.cc ===============///
/// Create a transaction continuation. This file exposes the bug with
/// Expect:100-continue bug in the ATS core code.

#include <unistd.h>
#include <ts/ts.h>

namespace {
  int transactionHook(TSCont cont, TSEvent /*event*/, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);

    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    TSContDestroy(cont);
    sleep(1);
    return 0;
  }

  int handleReadRequestHeader(TSCont /*cont*/, TSEvent /*event*/, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);
    if (TSHttpTxnIsInternal(txn) == TS_SUCCESS) {
      TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
      return 0;
    }

    // Add a transaction continuation here.
    TSCont delayCont = TSContCreate(transactionHook, 0);
    // TSHttpTxnHookAdd(txn, TS_HTTP_READ_REQUEST_HDR_HOOK, delayCont); // this is illegal.
    // TSHttpTxnHookAdd(txn, TS_HTTP_OS_DNS_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_SEND_REQUEST_HDR_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_READ_CACHE_HDR_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_READ_RESPONSE_HDR_HOOK, delayCont); // yes, go through twice.
    TSHttpTxnHookAdd(txn, TS_HTTP_SEND_RESPONSE_HDR_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_REQUEST_TRANSFORM_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_RESPONSE_TRANSFORM_HOOK, delayCont); // does not go through HttpSM twice.
    // TSHttpTxnHookAdd(txn, TS_HTTP_TXN_CLOSE_HOOK, delayCont); // does not go through HttpSM twice.

    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    return 0;
  }

  // If it is a global continuation, then it cannot be destroyed. so the problem
  // is not triggered.
  int handleReadResponseHeaderGlobal(TSCont /*cont*/, TSEvent /*event*/, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);

    if (TSHttpTxnIsInternal(txn) == TS_SUCCESS) {
      TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
      return 0;
    }
    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    sleep(1);
    return 0;
  }
}

void TSPluginInit(int /*argc*/, const char */*argv*/[]) {
  TSPluginRegistrationInfo info;

  info.plugin_name   = (char *)"createTransactionCont";
  info.vendor_name   = (char *)"LinkedIn";
  info.support_email = (char *)"bzeng@linkedin.com";

  // if (!TSPluginRegister (TS_SDK_VERSION_2_0 , &info)) {
  if (!TSPluginRegister(&info)) {
    TSError ("Plugin registration failed. \n");
  }

  TSCont cont = TSContCreate(handleReadRequestHeader, 0);
  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);
  TSCont cont2 = TSContCreate(handleReadResponseHeaderGlobal, 0);
  TSHttpHookAdd(TS_HTTP_SEND_RESPONSE_HDR_HOOK, cont2);
}

