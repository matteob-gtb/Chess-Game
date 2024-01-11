CFLAGS = -std=c++17 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -I./headers -I.

FILENAME = Graphics.cpp
OUTNAME = chessGame

VulkanTest: $(FILENAME)
	glslc shaders/Mesh.vert -o shaders/MeshVert.spv
	glslc shaders/Mesh.frag -o shaders/MeshFrag.spv
	glslc shaders/TexMesh.vert -o shaders/TexMeshVert.spv
	glslc shaders/TexMesh.frag -o shaders/TexMeshFrag.spv

	glslc shaders/TexIcon.frag -o shaders/TexIconFrag.spv

	glslc shaders/Overlay.vert -o shaders/OverlayVert.spv
	glslc shaders/Overlay.frag -o shaders/OverlayFrag.spv
	glslc shaders/VColor.vert -o shaders/VColorVert.spv
	glslc shaders/VColor.frag -o shaders/VColorFrag.spv

	glslc shaders/VColorCheckerboard.frag -o shaders/VColorCheckerboardFrag.spv

	g++ $(CFLAGS) -o $(OUTNAME) $(FILENAME)  $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./$(OUTNAME)

clean:
	rm -f $(OUTNAME)
