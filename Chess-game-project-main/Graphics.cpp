// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"

#include "Structures.hpp"

#include "ChessPiece.hpp"

static BaseProject::DIRECTION choosingPieceDirection;
static Position2D selectingSpot; //can be modified to hold the first available place a piece can move to
/*once  enter has been pressed we can select a new place for the selected piece*/
static bool selectingNewPosition; //set to true when we press enter the first time
static bool enterPressed;
static int frameSinceKeysPolled = 0;


/*records if a key is pressed, checked every frame*/
static std::map<int, bool> keysMap{ {GLFW_KEY_W, 0}, {GLFW_KEY_A, 0}, {GLFW_KEY_S, 0},{GLFW_KEY_D, 0},{GLFW_KEY_F, 0},{GLFW_KEY_R, 0} };


// MAIN ! 
class A16 : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo, DSLMesh, DSLOverlay;
	/* A16 */
	/* Add the variable that will contain the required Descriptor Set Layout */
	DescriptorSetLayout DSLVColor;

	// Vertex formats
	VertexDescriptor VMesh;
	VertexDescriptor VOverlay;
	/* A16 */
	/* Add the variable that will contain the required Vertex format definition */
	VertexDescriptor VVColor;

	// Pipelines [Shader couples]
	Pipeline PMesh;
	Pipeline POverlay;
	/* A16 */
	/* Add the variable that will contain the new pipeline */
	Pipeline PChessboard;
	Pipeline PChesspiece;
	Pipeline PIcons;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MBody, MHandle, MWheel;


	Model<VertexMesh> MIconSquareModel; //squares that will contain the texture of the icon (projection of the piece on xy)



	/* A16 */
	/* Add the variable that will contain the model for the room */
	Model<VertexMesh> MChessboard;
	Model<VertexMesh> MPawn, MRook, MKnight, MBishop, MQueen, MKing;

	Model<VertexOverlay> MKey, MSplash;
	DescriptorSet DSGubo, DSIcons;



	/* A16 */
	/* Add the variable that will contain the Descriptor Set for the room */
	DescriptorSet DSChessboard;

	Texture TBody, THandle, TWheel, TKey, TSplash;

	//texture for the chess pieces
	Texture TWhitePawn, TWhiteRook, TWhiteKnight, TWhiteBishop, TWhiteQueen, TWhiteKing;
	Texture TBlackPawn, TBlackRook, TBlackKnight, TBlackBishop, TBlackQueen, TBlackKing;

	Texture TChessboard;

	//textures for the projection of the pieces
	Texture TIconsTextures[32];

	// C++ storage for uniform variables
	/* A16 */
	/* Add the variable that will contain the Uniform Block in slot 0, set 1 of the room */
	MeshUniformBlock uboChessboard;



	//ADDED FOR TEST PURPOSES
	ChessPiece* chessPieces[32];

	Icon* pieceProjections[32];



	GlobalUniformBlock gubo;
	OverlayUniformBlock uboKey, uboSplash;

	// Other application parameters
	float CamH, CamRadius, CamPitch, CamYaw;
	int gameState;
	float HandleRot = 0.0;
	float Wheel1Rot = 0.0;
	float Wheel2Rot = 0.0;
	float Wheel3Rot = 0.0;


