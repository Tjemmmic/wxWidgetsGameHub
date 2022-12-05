#pragma once
namespace window
{
	enum id
	{
		MAINWINDOW = wxID_HIGHEST + 1,
		GAMESWINDOW = wxID_HIGHEST + 2,
		MINEGAMEWINDOW = wxID_HIGHEST + 3,
		SPLITTERPRIMARY = wxID_HIGHEST + 4,
		SPLITTERSECONDARY = wxID_HIGHEST + 5,
		SPLITTERTHIRD = wxID_HIGHEST + 6
	};
}

namespace menu
{
	enum id
	{
		MENUQUIT = wxID_HIGHEST + 101,
		MENUNEW = wxID_HIGHEST + 102,
		MINEGAMEID = wxID_HIGHEST + 103,
		TTTGAMEID = wxID_HIGHEST + 104
	};
}

namespace button
{				
	enum id
	{
		BUTTON_ID1 = wxID_HIGHEST + 201,
		MINEGAMEPLAY = wxID_HIGHEST + 202,
		MINEGAMECLOSE = wxID_HIGHEST + 203,
		GAMESHIDETOGGLE = wxID_HIGHEST + 204,
		TTTGAMEPLAY = wxID_HIGHEST + 205,
		TTTGAMECLOSE = wxID_HIGHEST + 206,
		TTTDIFFSLIDER = wxID_HIGHEST + 207
	};
	// Buttons for Mystery Mines start at wxID_HIGHEST + 1200
	// Buttons for Tic-Tac-Toe start at wxID_HIGHEST + 1190 -> There are 9 buttons here
}

namespace panel
{
	enum id
	{
		MINEGAMEPANEL = wxID_HIGHEST + 301,
		TTTGAMEPANEL = wxID_HIGHEST + 302
	};
}

namespace image
{
	enum id
	{
		MINEGAMETITLE = wxID_HIGHEST + 401,
		TTTGAMETITLE = wxID_HIGHEST + 402
	};
}

namespace text
{
	enum id
	{
		TTTDIFFICULTYTEXT = wxID_HIGHEST + 501
	};
}