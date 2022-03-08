#!/usr/bin/env bash

eosio-cpp src/stuff_trade.cpp -o stuff_trade.wasm -abigen -I include -I ./src -I ../shared -R resources -R ricardian