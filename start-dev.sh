#!/bin/bash
#cd modules && make install && cd .. && make install &&
make makeall &&
sudo spawn-fcgi -d /home/omni/devel/openapi/trunk -n -p 9001 -u www-data -F 1 -- ./run-dev.sh
#cd modules && make install && cd .. && make install && sudo spawn-fcgi -n -p 9001 -u www-data -F 1 test
#cd modules && make install && cd .. && make install && sudo spawn-fcgi -p 9001 -u www-data -F 1 openapi
