# -----------------------------------------------------------------------------
# Copyright 2016 (C) Sérgio Leal.
# -----------------------------------------------------------------------------
# 
#  Use of this source code is governed by a MIT-style license (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License in the LICENSE file.
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# 
#

CC=clang
PREFIX ?= /usr/local

SRC = watch.c
OBJ = $(SRC:.c=.o)
CFLAGS = -std=c11 -O3 -Wall
FLAGS = -std=c11 -O3 -Wall -framework CoreFoundation -framework CoreServices

all: watch

watch: $(OBJ)
	$(CC) $(OBJ) $(FLAGS) -o $@

.c.o:
	$(CC) $< $(CFLAGS) -c -o $@

install: watch
	cp -f watch $(PREFIX)/bin/watch

uninstall:
	rm -f $(PREFIX)/bin/watch

clean:
	rm -f watch $(OBJ)

.PHONY: clean install uninstall
