// A tool to send requests.

package main

import (
	"fmt"
	"log"
	"net/http"
	"net/http/httputil"
	"io/ioutil"
	"bytes"
	"strconv"
)

var (
	loopbackUrl = "http://127.0.0.1:8081"
)

// Send a http POST to an url.
func sendHttpPost(url string) {
	body := "This is a test string."
	for i := 0; i < 10; i++ {
		body = body + body
	}
	// body, err := ioutil.ReadFile("epoll-example.c")
	// if err != nil {
	// 	panic(err) // log.Fatal(err)
	// }

	req, err := http.NewRequest("POST", url, bytes.NewBufferString(body))
	req.Header.Set("Expect", "100-continue")
	req.Header.Set("Content-Type", "text/html")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()

	fmt.Println("response Status:", resp.Status)
	fmt.Println("response Headers:", resp.Header)
	rcvbody, _ := ioutil.ReadAll(resp.Body)
	fmt.Println("response Body:", string(rcvbody))
}

// Send a Expect:100-continue
func sendExpect100(url string) {
	body := "This is a test string."
	for i := 0; i < 10; i++ {
		body = body + body
	}

	// Send Expect:100-continue without the body.
	req, _ := http.NewRequest("POST", url, nil)
	req.Header.Set("Expect", "100-continue")
	req.Header.Set("Content-Type", "text/html")
	req.Header.Set("Content-Length", strconv.Itoa(len(body)))
	req.Header.Set("Test-Header", "Test-Header-Value")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close() // Need to close the body.

	dump, err := httputil.DumpResponse(resp, true)
	if err != nil {
		panic(err)
	}
	fmt.Println(string(dump))

	fmt.Println("response Status:", resp.Status)
	fmt.Println("response Headers:", resp.Header)
	rcvBody, _ := ioutil.ReadAll(resp.Body)
	fmt.Println("response Body:", rcvBody)

	// Send the body.
	req2, err := http.NewRequest("POST", url, bytes.NewBufferString(body))
	if err != nil {
		log.Fatal(err)
	}
	resp2, rcvErr := client.Do(req2)
	if rcvErr != nil {
		panic(rcvErr)
	}
	defer resp2.Body.Close()
	dump2, err := httputil.DumpResponse(resp2, true)
	if err != nil {
		panic(err)
	}
	fmt.Println(string(dump2))
}

func sentHttpGet(getUrl string) {
	// Send a http GET request
	resp, err := http.Get(getUrl)
	if err != nil {
		fmt.Println(err)
	}
	defer resp.Body.Close() // must close the body
	fmt.Println(resp)
}

func main() {
	//sendHttpPost(loopbackUrl)
	//sendExpect100(loopbackUrl)
	sendExpect100("http://httpbin.com/post")
}


