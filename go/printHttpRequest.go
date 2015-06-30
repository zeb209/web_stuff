package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"net/http/httputil"
)

func initLog() {
	f, err := os.OpenFile("testlogfile", os.O_RDWR | os.O_CREATE | os.O_APPEND, 0666)
	if err != nil {
		panic("error opening file")
	}
	defer f.Close()

	log.SetOutput(f)
	log.Println("This is a test log entry")
}

func debug(data []byte, err error) {
	if err == nil {
		fmt.Printf("%s\n\n", data);
	} else {
		log.Fatalf("%s\n\n", err);
	}
}

func printRequest(w http.ResponseWriter, r *http.Request) {
	// We need to set the URL.Scheme field to "http" and URL.Host, in order to call DumpRequestOut.
	scheme := r.URL.Scheme
	host := r.URL.Host
	r.URL.Scheme = "http"
	r.URL.Host = "Host"
	dump, err := httputil.DumpRequestOut(r, true)
	if err != nil {
	 	fmt.Printf("%s\n", err)
	 	panic("error during dump request")
	}
	fmt.Printf("%s\n", dump)
	fmt.Fprintf(w, "Debug server in golang\n")
	r.URL.Scheme = scheme
	r.URL.Host = host
}

func main() {
//	initLog()

	// In one goroutine, serve http on port 80.
	go func() {
		http.ListenAndServe(":80", &httpHandler{})
	}()

	// Serve https on port 443.
	http.HandleFunc("/", printRequest)
	err := http.ListenAndServeTLS(":443", "/etc/trafficserver/server.crt", "/etc/trafficserver/server.key", nil)
	if err != nil {
		log.Fatal(err)
	}
}

type httpHandler struct {}

func (m *httpHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	printRequest(w, r)
}

