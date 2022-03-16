#!/bin/bash
ulimit -c unlimited
./openapi -c etc/openapi-prod.conf
