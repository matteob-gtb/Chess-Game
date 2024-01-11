#pragma once

#include <glm/glm.hpp>

#include "Starter.hpp"
#include "Structures.hpp"





class GraphicObject {
	virtual void initDescriptorSet(BaseProject* bp, DescriptorSetLayout* DSLVColor_p) {}
	virtual void initDescriptorSet(BaseProject* bp, DescriptorSetLayout* DSLMesh, Texture* iconTexture) {}
	virtual void cleanupDescriptorSet() {}
	virtual void cmdBuffFill(VkCommandBuffer& commandBuffer, int currentImage, Pipeline pipeline) {}
	virtual void uboMapping(glm::mat4& Prj, glm::mat4& View, int currentImage) {}
};


static int nIcons = 0;

class Icon : public GraphicObject {
private:

	friend class ChessPiece;
	int instanceID = nIcons++;
	Model<VertexMesh>* iconModel;
	DescriptorSet  iconDescriptorSet;
	MeshUniformBlock meshUniformBlock;
	const char* fullPieceName;
	Texture* texture;
	glm::vec3 position;
public:
	Icon() {};
	Icon(glm::vec3 pos3d, Model<VertexMesh>* iconModel, const char* name) : position(pos3d), iconModel(iconModel), fullPieceName(name) {};



	void initDescriptorSet(BaseProject* bp, DescriptorSetLayout* DSLMesh, Texture* iconTexture) {
		iconDescriptorSet.init(bp, DSLMesh, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
				{1, TEXTURE, 0, iconTexture}
			});
		this->texture = iconTexture;
	}

	void cleanupDescriptorSet() { iconDescriptorSet.cleanup(); }

	void cmdBuffFill(VkCommandBuffer& commandBuffer, int currentImage, Pipeline pipeline) {
		iconModel->bind(commandBuffer);
		iconDescriptorSet.bind(commandBuffer, pipeline, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(iconModel->indices.size()), 1, 0, 0, 0);
		printf("Texture %p\n", texture);
	}

	void uboMapping(glm::mat4& Prj, glm::mat4& View, int currentImage) {

		glm::mat4 World = glm::translate(glm::mat4(1), position) *
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		meshUniformBlock.amb = 1.0f;
		meshUniformBlock.gamma = 180.0f;
		meshUniformBlock.sColor = glm::vec3(1.0f);
		meshUniformBlock.mvpMat = Prj * View * World;
		meshUniformBlock.mMat = World;
		meshUniformBlock.nMat = glm::inverse(glm::transpose(World));
		iconDescriptorSet.map(currentImage, &meshUniformBlock, sizeof(meshUniformBlock), 0);


	}

};


class ChessBoard;
class Icon;

struct Position2D {
	int row, col;
};

static int pieceCounter;

// The structure of a chess piece which contains both the logic for the single chess piece movements 
// and the graphics initialization for it
class ChessPiece : public GraphicObject {
private:
	bool isWhite;
	glm::vec3 position;
	float rotation;
	Model<VertexMesh>* mesh;
	Texture* texture;
	DescriptorSet descriptorSet;
	MeshUniformBlock meshUniformBlock;
	int rowPos, colPos; //not 100% needed
	const char* fullyQualifiedName;
	Icon* pieceProjection;



public:
	int UUID;

	ChessPiece();
	ChessPiece(glm::vec3 position, Model<VertexMesh>* mesh, Texture* texture, bool isWhite, int row, int col, const char* name);
	//~ChessPiece();


	void bindIconProjection(Icon* projection) {
		this->pieceProjection = projection;
		//this->pieceProjection->projectionOf = this;
	}

	bool getColor() { return isWhite; }

	Position2D getPos();


	void printWorldPos() {
		printf("Piece %s position [%f %f %f]\n", fullyQualifiedName, position.x, position.y, position.z);
	}

	int getX();
	int getY();
	void move(Position2D position);
	void moveY(float upOrDown);
	void initDescriptorSet(BaseProject* bp, DescriptorSetLayout* DSLVColor_p);
	void cleanupDescriptorSet();

	void cmdBuffFill(VkCommandBuffer& commandBuffer, int currentImage, Pipeline pizpeline);

	void uboMapping(glm::mat4& Prj, glm::mat4& View, int currentImage);

	const char* getName() {
		return fullyQualifiedName;
	}

