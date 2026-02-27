#!/bin/sh
# Copyright (C) 2026 Ivan Gaydardzhiev
# Licensed under the GPL-3.0-only

for obj in *.o; do
	printf "#### %s ####\n" "${obj}"
	objdump -d "${obj}"
	printf "\n\n"
done
