package http_test

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"net/url"
	"io/ioutil"
	"strings"
	"testing"
)

const continueResponse = `HTTP/1.1 100 Continue

HTTP/1.1 200 OK
X-foo: bar

baz`

var continueHandler = http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
	c, _, _ := w.(http.Hijacker).Hijack()
	defer c.Close()
	fmt.Fprintf(c, continueResponse)
})

func TestHandle100Continue(t *testing.T) {
	ts := httptest.NewServer(continueHandler)
	defer ts.Close()

	tr := &http.Transport{}
	c  := &http.Client{Transport: tr}

	req := new(http.Request)
	var err error
	req.Method = "POST"
	req.Proto = "HTTP/1.1"
	req.ProtoMajor = 1
	req.ProtoMinor = 1
	req.URL, err = url.Parse(ts.URL)
	if err != nil {
		t.Fatalf("URL parse error: %v", err)
	}
	req.Body = ioutil.NopCloser(strings.NewReader("123"))

	res, err := c.Do(req)
	if err != nil {
		t.Fatalf("Error in Do: %v", err)
	}
	body, err := ioutil.ReadAll(res.Body)
	defer res.Body.Close()
	if err != nil {
		t.Fatalf("Error in ReadAll: %v", err)
	}

	if res.StatusCode != 200 {
		t.Errorf("StatusCode is not 200: %d", res.StatusCode)
	}
	xfoo := res.Header.Get("X-foo")
	if xfoo != "bar" {
		t.Errorf("No X-foo header/X-foo header is not bar: %v", xfoo)
	}
	str := string(body)
	if str != "baz" {
		t.Errorf("Body is not baz: %v", str)
	}
}
