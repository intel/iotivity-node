#!/bin/bash

TEST_SCRIPT=$1
shift

exec node_modules/.bin/istanbul cover \
	--print none \
	--report none \
	--config "$(pwd)/tests/istanbul.json" \
	--dir "./coverage/$(echo "${TEST_SCRIPT}" | awk -F '/' '{ x = NF - 1; print $x "!" $NF;}')" "${TEST_SCRIPT}" \
	-- $@
