#include "jinxle.h"

#include <chrono>
#include <fstream>
#include <random>
#include <thread>

#include "DU/enum_utils.h"

auto const LeftHalfBlock = L"\x258C";

Jinxle::Jinxle()
{
	context.set_cursor_visible(true);
	initializeColors();
	loadVocab();
}

std::array<Response, 5> evaluateGuess(std::array<char, 5> const& word, std::array<char, 5> const& guess)
{
	auto response = std::array{Response::Miss, Response::Miss, Response::Miss, Response::Miss, Response::Miss};
	auto wordUsed = std::array<bool, 5>{};
	auto guessUsed = std::array<bool, 5>{};
	for (auto i = 0ul; i < 5ul; i++)
	{
		if (word[i] == guess[i])
		{
			wordUsed[i] = true;
			guessUsed[i] = true;
			response[i] = Response::Bull;
		}
	}
	for (auto i = 0ul; i < 5ul; i++)
	{
		if (not guessUsed[i])
		{
			for (auto j = 0ul; j < 5ul; j++)
			{
				if (not wordUsed[j] && guess[i] == word[j])
				{
					response[i] = Response::Cow;
					wordUsed[j] = true;
					break;
				}
			}
		}
	}
	return response;
}

int Jinxle::mainLoop()
{
	auto print = [&](std::array<char, 5> const& word, std::array<Response, 5> const& responses = {})
	{
		auto previous = Response::None;
		for (auto i = 0ul; i < 5ul; i++)
		{
			if (word[i] == '\0')
			{
				continue;
			}

			auto current = responses[i];

			window.set_palette(borderPalettes[previous][current]);
			window.addwstr(LeftHalfBlock);

			window.set_palette(letterPalettes[current]);
			window.addch(word[i]);

			previous = current;
		}
		window.set_palette(borderPalettes[previous][Response::None]);
		window.addwstr(LeftHalfBlock);
	};

	auto word = std::array<char, 5>{};
	auto guess = std::array<char, 5>{};
	auto guessNumber = 0;
	auto guessLength = 0ul;

	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<std::size_t> distribution(0, vocab.size());

	auto n = distribution(generator);
	std::copy(vocab[n].begin(), vocab[n].end(), word.begin());

	auto done = false;

	window.mvaddstr({3, 0}, "JINXLE");
	window.move_cursor(ncurses::Point{1, guessNumber + 2});

	for (auto key = window.getch(); key != ncurses::Key::Escape; key = window.getch())
	{
		if (done)
		{
			break;
		}

		if ('a' <= key && key <= 'z')
		{
			key = {key + 'A' - 'a'};
		}
		if ('A' <= key && key <= 'Z')
		{
			if (guessLength < 5)
			{
				guess[guessLength] = key.to_char();
				guessLength++;
			}
		}
		if (key == ncurses::Key::Backspace)
		{
			if (guessLength > 0)
			{
				guessLength--;
				guess[guessLength] = ' ';
			}
		}
		window.move_cursor(ncurses::Point{0, guessNumber + 2});
		print(guess);
		if (key == ncurses::Key::Enter)
		{
			if (guessLength == 5)
			{
				if (validGuesses.contains(std::string{guess.begin(), guess.end()}))
				{
					auto responses = evaluateGuess(word, guess);
					auto responsesBuffer = std::array{Response::None, Response::None, Response::None, Response::None, Response::None};
					window.move_cursor(ncurses::Point{1, guessNumber + 3});
					window.refresh();
	
					done = true;
					for (auto i = 0ul; i < 5ul; i++)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
						if (responses[i] != Response::Bull)
						{
							done = false;
						}
	
						responsesBuffer[i] = responses[i];
						window.move_cursor(ncurses::Point{0, guessNumber + 2});
						print(guess, responsesBuffer);
	
						window.set_palette(borderPalettes[Response::None][Response::None]);
						window.move_cursor(ncurses::Point{1, guessNumber + 3});
						window.refresh();
					}
	
					if (done)
					{
						window.set_palette(letterPalettes[Response::None]);
						switch (guessNumber)
						{
							case 0:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("Word in one!");
								break;

							case 1:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("2/6  Eagle!");
								break;

							case 2:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("3/6  Birdie!");
								break;

							case 3:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("4/6  Par!");
								break;

							case 4:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("5/6  Bogie!");
								break;

							case 5:
								window.move_cursor(ncurses::Point{0, guessNumber + 4});
								window.addstr("6/6  Phew!");
								break;

							default:
								break;
						}
						window.refresh();
					}
					else if (guessNumber == 5)
					{
						window.set_palette(letterPalettes[Response::None]);
						window.move_cursor(ncurses::Point{1, guessNumber + 4});
						window.addstr("Too bad!");
						window.move_cursor(ncurses::Point{1, guessNumber + 5});
						for (auto i = 0ul; i < 5; i++) { window.addch(word[i]); window.addch(' '); }
						window.refresh();

						done = true;
					}
	
					guessNumber++;
					guessLength = 0;
					guess = {};
				}
				else
				{
					window.move_cursor(ncurses::Point{0, guessNumber + 2});
					print(guess, {Response::Error, Response::Error, Response::Error, Response::Error, Response::Error});
					window.refresh();

					std::this_thread::sleep_for(std::chrono::milliseconds(200));

					window.move_cursor(ncurses::Point{0, guessNumber + 2});
					print(guess);
				}
			}
		}
		window.move_cursor(ncurses::Point{2 * static_cast<int>(guessLength) + 1, guessNumber + 2});
	}

	return 0;
}

void Jinxle::initializeColors()
{
	std::unordered_map<Response, ncurses::Color> colors = {
		{Response::None, ncurses::Color::Black},
		{Response::Bull, ncurses::Color::LightGreen},
		{Response::Cow, ncurses::Color::Yellow},
		{Response::Miss, ncurses::Color::Gray},
		{Response::Error, ncurses::Color::Red}
	};

	short counter = 1;

	// letters, i.e. square middles
	for (auto response: DU::enumValues<Response>())
	{
		auto palette = ncurses::Palette{counter++};
		auto foreground = ncurses::Color::Black;
		if (response == Response::None)
		{
			foreground = ncurses::Color::White;
		}
		context.register_palette(palette, foreground, colors[response]);
		letterPalettes[response] = palette;
	}

	// square borders
	for (auto leftResponse: DU::enumValues<Response>())
	{
		for (auto rightresponse: DU::enumValues<Response>())
		{
			auto palette = ncurses::Palette{counter++};
			context.register_palette(palette, colors[leftResponse], colors[rightresponse]);
			borderPalettes[leftResponse][rightresponse] = palette;
		}
	}
}

void Jinxle::loadVocab()
{
	auto capitalize = [](std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), [](auto c){ return std::toupper(c); });
	};
	
	std::string word;
	{
		auto stream = std::ifstream{"data/vocab.txt"};
		while (std::getline(stream, word))
		{
			capitalize(word);
			vocab.push_back(word);
			validGuesses.insert(word);
		}
	}

	{
		auto stream = std::ifstream{"data/valid_guesses.txt"};
		while (std::getline(stream, word))
		{
			capitalize(word);
			validGuesses.insert(word);
		}
	}
}
