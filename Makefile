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

CXX         := g++ -W -Wall -Wextra -Wmissing-field-initializers -Wconversion
CXX_FLAGS   := -std=c++17 -O2 -g --pedantic -ffast-math
GLSLC_FLAGS := -O --target-env=vulkan1.3
DEFINES     :=
IFLAGS      := -I./include
LFLAGS      := -L./output/lib -Wl,-rpath,./output/lib -lVkWrappers -lvulkan -lglfw -ltinyply

SHADERS_SPV := $(patsubst samples/shaders/%.comp,output/spv/%_comp.spv,$(wildcard samples/shaders/*.comp)) \
			   $(patsubst samples/shaders/%.vert,output/spv/%_vert.spv,$(wildcard samples/shaders/*.vert)) \
			   $(patsubst samples/shaders/%.frag,output/spv/%_frag.spv,$(wildcard samples/shaders/*.frag)) \
			   $(patsubst samples/shaders/%.mesh,output/spv/%_task.spv,$(wildcard samples/shaders/*.task)) \
			   $(patsubst samples/shaders/%.mesh,output/spv/%_mesh.spv,$(wildcard samples/shaders/*.mesh))
OBJ_FILES   := $(patsubst src/%.cpp,output/obj/%.o,$(wildcard src/*.cpp))

MODULE := output/lib/libVkWrappers.so

MAIN_UTILS := $(wildcard samples/utils/*.cpp)
SAMPLES := output/bin/ArrayAdd 			 \
           output/bin/ArraySaxpy 	   	 \
		   output/bin/GaussianBlur 		 \
		   output/bin/Triangle 			 \
		   output/bin/MeshShader

all: deps $(MODULE) $(SHADERS_SPV) $(SAMPLES)
lib: deps $(MODULE)
	$(shell) rm -rfd output/obj/ output/spv/ output/bin

output/obj/%.o: src/%.cpp
	$(CXX) $(CXX_FLAGS) $(DEFINES) -c -fPIC $(IFLAGS) -o $@ $<

$(MODULE): $(OBJ_FILES)
	$(CXX) $(CXX_FLAGS) -shared -o $@ $^

output/bin/%: samples/%.cpp $(MAIN_UTILS)
	$(CXX) $(CXX_FLAGS) -o $@ $(IFLAGS) -I./samples/utils -I./stb/ $^ $(LFLAGS)

output/spv/%_comp.spv: samples/shaders/%.comp
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=compute -o $@ $^
output/spv/%_vert.spv: samples/shaders/%.vert
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=vertex -o $@ $^
output/spv/%_frag.spv: samples/shaders/%.frag
	glslc -std=450core $(GLSLC_FLAGS) -fshader-stage=fragment -o $@ $^
output/spv/%_mesh.spv: samples/shaders/%.mesh
	glslc -std=450 $(GLSLC_FLAGS) -fshader-stage=mesh -o $@ $^
shaders: $(SHADERS_SPV)

$(SAMPLES): $(MODULE)

.PHONY: deps clean
deps:
	$(shell mkdir -p output/spv)
	$(shell mkdir -p output/obj)
	$(shell mkdir -p output/lib)
	$(shell mkdir -p output/bin)
clean:
	$(shell rm -rfd output)