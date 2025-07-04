#!/bin/bash
set -e

./scripts/build.sh

URLS=()
while IFS= read -r url; do
    URLS+=("$url")
done <test_urls.txt

./bin/website-word-counter "${URLS[@]}"
