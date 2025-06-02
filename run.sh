#!/bin/bash

gcc main.c -o app -Wall -Werror -std=c99 -fsanitize=address && ./app
