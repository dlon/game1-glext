#include "TextureRegion.h"


TextureRegion::TextureRegion(const Texture& texture, int subX, int subY, int subWidth, int subHeight)
	: tx(subX), ty(subY), tw(subWidth), th(subHeight), texture(texture)
{

}


TextureRegion::~TextureRegion()
{
}

void TextureRegion::writeVertices(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	/*
	aj[8 * 0 + 0] = x + self.relativeVertices[0][0]
	aj[8 * 0 + 1] = y + self.relativeVertices[0][1]
	aj[8 * 1 + 0] = x + self.relativeVertices[1][0]
	aj[8 * 1 + 1] = y + self.relativeVertices[1][1]
	aj[8 * 2 + 0] = x + self.relativeVertices[2][0]
	aj[8 * 2 + 1] = y + self.relativeVertices[2][1]
	aj[8 * 3 + 0] = x + self.relativeVertices[3][0]
	aj[8 * 3 + 1] = y + self.relativeVertices[3][1]
	*/
	// !TODO: replace with non-fixed values
	vertexAttribData[8 * 4 * offset + 8 * 0 + 0] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 0 + 1] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 0] = 100;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 1] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 0] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 1] = 100;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 0] = 100;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 1] = 100;
}

void TextureRegion::writeTexCoords(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	/*
	nc = self.normalizedCoords
    aj[8 * 0 + 2] = nc[0]
    aj[8 * 0 + 3] = nc[1]
    aj[8 * 1 + 2] = nc[2]
    aj[8 * 1 + 3] = nc[3]
    aj[8 * 2 + 2] = nc[4]
    aj[8 * 2 + 3] = nc[5]
    aj[8 * 3 + 2] = nc[6]
    aj[8 * 3 + 3] = nc[7]
	*/
	// !TODO: replace with non-fixed values
	vertexAttribData[8 * 4 * offset + 8 * 0 + 2] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 0 + 3] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 2] = 1;
	vertexAttribData[8 * 4 * offset + 8 * 1 + 3] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 2] = 0;
	vertexAttribData[8 * 4 * offset + 8 * 2 + 3] = 1;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 2] = 1;
	vertexAttribData[8 * 4 * offset + 8 * 3 + 3] = 1;
}

void TextureRegion::writeColors(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	/*
	aj[8 * 0 + 4] = self.color[0]
    aj[8 * 0 + 5] = self.color[1]
    aj[8 * 0 + 6] = self.color[2]
    aj[8 * 0 + 7] = self.color[3]
    aj[8 * 1 + 4] = self.color[0]
    aj[8 * 1 + 5] = self.color[1]
    aj[8 * 1 + 6] = self.color[2]
    aj[8 * 1 + 7] = self.color[3]
    aj[8 * 2 + 4] = self.color[0]
    aj[8 * 2 + 5] = self.color[1]
    aj[8 * 2 + 6] = self.color[2]
    aj[8 * 2 + 7] = self.color[3]
    aj[8 * 3 + 4] = self.color[0]
    aj[8 * 3 + 5] = self.color[1]
    aj[8 * 3 + 6] = self.color[2]
    aj[8 * 3 + 7] = self.color[3]
	*/
	// !TODO: replace with non-fixed values
	for (int i = 0; i < 4; i++) {
		vertexAttribData[8 * 4 * offset + 8 * i + 4] = 1;
		vertexAttribData[8 * 4 * offset + 8 * i + 5] = 1;
		vertexAttribData[8 * 4 * offset + 8 * i + 6] = 1;
		vertexAttribData[8 * 4 * offset + 8 * i + 7] = 1;
	}
}

void TextureRegion::updateArray(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	writeVertices(vertexAttribData, offset, x, y);
	writeTexCoords(vertexAttribData, offset, x, y);
	writeColors(vertexAttribData, offset, x, y);
}
