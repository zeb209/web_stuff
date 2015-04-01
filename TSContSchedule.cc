#include <netinet/in.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <ts/ts.h>
#include <ink_defs.h>

extern void *plugin_http_accept;

class AutoAsyncRefresher;

struct RefreshContData {
  TSAction firstRefreshAction;
  TSAction periodicRefreshAction;
  TSAction backupDataReadAction;
  std::string backupDataFilename;
  AutoAsyncRefresher *refresher; // Does not take ownership.
  RefreshContData(const std::string &backupDataFilename, AutoAsyncRefresher *refresher)
  : firstRefreshAction(NULL),
    periodicRefreshAction(NULL),
    backupDataReadAction(NULL),
    backupDataFilename(backupDataFilename),
    refresher(refresher) { }
};

static int handleFetchEvents(TSCont cont, TSEvent event, void *edata) {
  if (event == 10000) { // success.
    TSHttpTxn txn = static_cast<TSHttpTxn>(edata);
    int dataLen = 0; // The length of fetched data.
    // Get the starting position of fetched data.
    const char *dataStart = TSFetchRespGet(txn, &dataLen);
    const char *dataEnd = dataStart + dataLen;
    std::cout << dataStart;

    TSHttpParser parser = TSHttpParserCreate();
    TSMBuffer buf = TSMBufferCreate();
    TSMLoc hdrLoc = TSHttpHdrCreate(buf);

    TSHttpHdrTypeSet(buf, hdrLoc, TS_HTTP_TYPE_RESPONSE);
    if (TSHttpHdrParseResp(parser, buf, hdrLoc, &dataStart, dataEnd) == TS_PARSE_DONE) {
      TSHttpStatus status = TSHttpHdrStatusGet(buf, hdrLoc);
      if (!status)
        std::cout << dataStart << "\n";
      // TODO: handle the fetched data here.
    } else {
      std::cout << "Fail to parse http response\n";
    }

    TSHandleMLocRelease(buf, 0, hdrLoc);
    TSMBufferDestroy(buf);
    TSHttpParserDestroy(parser);
  } else if (event == 10001) { // failure.
    std::cout << "HTTP GET failure";
  } else if (event == 10002) { // timeout
    std::cout << "HTTP GET timeout";
  } else {
    std::cout << "Unrecognized status";
  }
  return 0;
}

// Initialize the sockaddr_in with the target address.
int initializeSockaddr(const char *ip, int port, struct sockaddr_in *addr) {
  if (inet_aton(ip, &addr->sin_addr) == 0) {
    std::cout << "Invalid IP address\n";
    return -1;
  }
  addr->sin_family = AF_INET;
  // addr->sin_addr.s_addr = 0x0100007f /* 127.0.0.1 */;
  addr->sin_port = port /* 8080 */;
  return 0;

}

// Fetch an object through a URL and http 1.0.
void fetchHttpUrl(const std::string &url) {
  std::string requestString("GET ");
  requestString.append(url);
  requestString.append(" HTTP/1.0\r\n\r\n");

  // Create a continuation to handle the event when the fetch result arrives.
  TSCont fetchCont = TSContCreate(handleFetchEvents, TSMutexCreate());

  TSFetchEvent eventIds;
  eventIds.success_event_id = 10000;
  eventIds.failure_event_id = 10001;
  eventIds.timeout_event_id = 10002;

  struct sockaddr_in addr;
  //if (initializeSockaddr("172.20.200.150", 10126 /* port number*/, &addr) != 0) {
  if (initializeSockaddr("127.0.0.1", 8080 /* port number*/, &addr) != 0) {
    std::cout << "Error initializing sockaddr\n";
  }

  TSFetchUrl(requestString.c_str(), requestString.size(), reinterpret_cast<struct sockaddr const *>(&addr),
             fetchCont, AFTER_BODY, eventIds);
}

class AutoAsyncRefresher {
public:
  AutoAsyncRefresher(const std::string backupFilename, const std::string url) : url(url){
    if (pthread_rwlock_init(&dataLock, 0)) {
      std::cout << "Failed to initialize dataLock\n";
      exit(1);
    }
    if (pthread_mutex_init(&refreshLock, NULL)) {
      std::cout << "Failed to initialize refreshLock\n";
      exit(1);
    }
    refreshFailureStat = TSStatCreate("refresh_failure_stat", TS_RECORDDATATYPE_INT, TS_STAT_PERSISTENT,
                                      TS_STAT_SYNC_COUNT);
    refreshCont = TSContCreate(handleRefreshContEvent, 0);
    RefreshContData *refreshContData = new RefreshContData(backupFilename, this);

    TSContDataSet(refreshCont, refreshContData);
    //refreshContData->periodicRefreshAction = TSContScheduleEvery(refreshCont, 5000 /* ms */, TS_THREAD_POOL_DEFAULT);
    refreshContData->periodicRefreshAction = TSContSchedule(refreshCont, 5000 /* ms */, TS_THREAD_POOL_DEFAULT);
  }

  static int handleRefreshContEvent(TSCont cont, TSEvent event, void *edata) {
    assert(event == TS_EVENT_TIMEOUT || event == TS_EVENT_IMMEDIATE);
    RefreshContData *refreshContData = static_cast<RefreshContData *>(TSContDataGet(cont));
    refreshContData->refresher->refresh(refreshContData);

    // Reschedule itself here.
    refreshContData->periodicRefreshAction = TSContSchedule(cont, 5000 /* ms */, TS_THREAD_POOL_DEFAULT);
    return 0;
  }

  void refresh(RefreshContData *refreshContData) {
    if (refreshContData->backupDataReadAction) {
      refreshContData->backupDataReadAction = NULL;
    } else if (!plugin_http_accept){
      refreshContData->firstRefreshAction = TSContSchedule(refreshCont, 1000 /* ms */,
                                                           TS_THREAD_POOL_DEFAULT);
    } else {
      refreshContData->firstRefreshAction = 0;
      bool initiateRefresh = false;
      pthread_mutex_lock(&refreshLock);
      if (!refreshInProgress) {
        refreshInProgress = true;
        initiateRefresh = true;
      }
      pthread_mutex_unlock(&refreshLock);

      if (initiateRefresh) {
        fetchHttpUrl(url);
        refreshInProgress = false;
      } else {
        std::cout << "Refresh already in progress...";
      }
    }
  }

private:
  int refreshFailureStat;
  TSCont refreshCont; // The continuation used to fetch data.
  std::string url;    // The url to fetch data from.
  pthread_rwlock_t dataLock;
  pthread_mutex_t refreshLock;
  bool refreshInProgress;
};

void TSPluginInit(int argc, const char *argv[]) {
  AutoAsyncRefresher *refresher = new AutoAsyncRefresher("/home/bzeng/web/ATS_plugins/test/backup.data",
                                                         "http://eat1-app533.stg.linkedin.com:10126/bwl-ds/rest/getbwl?name=CIPL");
                                                         //"http://eat1-app533.stg.linkedin.com:10126/bwl-ds/rest/getbwl?name=CWLP&type=current");
                                                         //"www.google.com");
  (void) refresher;
}
