#!/bin/bash
# Frozen upstream artifact flow (1.36.2); build_web_gen3.sh refuses to run.
exec bash "$(dirname "$0")/build_web_gen3.sh" "$@"
