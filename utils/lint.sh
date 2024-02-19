#!/bin/sh

# pip install flake8 

PATH=~/.local/bin/:$PATH flake8 python/ --ignore=W503 --max-line-length=120
