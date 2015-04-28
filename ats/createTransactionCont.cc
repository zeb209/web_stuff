///================================= createTransactionCont.cc ===============///
/// Create a transaction continuation. This file exposes the bug with
/// Expect:100-continue bug in the ATS core code.

#include <unistd.h>
#include <ts/ts.h>

namespace {
  int handleReadResponseHeader(TSCont cont, TSEvent event, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);

    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    TSContDestroy(cont);
    sleep(1);
    return 0;
  }

  int handleReadRequestHeader(TSCont cont, TSEvent event, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);
    if (TSHttpIsInternalRequest(txn) == TS_SUCCESS) {
      TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
      return 0;
    }

    // Add a transaction continuation here.
    TSCont delayCont = TSContCreate(handleReadResponseHeader, 0);
    TSHttpTxnHookAdd(txn, TS_HTTP_READ_RESPONSE_HDR_HOOK, delayCont);
    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    return 0;
  }

  // If it is a global continuation, then it cannot be destroyed. so the problem
  // is not triggered.
  int handleReadResponseHeaderGlobal(TSCont cont, TSEvent event, void *edata) {
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);

    if (TSHttpIsInternalRequest(txn) == TS_SUCCESS) {
      TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
      return 0;
    }
    TSHttpTxnReenable(txn, TS_EVENT_HTTP_CONTINUE);
    sleep(1);
    return 0;
  }
}

void TSPluginInit(int argc, const char *argv[]) {
  TSPluginRegistrationInfo info;

  info.plugin_name   = (char *)"createTransactionCont";
  info.vendor_name   = (char *)"LinkedIn";
  info.support_email = (char *)"bzeng@linkedin.com";

  if (!TSPluginRegister (TS_SDK_VERSION_2_0 , &info)) {
    TSError ("Plugin registration failed. \n");
  }

  TSCont cont = TSContCreate(handleReadRequestHeader, 0);
  TSHttpHookAdd(TS_HTTP_READ_REQUEST_HDR_HOOK, cont);
  TSCont cont2 = TSContCreate(handleReadResponseHeaderGlobal, 0);
  TSHttpHookAdd(TS_HTTP_SEND_RESPONSE_HDR_HOOK, cont2);
}

