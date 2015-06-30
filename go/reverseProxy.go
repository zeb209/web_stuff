package main

import (
	"fmt"
	"log"
	"net/http"
	"net/http/httputil"
)

func printRequest(r *http.Request) {
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
	r.URL.Scheme = scheme
	r.URL.Host = host
}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		director := func(req *http.Request) {
			printRequest(req)
			req = r
			req.URL.Scheme = "http"
			req.URL.Host = r.Host
		}
		proxy := &httputil.ReverseProxy{Director: director}
		// fmt.Fprintf(w, "Debug server in golang\n")
		w.Header().Set("Via", "Golang-Reverse-Proxy")
		proxy.ServeHTTP(w, r)
	})
	log.Fatal(http.ListenAndServe(":8181", nil))
	// log.Fatal(http.ListenAndServeTLS(":8181", "/etc/trafficserver/server.crt", "/etc/trafficserver/server.key", nil))
}