#include <map>




	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{

		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		{
			enterPressed = true;
		}
		if (keysMap.count(key) != 0 && action == GLFW_RELEASE)
		{
			keysMap[key] = 1;
		}
	}


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "A16";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes
		/* A16 */
		/* Update the requirements for the size of the pool */
		uniformBlocksInPool = 50; //TO FIX
		texturesInPool = 7 + 32;
		setsInPool = 100;

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
			// this array contains the bindings:
			// first  element : the binding number
			// second element : the type of element (buffer or texture)
			//                  using the corresponding Vulkan constant
			// third  element : the pipeline stage where it will be used
			//                  using the corresponding Vulkan constant
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLOverlay.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});
		/* A16 */
		/* Init the new Data Set Layout */
		DSLVColor.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		// Vertex descriptors
		VMesh.init(this, {
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binging
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
					   sizeof(glm::vec2), UV}
			});

		VOverlay.init(this, {
				  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
					 sizeof(glm::vec2), OTHER},
			  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
					 sizeof(glm::vec2), UV}
			});
		/* A16 */
		/* Define the new Vertex Format */
		VVColor.init(this, {
				  {0, sizeof(VertexVColor), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexVColor, pos),
					 sizeof(glm::vec3), POSITION},
			  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexVColor, norm),
					 sizeof(glm::vec3), NORMAL},
			  {0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexVColor, color),
					 sizeof(glm::vec3), COLOR}
			});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		PMesh.init(this, &VMesh, "shaders/TexMeshVert.spv", "shaders/TexMeshFrag.spv", { &DSLGubo, &DSLMesh });


		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSLOverlay });
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

		/* A16 */
		/* Create the new pipeline, using shaders "VColorVert.spv" and "VColorFrag.spv" */
		PChessboard.init(this, &VMesh, "shaders/TexMeshVert.spv", "shaders/TexMeshFrag.spv", { &DSLGubo, &DSLMesh });
		PChesspiece.init(this, &VMesh, "shaders/TexMeshVert.spv", "shaders/TexMeshFrag.spv", { &DSLGubo, &DSLMesh });
		PIcons.init(this, &VMesh, "shaders/TexMeshVert.spv", "shaders/TexIconFrag.spv", { &DSLGubo, &DSLMesh });
		PIcons.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, true);
		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF



		MIconSquareModel.init(this, &VMesh, "models/Rectangle.obj", OBJ);


		/* A16 */
		/* load the mesh for the room, contained in OBJ file "Room.obj" */
		MChessboard.init(this, &VMesh, "models/Chessboard.obj", OBJ);
		MPawn.init(this, &VMesh, "models/Pawn.obj", OBJ);
		MRook.init(this, &VMesh, "models/Rook.obj", OBJ);
		MKnight.init(this, &VMesh, "models/Knight.obj", OBJ);
		MBishop.init(this, &VMesh, "models/Bishop.obj", OBJ);
		MQueen.init(this, &VMesh, "models/Queen.obj", OBJ);
		MKing.init(this, &VMesh, "models/King.obj", OBJ);




		// Create the textures
		// The second parameter is the file name

		//textures for the chess pieces
		TWhitePawn.init(this, "textures/LightWood/Pawn.png");
		TWhiteRook.init(this, "textures/LightWood/Rook.png");
		TWhiteKnight.init(this, "textures/LightWood/Knight.png");
		TWhiteBishop.init(this, "textures/LightWood/Bishop.png");
		TWhiteQueen.init(this, "textures/LightWood/Queen.png");
		TWhiteKing.init(this, "textures/LightWood/King.png");

		TBlackPawn.init(this, "textures/DarkWood/Pawn.png");
		TBlackRook.init(this, "textures/DarkWood/Rook.png");
		TBlackKnight.init(this, "textures/DarkWood/Knight.png");
		TBlackBishop.init(this, "textures/DarkWood/Bishop.png");
		TBlackQueen.init(this, "textures/DarkWood/Queen.png");
		TBlackKing.init(this, "textures/DarkWood/King.png");

		TChessboard.init(this, "textures/Chessboard.png");

		//texures for the projection of the pieces
		TIconsTextures[0].init(this, "textures/Icon/Black/Pawn.png");
		TIconsTextures[16].init(this, "textures/Icon/White/Pawn.png");


		//just to make it easier to map it to the c++ object
		for (int i = 1; i < 8; i++) {
			TIconsTextures[i] = TIconsTextures[0];
			TIconsTextures[i + 16] = TIconsTextures[16];
		}

		TIconsTextures[8].init(this, "textures/Icon/Black/Rook.png");
		TIconsTextures[15] = TIconsTextures[8];
		TIconsTextures[9].init(this, "textures/Icon/Black/Knight.png");
		TIconsTextures[14] = TIconsTextures[9];
		TIconsTextures[10].init(this, "textures/Icon/Black/Bishop.png");
		TIconsTextures[13] = TIconsTextures[10];
		TIconsTextures[11].init(this, "textures/Icon/Black/Queen.png");
		TIconsTextures[12].init(this, "textures/Icon/Black/King.png");

		TIconsTextures[24].init(this, "textures/Icon/White/Rook.png");
		TIconsTextures[31] = TIconsTextures[24];


		TIconsTextures[25].init(this, "textures/Icon/White/Knight.png");
		TIconsTextures[30] = TIconsTextures[25];

		TIconsTextures[26].init(this, "textures/Icon/White/Bishop.png");
		TIconsTextures[29] = TIconsTextures[26];


		TIconsTextures[27].init(this, "textures/Icon/White/Queen.png");
		TIconsTextures[28].init(this, "textures/Icon/White/King.png");




		// Init local variables
		CamH = 0.0f;
		CamRadius = 5.0f;
		CamPitch = glm::radians(15.f);
		CamYaw = glm::radians(30.f);
		gameState = 0;


		// Init the chess pieces
		//pawns

		/*
		-3.5f,0,-3.5f    |   4,5f,0.0f,-3.5f

		-3.5f,0,3.5f	 |   4.5f,0,3.5f
		*/

		/*init the icons (projection of the pieces on the xz plane)*/
