#!/usr/bin/env bash

eosio-cpp src/stuff_token.cpp -o stuff_token.wasm -abigen -I include -I ./src -R resources