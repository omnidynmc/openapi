#!/bin/bash
ulimit -c unlimited
./openapi -d -c etc/openapi.conf