#define LOWER_PROJECTION_HEIGHT -4.0f

		for (int i = 0; i < 8; i++) {
			pieceProjections[i] = new Icon(glm::vec3(i - 3.5f, LOWER_PROJECTION_HEIGHT, -3.5f + 1.0f), &MIconSquareModel, "WhitePawn");
			pieceProjections[i + 16] = new Icon(glm::vec3(i - 3.5f, LOWER_PROJECTION_HEIGHT, 3.5f - 1.0f), &MIconSquareModel, "BlackPawn");
		}
		pieceProjections[8] = new Icon(glm::vec3(-3.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteRook");
		pieceProjections[9] = new Icon(glm::vec3(-2.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteKnight");
		pieceProjections[10] = new Icon(glm::vec3(-1.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteBishop");
		pieceProjections[11] = new Icon(glm::vec3(-0.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteQueen");
		pieceProjections[12] = new Icon(glm::vec3(0.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteKing");
		pieceProjections[13] = new Icon(glm::vec3(1.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteBishop");
		pieceProjections[14] = new Icon(glm::vec3(2.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteKnight");
		pieceProjections[15] = new Icon(glm::vec3(3.5f, LOWER_PROJECTION_HEIGHT, -3.5f), &MIconSquareModel, "WhiteRook");

		pieceProjections[24] = new Icon(glm::vec3(-3.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackRook");
		pieceProjections[25] = new Icon(glm::vec3(-2.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackKnight");
		pieceProjections[26] = new Icon(glm::vec3(-1.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackBishop");
		pieceProjections[27] = new Icon(glm::vec3(-0.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackQueen");
		pieceProjections[28] = new Icon(glm::vec3(0.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackKing");
		pieceProjections[29] = new Icon(glm::vec3(1.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackBishop");
		pieceProjections[30] = new Icon(glm::vec3(2.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackKnight");
		pieceProjections[31] = new Icon(glm::vec3(3.5f, LOWER_PROJECTION_HEIGHT, 3.5f), &MIconSquareModel, "BlackRook");




		for (int i = 0; i < 8; i++) {
			chessPieces[i] = new ChessPiece(glm::vec3(i - 3.5f, 0.5f, -3.5f + 1.0f), &MPawn, &TWhitePawn, true, 1, i, "WhitePawn");
			chessPieces[i + 16] = new ChessPiece(glm::vec3(i - 3.5f, 0.5f, 3.5f - 1.0f), &MPawn, &TBlackPawn, false, 6, i, "BlackPawn");
		}

		/*allocated on the heap*/

		//white pieces
		chessPieces[8] = new ChessPiece(glm::vec3(-3.5f, 0.5f, -3.5f), &MRook, &TWhiteRook, true, 0, 0, "WhiteRook");
		chessPieces[9] = new ChessPiece(glm::vec3(-2.5f, 0.5f, -3.5f), &MKnight, &TWhiteKnight, true, 0, 1, "WhiteKnight");
		chessPieces[10] = new ChessPiece(glm::vec3(-1.5f, 0.5f, -3.5f), &MBishop, &TWhiteBishop, true, 0, 2, "WhiteBishop");
		chessPieces[11] = new ChessPiece(glm::vec3(-0.5f, 0.5f, -3.5f), &MKing, &TWhiteKing, true, 0, 3, "WhiteKing");
		chessPieces[12] = new ChessPiece(glm::vec3(0.5f, 0.5f, -3.5f), &MQueen, &TWhiteQueen, true, 0, 4, "WhiteQueen");
		chessPieces[13] = new ChessPiece(glm::vec3(1.5f, 0.5f, -3.5f), &MBishop, &TWhiteBishop, true, 0, 5, "WhiteBishop");
		chessPieces[14] = new ChessPiece(glm::vec3(2.5f, 0.5f, -3.5f), &MKnight, &TWhiteKnight, true, 0, 6, "WhiteKnight");
		chessPieces[15] = new ChessPiece(glm::vec3(3.5f, 0.5f, -3.5f), &MRook, &TWhiteRook, true, 0, 7, "WhiteRook");

		//black pieces	   
		chessPieces[24] = new ChessPiece(glm::vec3(-3.5f, 0.5f, 3.5f), &MRook, &TBlackRook, false, 7, 0, "BlackRook");
		chessPieces[25] = new ChessPiece(glm::vec3(-2.5f, 0.5f, 3.5f), &MKnight, &TBlackKnight, false, 7, 1, "BlackKnight");
		chessPieces[26] = new ChessPiece(glm::vec3(-1.5f, 0.5f, 3.5f), &MBishop, &TBlackBishop, false, 7, 2, "BlackBishop");
		chessPieces[27] = new ChessPiece(glm::vec3(-0.5f, 0.5f, 3.5f), &MKing, &TBlackKing, false, 7, 3, "BlackKing");
		chessPieces[28] = new ChessPiece(glm::vec3(0.5f, 0.5f, 3.5f), &MQueen, &TBlackQueen, false, 7, 4, "BlackQueen");
		chessPieces[29] = new ChessPiece(glm::vec3(1.5f, 0.5f, 3.5f), &MBishop, &TBlackBishop, false, 7, 5, "BlackBishop");
		chessPieces[30] = new ChessPiece(glm::vec3(2.5f, 0.5f, 3.5f), &MKnight, &TBlackKnight, false, 7, 6, "BlackKnight");
		chessPieces[31] = new ChessPiece(glm::vec3(3.5f, 0.5f, 3.5f), &MRook, &TBlackRook, false, 7, 7, "BlackRook");

		/*default selected piece is the white queen*/
		ChessBoardInstance::chessBoard.setSelectedPiece(chessPieces[11]);

		ChessBoardInstance::chessBoard.printChessBoard();



		for (int i = 0; i < 32; i++) {
			chessPieces[i]->bindIconProjection(pieceProjections[i]);
		}


		//set scrollback callback function
		glfwSetScrollCallback(window, scrollCallback);

		glfwSetKeyCallback(window, keyCallback);

		std::cout << "Init done" << std::endl;
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();
		POverlay.create();
		/* A16 */
		/* Create the new pipeline */
		PChessboard.create();
		PChesspiece.create();
		PIcons.create();

		DSChessboard.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TChessboard}
		});
			
		
		for (int i = 0; i < 32; i++)
			pieceProjections[i]->initDescriptorSet(this, &DSLMesh, &TIconsTextures[i]);


		for (int i = 0; i < 32; i++) {
			chessPieces[i]->initDescriptorSet(this, &DSLMesh);
			//std::cout << "Descriptor set for " << i << " created" << std::endl;
		}

		DSGubo.init(this, &DSLGubo, {
					{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
			});

		std::cout << "Pipelines and descriptor sets created" << std::endl;
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMesh.cleanup();
		POverlay.cleanup();
		/* A16 */
		/* cleanup the new pipeline */
		PChessboard.cleanup();
		PChesspiece.cleanup();
		PIcons.cleanup();

		// Cleanup datasets

		DSIcons.cleanup();

		/* A16 */
		/* cleanup the dataset for the room */
		DSChessboard.cleanup();

		for (int i = 0; i < 32; i++) {
			pieceProjections[i]->cleanupDescriptorSet();
			chessPieces[i]->cleanupDescriptorSet();
		}

		DSGubo.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures

		TWhitePawn.cleanup();
		TWhiteRook.cleanup();
		TWhiteKnight.cleanup();
		TWhiteBishop.cleanup();
		TWhiteQueen.cleanup();
		TWhiteKing.cleanup();

		TBlackPawn.cleanup();
		TBlackRook.cleanup();
		TBlackKnight.cleanup();
		TBlackBishop.cleanup();
		TBlackQueen.cleanup();
		TBlackKing.cleanup();

		TChessboard.cleanup();

		// Cleanup models
		MBody.cleanup();
		MHandle.cleanup();
		MWheel.cleanup();
		MKey.cleanup();
		MSplash.cleanup();

		MIconSquareModel.cleanup();


		/* A16 */
		/* Cleanup the mesh for the room */
		MChessboard.cleanup();
		MPawn.cleanup();
		MRook.cleanup();
		MKnight.cleanup();
		MBishop.cleanup();
		MQueen.cleanup();
		MKing.cleanup();

		// Cleanup descriptor set layouts
		DSLMesh.cleanup();
		DSLOverlay.cleanup();
		/* A16 */
		/* Cleanup the new Descriptor Set Layout */
		DSLVColor.cleanup();

		DSLGubo.cleanup();

		// Destroies the pipelines
		PMesh.destroy();
		POverlay.destroy();
		/* A16 */
		/* Destroy the new pipeline */
		PChessboard.destroy();
		PChesspiece.destroy();
		PIcons.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// sets global uniforms (see below for parameters explanation)
		DSGubo.bind(commandBuffer, PMesh, 0, currentImage);

		// binds the pipeline
		PMesh.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the model


		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter

		// binds the descriptor set
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter

		// record the drawing command in the command buffer 
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.




		
		/* A16 */
		/* Insert the commands to draw the room */
		


		PChessboard.bind(commandBuffer);
		MChessboard.bind(commandBuffer);
		DSChessboard.bind(commandBuffer, PChessboard, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MChessboard.indices.size()), 1, 0, 0, 0);

		PChesspiece.bind(commandBuffer);
		for (int i = 0; i < 32; i++) {
			chessPieces[i]->cmdBuffFill(commandBuffer, currentImage, PChesspiece);
		}

		PIcons.bind(commandBuffer);
		for (int i = 0; i < 32; i++) {
			pieceProjections[i]->cmdBuffFill(commandBuffer, currentImage, PIcons);
		}

		


		std::cout << "Command buffer populated" << std::endl;
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.

#define SKIP_N_FRAMES_CONSTANT 5 

	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire, choosingPieceDirection, enterPressed);

#define W GLFW_KEY_W
#define A GLFW_KEY_A
#define S GLFW_KEY_S
#define D GLFW_KEY_D
#define F GLFW_KEY_F
#define R GLFW_KEY_R



		if (keysMap[W] && keysMap[A]) {
			choosingPieceDirection = DIRECTION::NORTHWEST;
		}
		else if (keysMap[W] && keysMap[D]) {
			choosingPieceDirection = DIRECTION::NORTHEAST;
		}
		else if (keysMap[S] && keysMap[D]) {
			choosingPieceDirection = DIRECTION::SOUTHEAST;
		}
		else if (keysMap[S] && keysMap[A]) {
			choosingPieceDirection = DIRECTION::SOUTHWEST;
		}
		else if (keysMap[W]) {
			choosingPieceDirection = DIRECTION::NORTH;
		}
		else if (keysMap[A]) {
			choosingPieceDirection = DIRECTION::WEST;
		}
		else if (keysMap[S]) {
			choosingPieceDirection = DIRECTION::SOUTH;
		}
		else if (keysMap[D]) {
			choosingPieceDirection = DIRECTION::EAST;
		}
#define MIN_VERT_MOVEMENT 1.0f
		else if (keysMap[F]) {
			ChessBoardInstance::chessBoard.moveSelectedPieceY(MIN_VERT_MOVEMENT);
			keysMap[F] = 0;
		}
		else if (keysMap[R]) {
			ChessBoardInstance::chessBoard.moveSelectedPieceY(-MIN_VERT_MOVEMENT);
			keysMap[R] = 0;

		}



		if (enterPressed && !selectingNewPosition)

		{
			printf("Confirmed selection of piece [%s]\n", ChessBoardInstance::chessBoard.getCurrentlySelectedPiece()->getName());
			selectingNewPosition = true;
			enterPressed = false;
		}

		if (selectingNewPosition && enterPressed)  //move the piece to the new spot
		{
			//move the piece
			printf("Moving piece %s", ChessBoardInstance::chessBoard.getCurrentlySelectedPiece()->getName());
			ChessBoardInstance::chessBoard.moveSelectedPiece(selectingSpot);
			selectingNewPosition = enterPressed = false;
		}

		if (choosingPieceDirection >= BaseProject::DIRECTION::NORTH && choosingPieceDirection <= BaseProject::DIRECTION::SOUTHWEST) {
			frameSinceKeysPolled++;

			if (frameSinceKeysPolled != SKIP_N_FRAMES_CONSTANT) goto skipFrame;
			frameSinceKeysPolled = 0;
			for (const auto& pair : keysMap)
				keysMap[pair.first] = 0;
			if (selectingNewPosition) //we're choosing a place for the currently selected piece
			{
				switch (choosingPieceDirection) {
				case  BaseProject::DIRECTION::NORTH:
					selectingSpot.row = (selectingSpot.row - 1 < 0 ? 0 : selectingSpot.row - 1);
					break;
				case  BaseProject::DIRECTION::SOUTH:
					selectingSpot.row = (selectingSpot.row + 1 > 7 ? 7 : selectingSpot.row + 1);
					break;
				case  BaseProject::DIRECTION::WEST:
					selectingSpot.col = (selectingSpot.col - 1 < 0 ? 0 : selectingSpot.col - 1);
					break;
				case  BaseProject::DIRECTION::EAST:
					selectingSpot.col = (selectingSpot.col + 1 > 7 ? 7 : selectingSpot.col + 1);
					break;
				}
				printf("New selecting spot position (%d,%d)\n", selectingSpot.row, selectingSpot.col);
			}
			else { //we're choosing a piece still
				ChessBoardInstance::chessBoard.getNextPiece(choosingPieceDirection);
				printf("Selected [%s]\n", ChessBoardInstance::chessBoard.getCurrentlySelectedPiece()->getName());
				selectingSpot = ChessBoardInstance::chessBoard.getCurrentlySelectedPiece()->getPos();
			}
		}
	skipFrame:
		if (enterPressed) enterPressed = false;
		choosingPieceDirection = INVALID;

		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// To debounce the pressing of the fire button, and start the event when the key is released
		static bool wasFire = false;
		bool handleFire = (wasFire && (!fire));
		wasFire = fire;


		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(30.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;
		const float rotSpeed = glm::radians(90.0f);
		const float movSpeed = 1.0f;

		static float dist = 2.0f;




		CamH += m.z * movSpeed * deltaT;
		CamRadius -= (m.x * dist/100) * movSpeed * deltaT; //To be finetuned since different mouses produce different values of m.x
		CamPitch -= r.x * rotSpeed * deltaT;
		CamYaw += r.y * rotSpeed * deltaT;


		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;
		glm::vec3 camTarget = glm::vec3(0, CamH, 0);


		glm::vec3 camPos = camTarget +
			CamRadius * glm::vec3(cos(CamPitch) * sin(CamYaw),
				sin(CamPitch),
				cos(CamPitch) * cos(CamYaw));

		dist = length(camPos - glm::vec3(0, 0, 0));

		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));

		gubo.DlightDir = glm::normalize(glm::vec3(1, 2, 3));
		gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.AmbLightColor = glm::vec3(0.1f);
		gubo.eyePos = camPos;

		//gubo.spotLightDir = glm::normalize(glm::vec3(1, 2, 3));
		//gubo.spotLightPos = glm::vec3(0.0f, 5.0f, 0.0f);
		//gubo.spotLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		//gubo.spotLight_eyePos = camPos;


		////test spotlight
		//float dang = CamPitch + glm::radians(15.0f);
		//gubo.spotLightPos = camPos + glm::vec3(0, 1, 0);
		//gubo.spotLightDir = glm::vec3(cos(dang) * sin(CamYaw), sin(dang), cos(dang) * cos(CamYaw));
		//gubo.spotLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		//gubo.eyePos = camPos;

		glm::mat4 World;

		// Writes value to the GPU
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);

		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		for (int i = 0; i < 32; i++) {
			pieceProjections[i]->uboMapping(Prj, View, currentImage);
		}


		/* A16 */
		/* fill the uniform block for the room. Identical to the one of the body of the slot machine */
		World = glm::scale(glm::mat4(1), glm::vec3(4.001f, 4.001f, 4.001f)) * glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
		uboChessboard.amb = 1.0f; uboChessboard.gamma = 180.0f; uboChessboard.sColor = glm::vec3(1.0f);
		uboChessboard.mvpMat = Prj * View * World;
		uboChessboard.mMat = World;
		uboChessboard.nMat = glm::inverse(glm::transpose(World));

		/* map the uniform data block to the GPU */
		DSChessboard.map(currentImage, &uboChessboard, sizeof(uboChessboard), 0);

		for (int i = 0; i < 32; i++) {
			chessPieces[i]->uboMapping(Prj, View, currentImage);
		}





	}
};


// This is the main: probably you do not need to touch this!
int main() {
	A16 app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}