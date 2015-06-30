// http://stackoverflow.com/questions/27880930/mocking-https-responses-in-go
// http://play.golang.org/p/afljO086iB

package main

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"path"
	"strings"
)

func Handler(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "hellow %s\n", path.Base(r.URL.Path))
}

func main() {
	s := httptest.NewServer(http.HandlerFunc(Handler))
	u, err := url.Parse(s.URL)
	if err != nil {
		log.Fatalln("failed to parse httptest.Server URL:", err)
	}
	http.DefaultClient.Transport = RewriteTransport{URL: u}
	resp, err := http.Get("https://google.com/path-one")
	if err != nil {
		log.Fatalln("failed to send first request:", err)
	}
	fmt.Println("[First Response]")
	resp.Write(os.Stdout)

	fmt.Print("\n", strings.Repeat("-", 80), "\n\n")

	http.DefaultClient.Transport = HandlerTransport{http.HandlerFunc(Handler)}
	resp, err = http.Get("https://google.com/path-two")
	if err != nil {
		log.Fatalln("failed to send second request:", err)
	}
	fmt.Println("[Second Response]")
	resp.Write(os.Stdout)
}

// RewriteTransport is an http.RoundTripper that rewrites requests using the
// provided URL's Scheme and Host, and its Path as a prefix.
// The Opaque field is untouched.
// If Transport is nil, http.DefaultTransport is used
type RewriteTransport struct {
	Transport http.RoundTripper
	URL       *url.URL
}

func (t RewriteTransport) RoundTrip(req *http.Request) (*http.Response, error) {
	// note that url.URL.ResolveReference doesn't work here.
	// since t.u is an absolute url
	req.URL.Scheme = t.URL.Scheme
	req.URL.Host = t.URL.Host
	req.URL.Path = path.Join(t.URL.Path, req.URL.Path)
	rt := t.Transport
	if rt == nil {
		rt = http.DefaultTransport
	}
	return rt.RoundTrip(req)
}

type HandlerTransport struct { h http.Handler }

func (t HandlerTransport) RoundTrip(req *http.Request) (*http.Response, error) {
	r, w := io.Pipe()
	resp := &http.Response {
		Proto:  "HTTP/1.1",
		ProtoMajor: 1,
		ProtoMinor: 1,
		Header: make(http.Header),
		Body: r,
		Request: req,
	}
	ready := make(chan struct{})
	prw := &pipeResponseWriter{r, w, resp, ready}
	go func() {
		defer w.Close()
		t.h.ServeHTTP(prw, req)
	}()
	<-ready
	return resp, nil
}

type pipeResponseWriter struct {
	r     *io.PipeReader
	w     *io.PipeWriter
	resp  *http.Response
	ready chan<-struct{}
}

func (w *pipeResponseWriter) Header() http.Header {
	return w.resp.Header
}

func (w *pipeResponseWriter) Write(p []byte) (int, error) {
	if w.ready != nil {
		w.WriteHeader(http.StatusOK)
	}
	return w.w.Write(p)
}

func (w *pipeResponseWriter) WriteHeader(status int) {
	if w.ready == nil {
		// already called
		return
	}
	w.resp.StatusCode = status
	w.resp.Status = fmt.Sprintf("%d %s", status, http.StatusText(status))
	close(w.ready)
	w.ready = nil
}


