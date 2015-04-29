// Copyright 2015 zeb209. All rights reserved.
// Use of this source code is governed by a Pastleft
// license that can be found in the LICENSE file.

// A simple origin server for testing purposes.

package main

import (
	"fmt"
	"net/http"
	//"net/http/httptest"
	"net/http/httputil"
	//"net/url"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	//"strings"
	//"testing"
)

type Page struct {
	Title string
	Body []byte
}

func (p *Page) save() error {
	filename := p.Title + ".txt"
	return ioutil.WriteFile(filename, p.Body, 0600)
}

func loadPage(title string) (*Page, error) {
	filename := title + ".txt"
	body, err := ioutil.ReadFile(filename)
	return &Page{Title: title, Body: body}, err
}

// The handler for http request path /view/page_name
func viewHandler(w http.ResponseWriter, r *http.Request) {
	title := r.URL.Path[len("/view/"):]
	p, _ := loadPage(title)
	fmt.Fprintf(w, "<h1>%s</h1><div>%s</div>", p.Title, p.Body)
}

// The default handler for all http requests.
func defaultHandler(w http.ResponseWriter, r *http.Request) {
	// Dump the request for debuggin.
	dump, err := httputil.DumpRequest(r, true)
	if err != nil {
		panic(err)
	}
	fmt.Println(string(dump))

	p, err := loadPage("TestPage") // ".txt" will be appended.
	if err != nil {
		log.Fatal(err)
	}

	// Send the response.
	fmt.Fprintf(w, "<h1>%s</h1><div>%s</div>", p.Title, p.Body)
}


func main() {
	// Get the current directory.
	dir, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		log.Fatal(err)
	}
	body := "This is a test page under: " + dir
	p1 := &Page{Title: "TestPage", Body: []byte(body)}
	p1.save()
	p2, _ := loadPage("TestPage")
	fmt.Println(string(p2.Body))

	// http.HandleFunc("/view/", viewHandler)
	http.HandleFunc("/", defaultHandler)
	http.ListenAndServe(":8081", nil)
}
