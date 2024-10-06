#include <cstdio>
#include "pack.h"
#include <SFML\Graphics.hpp>
#include <vector>
#include <boost/filesystem.hpp>
#include <iostream>
#include "nlohmann\json.hpp"
#include <fstream>

using namespace std;
using namespace sf;
using namespace nlohmann;


#define MAX_RECTS 200
#define ALLOW_FLIP false

int main() {
	rect_xywhf rects[MAX_RECTS], *ptr_rects[MAX_RECTS];
	sf::Texture *rectTextures[MAX_RECTS];

	cout << "please enter the folder name to be used: " << endl;
	string folderName;
	cin >> folderName;
	cout << "please enter the spritesheet name to be used: " << endl;
	string ssName;
	cin >> ssName;

	std::map<std::string, sf::Texture*> textureMap;
	
	json myjson;

	int inputTest = 0;
	boost::filesystem::path p("Input");
	if (!boost::filesystem::exists(p))
	{
		cout << "input folder doesn't exist!" << endl;
		cin >> inputTest;
		return 0;
	}

	std::vector<boost::filesystem::path> v;
	copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(v));
	for (auto it = v.begin(); it != v.end(); ++it)
	{
		assert(boost::filesystem::is_regular_file((*it)));

		sf::Texture *tex = new Texture;
		if (!tex->loadFromFile((*it).string()))
		{
			cout << "couldnt load file: " << (*it).string() << endl;
			cin >> inputTest;
			return 0;
		}

		textureMap[(*it).filename().string()] = tex;
		//textureMap[(*it).filename(), ]
	}

	int rectIndex = 0;
	for (auto it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		rects[rectIndex] = rect_xywhf(0, 0, (*it).second->getSize().x, (*it).second->getSize().y);
		rects[rectIndex].tex = (*it).second;
		rects[rectIndex].texName = (*it).first;
		ptr_rects[rectIndex] = rects + rectIndex;
		++rectIndex;
	}

	int maxSide = 2048;

	int numRects = textureMap.size();

	vector<bin> bins;

	if (pack(ptr_rects, numRects, maxSide, ALLOW_FLIP, bins)) {
		printf("bins: %d\n", bins.size());

		for (int i = 0; i < bins.size(); ++i) {
			printf("\n\nbin: %dx%d, rects: %d\n", bins[i].size.w, bins[i].size.h, bins[i].rects.size());

			for (int r = 0; r < bins[i].rects.size(); ++r) {
				rect_xywhf* rect = bins[i].rects[r];

				printf("rect %d: x: %d, y: %d, w: %d, h: %d, was flipped: %s\n", r, rect->x, rect->y, rect->w, rect->h, rect->flipped ? "yes" : " no");
			}
		}
	}
	else {
		printf("failed: there's a rectangle with width/height bigger than max_size!\n");
	}

	cout << endl;

	Color tempColor;
	std::vector<std::string> outputNames;
	outputNames.resize(bins.size());
	for (int i = 0; i < bins.size(); ++i)
	{
		cout << "processing bin image " << i << "\n";
		sf::Image outputImage;
		outputImage.create(bins[i].size.w, bins[i].size.h, Color::White);

		for (int j = 0; j < bins[i].rects.size(); ++j)
		{
			rect_xywhf *currRect = bins[i].rects[j];
			sf::Image inputImage = currRect->tex->copyToImage();

			int inputW = inputImage.getSize().x;
			int inputH = inputImage.getSize().y;

			for (int x = 0; x < inputW; ++x)
			{
				for (int y = 0; y < inputH; ++y)
				{
					tempColor = inputImage.getPixel(x, y);
					outputImage.setPixel(currRect->x + x, currRect->y + y, tempColor);
				}
			}
		}

		outputNames[i] = ssName + "_spritesheet_" + to_string(i) + ".png";

		outputImage.saveToFile("Output\\" + outputNames[i]);
	}

	
	rect_xywhf *currRect = NULL;
	for (int i = 0; i < bins.size(); ++i)
	{
		for (int j = 0; j < bins[i].rects.size(); ++j)
		{
			currRect = bins[i].rects[j];
			json &myCurrJSON = myjson[folderName + "/" + currRect->texName];//myjson.emplace_back( "test" );
			myCurrJSON["texName"] = folderName + "/" + outputNames[i];
			std::vector<int> originVec;
			originVec.push_back(currRect->x);
			originVec.push_back(currRect->y);
			myCurrJSON["origin"] = originVec;
			std::vector<int> texSizeVec;
			texSizeVec.push_back(currRect->tex->getSize().x);
			texSizeVec.push_back(currRect->tex->getSize().y);
			myCurrJSON["originalTexSize"] = texSizeVec;
			//myCurrJSON["originalTexName"] = folderName + "/" + currRect->texName;
		}
	}

	ofstream test;
	test.open("Output/test.json");
	test << myjson.dump( 4 );
	test.close();

	system("pause");
	return 0;
}