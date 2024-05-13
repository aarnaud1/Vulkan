# Copyright (C) 2024 Adrien ARNAUD
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

CXX       := g++ -W -Wall -Wextra -std=c++17
CXX_FLAGS := -O3 --pedantic -ffast-math
IFLAGS    := -I./include
LFLAGS    := -L./output/lib -lVkWrappers -lvulkan -lglfw

SHADERS_SPV := $(patsubst main/shaders/%.comp,output/spv/%.spv,$(wildcard main/shaders/*.comp))
OBJ_FILES   := $(patsubst src/%.cpp,output/obj/%.o,$(wildcard src/*.cpp))

MODULE := output/lib/libVkWrappers.so

MAIN_UTILS := $(wildcard main/utils/*.cpp)
EXEC := output/bin/ArrayAdd \
        output/bin/ArraySaxpy \
		output/bin/GaussianBlur

all: deps $(MODULE) $(EXEC)
lib: deps $(MODULE)
	$(shell) rm -rfd output/obj/ output/spv/ output/bin

output/obj/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) --pedantic -c -fPIC $(IFLAGS) -o $@ $<

$(MODULE): $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) -shared -o $@ $^

output/bin/%: main/%.cpp $(MAIN_UTILS)
	$(CXX) $(CXX_FLAGS) -o $@ $(IFLAGS) -I./main/utils -I./stb/ $^ $(LFLAGS)

output/spv/%.spv: main/shaders/%.comp
	glslc -std=450core -fshader-stage=compute -o $@ $^

shaders: $(SHADERS_SPV)

deps:
	$(shell) mkdir -p output/spv
	$(shell) mkdir -p output/obj
	$(shell) mkdir -p output/lib
	$(shell) mkdir -p output/bin

clean:
	rm -rfd output