	char* to_string() {
		char* s = new char[100];
		snprintf(s, sizeof(s),
			"(%d,%d)\n", rowPos, colPos);
		return s;
	}


};
//this namespace holds all the references to the variables that might be needed during the game 





class ChessBoard {

private:
	ChessPiece* pieces[8][8];
	ChessPiece* currentlySelectedPiece;
public:
	ChessBoard() {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				pieces[i][j] = nullptr;
			}
		}
	}

	inline bool checkBounds(int x, int y) {
		return x >= 0 && x < 8 && y >= 0 && y < 8;
	}



	static glm::vec3 mapMatrixPosToWorldPos(Position2D& pos) {
		//dimensione di un quadrato 8x8

		glm::vec3 result = { pos.row * 1.0f - 3.5f,0.0f,pos.col * 1.0f - 3.5f };

		return result;
	}

	/* coordinate scacchiera dati i quattro angoli
		-3.5f,0,-3.5f    |   4,5f,0.0f,-3.5f

		-3.5f,0,3.5f	 |   4.5f,0,3.5f
	*/
	void moveSelectedPiece(Position2D newPosition) { //position expressed in x,y in the matrix
		Position2D oldPosition = currentlySelectedPiece->getPos();
		/* if not careful in checking the legality of the moves we can lose some pointers doing this*/

		if (pieces[newPosition.row][newPosition.col]) throw "Shouldn't happen at this stage yet";
		pieces[newPosition.row][newPosition.col] = pieces[oldPosition.row][oldPosition.col];
		pieces[oldPosition.row][oldPosition.col] = nullptr;
		currentlySelectedPiece->move(newPosition);
		currentlySelectedPiece->printWorldPos();

	}

	void moveSelectedPieceY(float upOrDown) { //position expressed in x,y in the matrix

		currentlySelectedPiece->moveY(upOrDown);

	}


	void setSelectedPiece(ChessPiece* piece) {
		this->currentlySelectedPiece = piece;
	}

	void getNextPiece(BaseProject::DIRECTION chosenDir) {
		Position2D currentPosition = currentlySelectedPiece->getPos();
		int x = currentPosition.row, y = currentPosition.col;
		const char* enumNames[] = { "NORTH"  , "SOUTH"  , "EAST" , "WEST"  , "NORTHEAST"  ,"NORTHWEST"  ,"SOUTHEAST"  ,"SOUTHWEST"  ,"INVALID" };

		printf("Moving [%s]\n", enumNames[chosenDir - 1]);

		switch (chosenDir) {
		case  BaseProject::DIRECTION::NORTH:
			for (int rowN = currentPosition.row - 1; rowN >= 0; rowN--) {
				if (pieces[rowN][y]) {
					currentlySelectedPiece = pieces[rowN][y];
					break;
				}
			}
			break;
		case  BaseProject::DIRECTION::SOUTH:
			for (int rowN = currentPosition.row + 1; rowN <= 7; rowN++) {
				if (pieces[rowN][y]) {
					currentlySelectedPiece = pieces[rowN][y];
					break;
				}
			}
			break;
		case  BaseProject::DIRECTION::WEST:
			for (int colN = currentPosition.col - 1; colN >= 0; colN--) {
				if (pieces[x][colN]) {
					currentlySelectedPiece = pieces[x][colN];
					break;
				}
			}
			break;
		case  BaseProject::DIRECTION::EAST:
			for (int colN = currentPosition.col + 1; colN <= 7; colN++) {
				printf("rowN %d\n", colN);
				if (pieces[x][colN]) {
					currentlySelectedPiece = pieces[x][colN];
					break;
				}
			}
			break;

		case  BaseProject::DIRECTION::NORTHWEST:
			for (int rowN = x - 1, colN = y - 1; rowN >= 0 && colN >= 0; rowN--, colN--) {
				if (pieces[rowN][colN]) {
					currentlySelectedPiece = pieces[rowN][colN];
					break;
				}
			}
			break;

		case  BaseProject::DIRECTION::NORTHEAST:
			for (int rowN = x - 1, colN = y + 1; rowN >= 0 && colN <= 7; rowN--, colN++) {
				if (pieces[rowN][colN]) {
					currentlySelectedPiece = pieces[rowN][colN];
					break;
				}
			}
			break;

		case  BaseProject::DIRECTION::SOUTHEAST:
			for (int rowN = x + 1, colN = y + 1; rowN <= 7 && colN <= 7; rowN++, colN++) {
				if (pieces[rowN][colN]) {
					currentlySelectedPiece = pieces[rowN][colN];
					break;
				}
			}
			break;
		case  BaseProject::DIRECTION::SOUTHWEST:
			for (int rowN = x + 1, colN = y - 1; rowN <= 7 && colN >= 0; rowN++, colN--) {
				if (pieces[rowN][colN]) {
					currentlySelectedPiece = pieces[rowN][colN];
					break;
				}
			}
			break;
		}
	}




	//DEBUG
	void printChessBoard() {
		printf("Called print chessboard\n");

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (pieces[i][j]) printf("%d, ", pieces[i][j]->UUID);
				else printf("O ");
			}
			printf("\n");
		}


	}


	ChessPiece* getCurrentlySelectedPiece() {
		return currentlySelectedPiece;
	}

	void addPiece(int row, int col, ChessPiece* p) {
		if (!checkBounds(row, col)) throw "Chessboard index out of bounds";
		pieces[row][col] = p;
	}
};

