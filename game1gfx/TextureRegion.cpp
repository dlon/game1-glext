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
}

void TextureRegion::updateArray(std::vector<Batch::attributeType> &vertexAttribData, int offset, GLfloat x, GLfloat y)
{
	writeVertices(vertexAttribData, offset, x, y);
	writeTexCoords(vertexAttribData, offset, x, y);
	writeColors(vertexAttribData, offset, x, y);
}
