#pragma once
#include <wx/wxprec.h>
#include <wx/splitter.h>
#include "id.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <wx/notebook.h>
#include <wx/textdlg.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#if _WIN32
	#include <windows.h>
	#include <Lmcons.h>
	#include <atlstr.h>
	#include <fileapi.h>
#endif

using std::string;

class MainWindow : public wxFrame
{
public:
	MainWindow(wxWindow *parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE,
		const wxString& name = wxASCII_STR(wxFrameNameStr));
	
	~MainWindow();
public:

	int CURRENT_OS; // 0 for Windows, 1 for Linux, 2 for macOS, 3 for Other
	TCHAR uname [ UNLEN + 1]; // Stores the username if on Windows
	DWORD unameSize = UNLEN + 1; // Size of username var
	string path; // Path to scores on device

	//Main Window Menu Items
	wxMenuBar* menuBar;
	wxMenu* fileMenu;
	wxMenu* gamesMenu;

	//Main Window Status Bar Items
	wxStatusBar* winStatus;
	wxButton* statusButton;

	// Scoring
	struct minesHighscore {		// Highscore structure for mines game
		char name[15];
		int nameSize;
		int score;
	};
	struct tttHighscore {		// Highscore structure for Tic-Tac-Toe game
		int difficulty;
		int wins;
		int draws;
		int losses;
	};
	tttHighscore tttScoreBuffer[4];		// Buffer for ttt scores that are being loaded or read
	minesHighscore minesScoreBuffer[10];	// Buffer for mines game scores that are being loaded or read
	wxStaticText* tttHighscoreDisplay[4];


	//Main Window General Items
	int displayWidth; // Width of current display
	int displayHeight; // Height of current display
	wxSplitterWindow* splitterPrimary;
	int primarySashPos;
	int primarySashLimit;
	wxSplitterWindow* splitterSecondary;
	int secondarySashPos;
	int secondarySashLimit;
	wxSplitterWindow* splitterThird;
	int thirdSashPos;
	int thirdSashLimit;

	bool gamesHidden = false;
	//wxColour revertColor;
	//childWindow* gamesWindow;
	wxPanel* viewGames;
	wxPanel* viewPlay;
	wxPanel* viewConsole;
	wxGridSizer* gamesGrid;

	// Game Info Panel and Tabs
	wxPanel* viewPlaySide;		// Parent panel on splitter window for games info
	wxBoxSizer* gamesInfo;		// Sizer to contain the games info notebook
	wxNotebook* gamesNotebook;  // Notebook for games info tabs for each game
	wxWindow* minesGamePage;	// Mines game info window in notebook
	wxBoxSizer* minesInfoBoxSizer;
	wxStaticText* minesDescription;
	wxStaticText* minesHighscoreList;
	wxNotebook* minesHighscoreNotebook;
	wxWindow* minesDifficultyPage;
	wxWindow* tttGamePage;		// Tic-Tac-Toe game info window in notebook
	wxBoxSizer* tttInfoBoxSizer;
	wxStaticText* tttDescription;
	wxNotebook* tttHighscoreNotebook;
	wxWindow* tttDifficultyPages[4];
	//wxStaticText* tttHighscoreList;

	// Mystery Mines
	wxPanel* mineGamePanel;					// Panel to display the Mine game
	wxBoxSizer* mineGameSizer;				// Sizer for displaying Mystery Mines on Game selection Panel
	wxButton* mineGamePlayButton;			// Play button that opens Mystery Mines
	wxButton** mineButtons;							// Buttons for the grid of Mines and Empty spaces
	bool mineGameOpen = false;				// True if the game is open
	bool buttonFirstClick = true;				// True if no buttons have been clicked yet in the current round
	int mineGridWidth = 15;					// Width (in buttons) of grid used for game
	int mineGridHeight = 15;					// Height (in buttons) of grid used for game
	int *mineField = nullptr;					// Keeps track of which buttons have mines
	int score;								// Score for current round (how many buttons clicked before losing or winning

	// Tic-Tac-Toe -> Shortened to TTT or ttt in most cases
	wxPanel* tttGamePanel;					// Panel to display the Tic-Tac-Toe (TTT) game
	wxBoxSizer* tttGameSizer;				// Sizer for displaying TTT on Game selection panel
	wxButton* tttGamePlayButton;			// Play button that activates TTT game
	wxSlider* tttDiffSlider;
	wxStaticText* tttDifficultyText;
	wxButton **tttButtons;					// Buttons where X's and O's go in TTT
	wxBoxSizer **tttSizers;
	wxStaticBitmap **tttImages;
	wxPanel* tttButtonPanel;
	wxBoxSizer* tttButtonSizer;
	bool tttGameOpen = false;				// True if the game is open
	bool tttPlayerTurn = true;				// True if it is the user's turn
	int tttCount = 0;						// Counter for how many marks have been made in a given round of TTT
	int tttDifficulty = 1;					// Marks which difficulty is active for TTT -> 0 is Easy, 1 is Normal, 2 is Hard, and 3 is Undefeatable
	int* tttMarks = nullptr;				// Array to keep track of X's and O's
	int tttSquares = 9;						// Total number of boxes in the Tic-Tac-Toe board -> Always equals 9
	int tttDimension = 3;					// Column/Row dimension of Tic-Tac-Toe board -> Always equals 3
	int tttRows[8][3] = { {6, 4, 2}, {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 4, 8}, {2, 5, 8}, {1, 4, 7}, {0, 3, 6}, }; // Every possible three-in-a-row
	int tttCorners[4] = {0, 2, 6 ,8};		// Each of the four corners
	int tttMiddles[4] = {1, 3, 5, 7};		// Each middle-edge
	string tttDiffStrings[4] = { "Easy", "Normal", "Hard", "Undefeatable" };
	
	

private:
	bool LoadHighscores(void);
	bool WriteHighscores(void);
	void MinesAddHighscore(int score);
	void HighscoreUpdate(void);
	void OnButtonClicked(wxCommandEvent &evt);
	void OnButtonRightClicked(wxCommandEvent &evt);
	void OnMenuNew(wxCommandEvent &evt);
	void OnMenuQuit(wxCommandEvent &evt);
	void OnMineGameLaunch(wxCommandEvent &evt);
	void OnMineGameClose(wxCommandEvent &evt);
	void OnTTTGameLaunch(wxCommandEvent &evt);
	void OnTTTGameClose(wxCommandEvent &evt);
	void OnTTTClick(wxCommandEvent &evt);
	void OnTTTSliderChange(wxScrollEvent &evt);
	int tttGetButtonBorders(int button);
	bool tttWinCheck(int player);			// Checks if there is a valid win for the player passed as the argument
	int tttCheckTwo(int player);			// Checks for rows one mark from completion for the player passed as the argument
	int tttFindPlay(int player);			// Finds a move that could potentially lead to a win for the player passed as the argument
	int tttHardBlock(int player);			// Determines preemptive blocks to keep the player passed as the argument from winning in the future
	void tttAIPlay(void);					// Plays the AI's turn based on the current board and difficulty
	void OnWindowRestore(wxIconizeEvent &evt);
	void OnWindowRS(wxSizeEvent &evt);
	void OnGamesHideToggle(wxCommandEvent &evt);
	void OnPrimarySashChange(wxSplitterEvent &evt);
	void OnSecondarySashChange(wxSplitterEvent &evt);
	void OnThirdSashChange(wxSplitterEvent &evt);
	bool PlayAccessChange(bool state);
	void gamesInfoInit(void);
	

	wxDECLARE_EVENT_TABLE();
};

