#!/bin/bash

# Copyright 2015 zeb209. All rights reserved.
# Use of this source code is governed by a Pastleft
# license that can be found in the LICENSE file.

# svn does not have a command to show all the diffs of a file.
# we simulate it with svn diff here.

# Outputs the full history of a given file as a sequence of
# log entry/diff pairs.  The first revision of the file is emitted as
# full text since there's not previous version to compare it to.

function svn_history() {
    url=$1 # current url of file
    # get the revisions of the file.
    svn log -q $url | grep -E -e "^r[[:digit:]]+" -o | cut -c2- | sort -n | {

#       first revision as full text
        echo
        read r
        svn log -r$r $url@HEAD
        svn cat -r$r $url@HEAD
        echo

#       remaining revisions as differences to previous revision
        while read r
        do
            echo
            svn log -r$r $url@HEAD
            svn diff -c$r $url@HEAD
            echo
        done
    }
}

svn_history $1