namespace ChessBoardInstance {
	static ChessBoard chessBoard;
}

ChessPiece::ChessPiece() {
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->rotation = 0.0f;
	this->mesh = nullptr;
	this->texture = nullptr;
	this->isWhite = true;
	this->rowPos = -1;
	this->colPos = -1;
}

Position2D ChessPiece::getPos() {
	return { rowPos,colPos };
}

ChessPiece::ChessPiece(glm::vec3 position, Model<VertexMesh>* mesh, Texture* texture, bool isWhite, int row, int col, const char* name) : 
						position(position), mesh(mesh), texture(texture), isWhite(isWhite), rowPos(row), colPos(col), fullyQualifiedName(name)
{
	if (isWhite)
		this->rotation = 3.14159f;
	else
		this->rotation = 0.0f;

	this->UUID = pieceCounter;
	pieceCounter += 1;
	ChessBoardInstance::chessBoard.addPiece(row, col, this);
}

int ChessPiece::getX() {
	return rowPos;
}
int ChessPiece::getY() {
	return colPos;
}
void ChessPiece::move(Position2D pos) {
	//missing check with reference to the actual movements the piece can perform
	/*
	update world and 2d position at the same time
	*/
	printf("OLD POS [%f %f %f]\n", position.x, position.y, position.z);
	this->position = { pos.col * 1.0f - 3.5f,position.y,pos.row * 1.0f - 3.5f };
	printf("NEW POS [%f %f %f]\n", position.x, position.y, position.z);
	this->pieceProjection->position = { pos.col * 1.0f - 3.5f,this->pieceProjection->position.y,pos.row * 1.0f - 3.5f };
	this->rowPos = pos.row;
	this->colPos = pos.col;
}

void ChessPiece::moveY(float upOrDown) {
	//missing check with reference to the actual movements the piece can perform
	/*
	update world and 2d position at the same time
	*/
	printf("OLD, NEW HEIGHT [%f %f]\n", position.y, position.y + upOrDown);
	this->position.y += upOrDown;

}


void ChessPiece::initDescriptorSet(BaseProject* bp, DescriptorSetLayout* DSL) {
	descriptorSet.init(bp, DSL, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, texture}
		});
}

void ChessPiece::cleanupDescriptorSet() {
	descriptorSet.cleanup();
}

void ChessPiece::cmdBuffFill(VkCommandBuffer& commandBuffer, int currentImage, Pipeline pipeline) {
	mesh->bind(commandBuffer);
	descriptorSet.bind(commandBuffer, pipeline, 1, currentImage);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
}


void ChessPiece::uboMapping(glm::mat4& Prj, glm::mat4& View, int currentImage) {
	glm::mat4 World = glm::translate(glm::mat4(1), position) *
		glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1), glm::vec3(0.5f, 0.5f, 0.5f));
	meshUniformBlock.amb = 1.0f;
	meshUniformBlock.gamma = 180.0f;
	meshUniformBlock.sColor = glm::vec3(1.0f);
	meshUniformBlock.mvpMat = Prj * View * World;
	meshUniformBlock.mMat = World;
	meshUniformBlock.nMat = glm::inverse(glm::transpose(World));

	descriptorSet.map(currentImage, &meshUniformBlock, sizeof(meshUniformBlock), 0);
}


