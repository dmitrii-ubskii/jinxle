#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ncursespp/ncurses.h"

enum class Response
{
	None, Bull, Cow, Miss, Error
};

class Jinxle
{
public:
	Jinxle();

	int mainLoop();

private:
	ncurses::Ncurses context{};
	ncurses::Window window{{{3, 1}, {12, 12}}};

	std::unordered_map<Response, ncurses::Palette> letterPalettes;
	std::unordered_map<Response, std::unordered_map<Response, ncurses::Palette>> borderPalettes;

	std::vector<std::string> vocab;
	std::unordered_set<std::string> validGuesses;

	void initializeColors();
	void loadVocab();
};

