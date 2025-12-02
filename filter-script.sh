#!/bin/bash
if [ -f README.md ]; then
  git show 8551983:README.md > README.md
  git add README.md
fi
