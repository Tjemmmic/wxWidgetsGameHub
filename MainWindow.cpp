#include "MainWindow.h"
#include <wx/artprov.h>
#include <wx/imagpng.h>
#include <wx/sizer.h>
#include <wx/generic/statbmpg.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_BUTTON(button::id::BUTTON_ID1, OnButtonClicked)
	EVT_BUTTON(button::id::MINEGAMEPLAY, OnMineGameLaunch)
	EVT_BUTTON(button::id::TTTGAMEPLAY, OnTTTGameLaunch)
	EVT_BUTTON(button::id::MINEGAMECLOSE, OnMineGameClose)
	EVT_BUTTON(button::id::TTTGAMECLOSE, OnTTTGameClose)
	EVT_BUTTON(button::id::GAMESHIDETOGGLE, OnGamesHideToggle)
	EVT_MENU(wxID_NEW, OnMenuNew)
	EVT_MENU(wxID_EXIT, OnMenuQuit)
	EVT_MENU(menu::id::MINEGAMEID, OnMineGameLaunch)
	EVT_MENU(menu::id::TTTGAMEID, OnTTTGameLaunch)
	EVT_ICONIZE(OnWindowRestore)
	EVT_SIZE(OnWindowRS)
	EVT_SPLITTER_SASH_POS_CHANGED(window::id::SPLITTERPRIMARY, OnPrimarySashChange)
	EVT_SPLITTER_SASH_POS_CHANGED(window::id::SPLITTERSECONDARY, OnSecondarySashChange)
	EVT_SPLITTER_SASH_POS_CHANGED(window::id::SPLITTERTHIRD, OnThirdSashChange)
	EVT_COMMAND_SCROLL_THUMBTRACK(button::id::TTTDIFFSLIDER, OnTTTSliderChange)
	EVT_COMMAND_SCROLL_PAGEUP(button::id::TTTDIFFSLIDER, OnTTTSliderChange)
	EVT_COMMAND_SCROLL_PAGEDOWN(button::id::TTTDIFFSLIDER, OnTTTSliderChange)
	EVT_COMMAND_SCROLL_LINEUP(button::id::TTTDIFFSLIDER, OnTTTSliderChange)
	EVT_COMMAND_SCROLL_LINEDOWN(button::id::TTTDIFFSLIDER, OnTTTSliderChange)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(wxWindow *parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name):
	wxFrame(parent, id, title, pos, size, style, name)
{
	// Check And Store Current Operating System - Get Username for Windows
	#if _WIN32
		CURRENT_OS = 0;
		if (GetUserName((TCHAR*)uname, &unameSize))
		{
			DBOUT(uname)
		}
		else
		{
			_tcscpy_s(uname, _countof(uname), _T("Default"));
			DBOUT(uname)
		}
	#elif __linux__
		CURRENT_OS = 1;
	#elif __APPLE__
		CURRENT_OS = 2;
	#else
		CURRENT_OS = 3;
	#endif

	LoadHighscores(); // Either load stored highscores, or load empty if there are none yet

	//Start Window Maximimized
	Maximize();
	displayWidth;
	displayHeight;
	GetClientSize(&displayWidth, &displayHeight);
	primarySashLimit = displayWidth / 3;
	secondarySashLimit = 5 * displayHeight / 6;
	thirdSashLimit = 6 * displayWidth / 12;
	primarySashPos = primarySashLimit;
	secondarySashPos = displayHeight;
	thirdSashPos = thirdSashLimit;

	//SPLITTERS FOR SEPARATE RESIZEABLE PANELS
	splitterPrimary = new wxSplitterWindow(this, window::id::SPLITTERPRIMARY);
	splitterSecondary = new wxSplitterWindow(splitterPrimary, window::id::SPLITTERSECONDARY);
	splitterThird = new wxSplitterWindow(splitterSecondary, window::id::SPLITTERTHIRD);
	viewPlay = new wxPanel(splitterThird);
	viewPlay->SetBackgroundColour(wxColour(90, 5, 10));
	viewPlaySide = new wxPanel(splitterThird);
	viewPlaySide->SetBackgroundColour(wxColour(80, 255, 80));
	viewConsole = new wxPanel(splitterSecondary);
	viewConsole->SetBackgroundColour(wxColour(80, 80, 255));
	viewGames = new wxPanel(splitterPrimary);
	viewGames->SetBackgroundColour(wxColour(80, 80, 80));
	//splitterPrimary->SetSize(GetClientSize());
	splitterPrimary->SplitVertically(viewGames, splitterSecondary, primarySashPos);
	splitterSecondary->SplitHorizontally(splitterThird, viewConsole, secondarySashPos);
	splitterThird->SplitVertically(viewPlay, viewPlaySide, thirdSashPos);
	
	//STATUS BAR
	winStatus = new wxStatusBar(this, wxID_ANY);
	SetStatusBar(winStatus);
	winStatus->PushStatusText("\t\t\t\t Ready");
	statusButton = new wxButton(winStatus, button::id::GAMESHIDETOGGLE);
	statusButton->SetPosition(winStatus->GetClientAreaOrigin());
	statusButton->SetLabelText(_("Hide Games"));



	//MENU BAR AND ITEMS
	menuBar = new wxMenuBar();
	fileMenu = new wxMenu();
	gamesMenu = new wxMenu();
	gamesMenu->Append(menu::id::MINEGAMEID, _("Mystery Mines"));
	gamesMenu->Append(menu::id::TTTGAMEID, _("Tic-Tac-Toe"));
	//wxWidgets default wxMenuItem example
	fileMenu->Append(wxID_NEW);
	//Custom wxMenuItem
	fileMenu->Append(wxID_ANY, _("&Test\tCtrl+T"));
	fileMenu->AppendSeparator();
	//Submenu
	wxMenu* subMenu = new wxMenu();
	subMenu->Append(wxID_CUT);
	subMenu->Append(wxID_COPY);
	subMenu->Append(wxID_PASTE);
	fileMenu->AppendSubMenu(subMenu, _("SubMenu"));
	fileMenu->AppendSeparator();
	//Create a wxMenuItem directly
	wxMenuItem* quitItem = new wxMenuItem(fileMenu, wxID_EXIT);
	quitItem->SetBitmap(wxArtProvider::GetBitmap("wxART_QUIT"));
	fileMenu->Append(quitItem);

	menuBar->Append(fileMenu, _("&File"));
	menuBar->Append(gamesMenu, _("&Games"));
	SetMenuBar(menuBar);

	wxPNGHandler* pngHandler = new wxPNGHandler;
	wxImage::AddHandler(pngHandler);

	//GAME GRID PANEL ON LEFT SIDE OF SCREEN
	gamesGrid = new wxGridSizer(2, 2, 0, 0);
	viewGames->SetSizer(gamesGrid);
	//GAME ONE -> MYSTERY MINES
	mineGamePanel = new wxPanel(viewGames, panel::id::MINEGAMEPANEL);
	mineGamePanel->SetBackgroundColour(wxColour(90, 5, 10));
	mineGameSizer = new wxBoxSizer(wxVERTICAL);

	wxGenericStaticBitmap* mineTitleImage = new wxGenericStaticBitmap(mineGamePanel, wxID_ANY, wxBitmap("mysteryMines.png", wxBITMAP_TYPE_PNG));
	mineGameSizer->AddStretchSpacer(2);
	mineGameSizer->Add(mineTitleImage, 0, wxALL | wxALIGN_CENTER);
	mineGameSizer->AddStretchSpacer(6);
	mineGamePlayButton = new wxButton(mineGamePanel, button::id::MINEGAMEPLAY);
	mineGamePlayButton->SetLabelText(_("PLAY"));
	mineGameSizer->Add(mineGamePlayButton, 0, wxALL | wxALIGN_CENTER, 10);
	mineGameSizer->AddStretchSpacer();
	mineGamePanel->SetSizer(mineGameSizer);
	//GAME TWO -> TIC-TAC-TOE (referenced as ttt in this code)
	tttGamePanel = new wxPanel(viewGames, panel::id::TTTGAMEPANEL);
	tttGamePanel->SetBackgroundColour(wxColour(200, 200, 50));
	tttGameSizer = new wxBoxSizer(wxVERTICAL);
	
	wxGenericStaticBitmap* tttTitleImage = new wxGenericStaticBitmap(tttGamePanel, wxID_ANY, wxBitmap("TTT.png", wxBITMAP_TYPE_PNG));
	//tttTitleImage->SetMinSize(wxSize(200, 36));
	//titleSizer->Add(tttTitleImage, wxALL);
	tttGameSizer->AddStretchSpacer(41);
	tttGameSizer->Add(tttTitleImage, 0, wxALL | wxALIGN_CENTER);
	tttGameSizer->AddStretchSpacer(100);
	wxBoxSizer* tttDiffSizer = new wxBoxSizer(wxVERTICAL);
	tttDifficultyText = new wxStaticText(tttGamePanel, text::id::TTTDIFFICULTYTEXT, _(tttDiffStrings[tttDifficulty]), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	tttDifficultyText->SetCanFocus(false);
	tttDiffSizer->Add(tttDifficultyText, wxALL | wxALIGN_CENTER);
	tttGameSizer->Add(tttDiffSizer, 0, wxALL | wxALIGN_CENTER, 10);
	tttDiffSlider = new wxSlider(tttGamePanel, button::id::TTTDIFFSLIDER, 1, 0, 3, wxDefaultPosition, wxSize(250, 32));
	tttDiffSlider->SetLineSize(1);
	tttDiffSlider->Enable();
	tttGameSizer->Add(tttDiffSlider, 0, wxALL | wxALIGN_CENTER, 10);
	tttGamePlayButton = new wxButton(tttGamePanel, button::id::TTTGAMEPLAY);
	tttGamePlayButton->SetLabelText(_("PLAY"));
	tttGameSizer->Add(tttGamePlayButton, 0, wxALL | wxALIGN_CENTER, 10);
	tttGameSizer->AddStretchSpacer();
	tttGamePanel->SetSizer(tttGameSizer);
	

	//ADD THE GAMES TO THE GRID
	gamesGrid->Add(mineGamePanel, 1, wxALL | wxEXPAND);
	gamesGrid->Add(tttGamePanel, 1, wxALL | wxEXPAND);
	gamesGrid->Layout();

	wxListBox* gameNew = new wxListBox(viewGames, wxID_ANY);
	gameNew->AppendString("Game Coming Soon...");
	gamesGrid->Add(gameNew, 1, wxALL | wxEXPAND);


	viewGames->SetAutoLayout(true);
	viewGames->Layout();

	//GAME INFO ON RIGHT SIDE OF SCREEN
	gamesInfo = new wxBoxSizer(wxVERTICAL);
	viewPlaySide->SetSizer(gamesInfo);
	gamesNotebook = new wxNotebook(viewPlaySide, wxID_ANY);
	minesGamePage = new wxWindow(gamesNotebook, wxID_ANY);
	minesGamePage->SetBackgroundColour(*wxLIGHT_GREY);
	tttGamePage = new wxWindow(gamesNotebook, wxID_ANY);
	tttGamePage->SetBackgroundColour(*wxLIGHT_GREY);
	gamesNotebook->InsertPage(0, minesGamePage, _("Mystery Mines"));
	gamesNotebook->InsertPage(1, tttGamePage, _("Tic-Tac-Toe"));
	gamesInfo->Add(gamesNotebook, 1, wxEXPAND | wxALL);
	gamesNotebook->SetSelection(0);
	gamesInfoInit();


}

MainWindow::~MainWindow()
{
	if (mineGameOpen)		// If the window was closed while the game was open
	{						// then the pointers still need to be deleted
		delete[] mineButtons;
		delete mineField;
	}
	if (tttGameOpen)
	{
		delete[] tttButtons;
		delete tttMarks;
	}
}

void MainWindow::OnButtonClicked(wxCommandEvent &evt)
{
	//Get Which Button was clicked
	int x = (evt.GetId() - wxID_HIGHEST - 1200) % mineGridWidth;
	int y = (evt.GetId() - wxID_HIGHEST - 1200) / mineGridWidth;

	if (buttonFirstClick)
	{
		score = 0;
		int mines = 40;
		while (mines)
		{
			int rx = rand() % mineGridWidth;
			int ry = rand() % mineGridHeight;

			if (mineField[ry * mineGridWidth + rx] == 0 && rx != x && ry != y)
			{
				mineField[ry * mineGridWidth + rx] = -1;
				mines--;
			}
		}

		buttonFirstClick = false;
	}

	mineButtons[y*mineGridWidth + x]->Enable(false);
	mineButtons[y*mineGridWidth + x]->SetBackgroundColour(wxColour(75, 75, 75));

	if (mineField[y*mineGridWidth + x] == -1)
	{
		mineButtons[y*mineGridWidth + x]->SetBackgroundColour(wxColour(255, 0, 0));
		wxMessageBox(wxString::Format("BOOOOM!!! Game Over...!\nSCORE: %d", score));
		MinesAddHighscore(score);
		buttonFirstClick = true;
		score = 0;
		for (int i = 0; i < mineGridWidth; i++)
		{
			for (int j = 0; j < mineGridHeight; j++)
			{

				mineField[j*mineGridWidth + i] = 0;
				mineButtons[j*mineGridWidth + i]->SetLabel("");
				mineButtons[j*mineGridWidth + i]->Enable(true);
				mineButtons[j*mineGridWidth + i]->SetBackgroundColour(wxColour(250, 250, 250));
				//btn[j*nFieldWidth + i]->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
			}
		}
	}
	else
	{
		score++;
		int mineCount = 0;
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (x + i >= 0 && x + i < mineGridWidth && y + j >= 0 && y + j < mineGridHeight)
				{
					if (mineField[(y + j) * mineGridWidth + (x + i)] == -1)
						mineCount++;
				}
			}
		}

		if (mineCount > 0)
		{
			mineButtons[y*mineGridWidth + x]->SetLabel(std::to_string(mineCount));
		}

	}

	evt.Skip();
}

void MainWindow::OnButtonRightClicked(wxCommandEvent &evt)
{
	//Get Which Button was clicked
	int x = (evt.GetId() - wxID_HIGHEST - 1200) % mineGridWidth;
	int y = (evt.GetId() - wxID_HIGHEST - 1200) / mineGridWidth;
	if ((mineButtons[y*mineGridWidth + x]->GetLabel() == "") && mineButtons[y*mineGridWidth + x]->IsEnabled())
	{
		mineButtons[y*mineGridWidth + x]->SetLabel("X");
		mineButtons[y*mineGridWidth + x]->SetForegroundColour(wxColour(255,0,0));
	}
	else if (mineButtons[y*mineGridWidth + x]->GetLabel() == "X")
	{
		mineButtons[y*mineGridWidth + x]->SetLabel("");
		mineButtons[y*mineGridWidth + x]->SetForegroundColour(wxColour(250, 250, 250));
	}
	evt.Skip();
}

void MainWindow::OnMenuNew(wxCommandEvent &evt)
{

	evt.Skip();
}

void MainWindow::OnMenuQuit(wxCommandEvent &evt)
{
	Close();
	evt.Skip();
}

void MainWindow::OnMineGameLaunch(wxCommandEvent &evt)
{
	if (!PlayAccessChange(false)) // If there was an error in disabling, handle it
	{
		// Write it to console? Not yet implemented...
	}
	mineGameOpen = true;
	int viewPlayWidth = 0;
	int viewPlayHeight = 0;

	wxBoxSizer* gameSizer = new wxBoxSizer(wxVERTICAL);
	mineButtons = new wxButton*[mineGridWidth * mineGridHeight];

	wxBoxSizer* gameBarSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton* gameBarClose = new wxButton(viewPlay, button::id::MINEGAMECLOSE);

	wxPNGHandler* pngHandler = new wxPNGHandler;
	wxImage::AddHandler(pngHandler);
	wxGenericStaticBitmap* mysteryMinesImage = new wxGenericStaticBitmap(viewPlay, image::id::MINEGAMETITLE, wxBitmap("scaledMines.png", wxBITMAP_TYPE_PNG));


	viewPlay->GetSize(&viewPlayWidth, &viewPlayHeight);
	//gameBarSizer->AddSpacer(viewPlayWidth - 64);
	gameBarSizer->Add(mysteryMinesImage);
	gameBarClose->SetLabelText(_("CLOSE"));
	gameBarClose->SetMinSize(wxSize(64, 64));
	gameBarClose->SetMaxSize(wxSize(64, 64));
	gameBarSizer->AddStretchSpacer(400);
	gameBarSizer->Add(gameBarClose, 0,  wxALL | wxALIGN_TOP);
	
	gameSizer->Add(gameBarSizer, wxSizerFlags().Expand().Proportion(1));

	wxGridSizer *grid = new wxGridSizer(mineGridWidth, mineGridHeight, 0, 0);
	gameSizer->Add(grid, wxSizerFlags().Expand().Proportion(9));
	viewPlay->SetSizer(gameSizer);

	mineField = new int[mineGridWidth * mineGridHeight];

	for (int i = 0; i < mineGridWidth; i++)
	{
		for (int j = 0; j < mineGridHeight; j++)
		{
			mineButtons[j*mineGridWidth + i] = new wxButton(viewPlay, wxID_HIGHEST + 1200 + (j*mineGridWidth + i));
			grid->Add(mineButtons[j*mineGridWidth + i], 1, wxEXPAND | wxALL);

			mineButtons[j*mineGridWidth + i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnButtonClicked, this);
			mineButtons[j*mineGridWidth + i]->Bind(wxEVT_CONTEXT_MENU, &MainWindow::OnButtonRightClicked, this);
			mineField[j*mineGridWidth + i] = 0;
			mineButtons[j*mineGridWidth + i]->SetBackgroundColour(wxColour(250, 250, 250));
		}
	}

	grid->Layout();
	viewPlay->Layout();
	//viewPlay->Refresh();
}

void MainWindow::OnMineGameClose(wxCommandEvent &evt)
{
	WriteHighscores();
	PlayAccessChange(true);
	mineGameOpen = false;
	viewPlay->DestroyChildren();
	delete[] mineButtons;
	delete mineField;
	evt.Skip();
}

void MainWindow::OnWindowRestore(wxIconizeEvent &evt)
{

	evt.Skip();
}

void MainWindow::OnWindowRS(wxSizeEvent &evt)
{

	evt.Skip();
}

void MainWindow::OnGamesHideToggle(wxCommandEvent &evt)
{
	if (gamesHidden)
	{
		splitterPrimary->Unsplit(viewGames);												// Unsplit and Split again -
		splitterPrimary->SetSashInvisible(false);											// to make the Sash visible.
		splitterPrimary->SplitVertically(viewGames, splitterSecondary, primarySashPos);		// Move Sash back to where it was before hiding.
		viewGames->Show();
		gamesHidden = false;
	}
	else
	{
		primarySashPos = splitterPrimary->GetSashPosition();								// Save the position of Sash.
		splitterPrimary->SetSashPosition(1, true);											// Hide the Panel and make -
		splitterPrimary->SetSashInvisible(true);											// the Sash invisible.
		viewGames->Hide();
		gamesHidden = true;
	}
}

void MainWindow::OnPrimarySashChange(wxSplitterEvent &evt)
{
	if (splitterPrimary->GetSashPosition() > primarySashLimit)
	{
		splitterPrimary->SetSashPosition(primarySashLimit);
	}
	evt.Skip();
}

void MainWindow::OnSecondarySashChange(wxSplitterEvent &evt)
{
	int sashPos = splitterSecondary->GetSashPosition();
	if ((sashPos < secondarySashLimit))
	{
		splitterSecondary->SetSashPosition(secondarySashLimit);
	}
	evt.Skip();
}

void MainWindow::OnThirdSashChange(wxSplitterEvent &evt)
{
	if (splitterThird->GetSashPosition() < thirdSashLimit)
	{
		splitterThird->SetSashPosition(thirdSashLimit);
	}
	evt.Skip();
}

void MainWindow::OnTTTGameLaunch(wxCommandEvent &evt)
{
	if (!PlayAccessChange(false))
	{
		// Write to Console?
	}
	tttGameOpen = true;

	wxBoxSizer* gameSizer = new wxBoxSizer(wxVERTICAL);
	tttButtons = new wxButton*[tttSquares];
	tttSizers = new wxBoxSizer*[tttSquares];
	tttImages = new wxStaticBitmap*[tttSquares];

	tttButtonPanel = new wxPanel(viewPlay, wxID_ANY);
	tttButtonSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* gameBarSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton* gameBarClose = new wxButton(viewPlay, button::id::TTTGAMECLOSE);

	wxPNGHandler* pngHandler = new wxPNGHandler;
	wxImage::AddHandler(pngHandler);
	wxGenericStaticBitmap* mysteryMinesImage = new wxGenericStaticBitmap(viewPlay, image::id::TTTGAMETITLE, wxBitmap("scaledTTT.png", wxBITMAP_TYPE_PNG));

	gameBarSizer->Add(mysteryMinesImage);
	gameBarClose->SetLabelText(_("CLOSE"));
	gameBarClose->SetMinSize(wxSize(64, 64));
	gameBarClose->SetMaxSize(wxSize(64, 64));
	gameBarSizer->AddStretchSpacer(400);
	gameBarSizer->Add(gameBarClose, 0, wxALL | wxALIGN_TOP);
	//gameSizer->AddSpacer(10);
	gameSizer->Add(gameBarSizer, wxSizerFlags().Expand().Proportion(1));

	wxGridSizer *grid = new wxGridSizer(tttDimension, tttDimension, 0, 0);
	
	tttButtonSizer->Add(grid, wxSizerFlags().Expand().Proportion(9));
	tttButtonPanel->SetSizer(tttButtonSizer);
	tttButtonPanel->SetBackgroundColour(*wxBLUE);
	gameSizer->Add(tttButtonPanel, wxSizerFlags().Expand().Proportion(9));
	viewPlay->SetSizer(gameSizer);

	tttMarks = new int[tttSquares];

	for (int i = 0; i < tttSquares; i++)
	{
		tttMarks[i] = 0;
		tttButtons[i] = new wxButton(tttButtonPanel, wxID_HIGHEST + 1190 + i, wxEmptyString, wxDefaultPosition, wxDefaultSize);
			//tttPanels[i] = new wxPanel(viewPlay, wxID_HIGHEST + 1400 + i);
			//tttPanels[i]->SetBackgroundColour(*wxRED);
		//tttSizers[i] = new wxBoxSizer(wxVERTICAL);
		//tttImages[i] = new wxStaticBitmap(viewPlay, wxID_HIGHEST + 1400 + i, wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
		//tttImages[i]->SetBackgroundColour(*wxRED);
		//tttSizers[i]->Add(tttImages[i], wxSizerFlags());
		//tttImages[i]->Bind(wxEVT_LEFT_DCLICK, &MainWindow::OnTTTClick);
		//grid->Add(tttSizers[i], wxEXPAND | wxALL);
		//tttPanels[i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnTTTClick, this);
			//grid->Add(tttPanels[i], 1, wxEXPAND | wxALL, 5);
		//tttButtons[i]->SetBackgroundColour(wxColor(0xFFFFFF));
		tttButtons[i]->SetSize(wxSize(200, 200));
		tttButtons[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
		tttButtons[i]->SetBitmapMargins(5, 5);
		grid->Add(tttButtons[i], 1, wxEXPAND | tttGetButtonBorders(i), 20);
		tttButtons[i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainWindow::OnTTTClick, this);
		tttButtons[i]->SetBackgroundColour(wxColour(250, 250, 250));
	}

	
	//tttButtonSizer->Layout();
	//tttButtonPanel->Layout();
	//tttButtonPanel->Show();
	gameSizer->Layout();
	grid->Layout();
	viewPlay->Layout();
	//viewPlay->Refresh();
}

void MainWindow::OnTTTGameClose(wxCommandEvent &evt)
{
	WriteHighscores();
	PlayAccessChange(true);
	tttCount = 0;
	tttGameOpen = false;
	tttPlayerTurn = true;
	viewPlay->DestroyChildren();
	delete[] tttButtons;
	delete[] tttSizers;
	delete[] tttImages;
	delete tttMarks;
	evt.Skip();
}

void MainWindow::OnTTTClick(wxCommandEvent &evt)
{
	if (tttPlayerTurn)												// Do not let the user play when it is not their turn
	{
		tttPlayerTurn = false;
		int x = (evt.GetId() - wxID_HIGHEST - 1190) % tttSquares;   // Get which button was pressed
		//int x = (evt.GetId() - wxID_HIGHEST - 1400) % tttSquares;   // Get which button was pressed
		if (tttMarks[x] != 0)	// If there is already an X or an O there, nothing should happen
		{
			tttPlayerTurn = true;
			//evt.Skip();
			return;
		}
		else if (tttMarks[x] == 0)	// If there is nothing there, then put an X there
		{
			tttCount++;		// Increase the count of marks on the board by one
			tttMarks[x] = 1;	// Set current box to 1 to show that the user marked it
			//tttButtons[x]->SetLabelText(_("X"));	// Mark with X
			tttButtons[x]->SetBitmap(wxBitmap("xMarkA.png",wxBITMAP_TYPE_PNG));
			//tttImages[x]->SetBitmap(wxBitmap("xMarkA.png", wxBITMAP_TYPE_PNG));
			if (tttWinCheck(1))		// Check if user has won
			{
				wxMessageBox(_("Congratulations! You Win!!"));
				tttScoreBuffer[tttDifficulty].wins += 1;
				for (int i = 0; i < tttSquares; i++)			
				{
					tttMarks[i] = 0;
					tttButtons[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
					//tttImages[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
				}
				tttPlayerTurn = true;
				tttCount = 0;
				//evt.Skip();
				return;
			}
			if (tttCount == tttSquares)								// Check if the board is now full
			{
				wxMessageBox(_("Draw! Try Again!"));
				tttScoreBuffer[tttDifficulty].draws += 1;
				for (int i = 0; i < tttSquares; i++)
				{
					tttMarks[i] = 0;
					tttButtons[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
					//tttImages[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
				}
				tttPlayerTurn = true;
				tttCount = 0;
				//evt.Skip();
				return;
			}

			tttAIPlay();											// AI's turn to play
			if (tttWinCheck(-1))									// Check if AI won
			{
				wxMessageBox(_("Uh Oh, You Lost! Try Again!!"));
				tttScoreBuffer[tttDifficulty].losses += 1;
				for (int i = 0; i < tttSquares; i++)
				{
					tttMarks[i] = 0;
					tttButtons[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
					//tttImages[i]->SetBitmap(wxBitmap("alpha.png", wxBITMAP_TYPE_PNG));
				}
				tttPlayerTurn = true;
				tttCount = 0;
				//evt.Skip();
				return;
			}				
			tttPlayerTurn = true;									// The user can now play again since the AI went
		}
	}
	//evt.Skip();
	return;
}

void MainWindow::tttAIPlay(void)
{
	int AIPlay;							// Integer that holds the play that the AI will use
	int chanceVar = rand() % 10;		// Variable for the random chances
	switch (tttDifficulty)				// AI plays according to difficulty
	{
	case 0:
		// Easy Mode - AI makes completely random plays
		while (true)
		{
			AIPlay = rand() % tttSquares;
			if (tttMarks[AIPlay] == 0)
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		break;
	case 1:
		// Normal Mode - AI will win if possible and will block 70% of the time
		AIPlay = tttCheckTwo(-1);
		if (AIPlay != -1)									// If AI can win, it will
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		if (chanceVar > 3)
		{
			AIPlay = tttCheckTwo(1);
			if (AIPlay != -1)									// If user is about to win, AI will block 70% of the time
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		AIPlay = tttFindPlay(-1);
		if (AIPlay != -1)									// If AI has a row/column/diagonal with a possiblity of a win, finds it
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		while (true)		// The AI plays randomly if there is no possible win, block, or priority play present
		{
			AIPlay = rand() % tttSquares;
			if (tttMarks[AIPlay] == 0)
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		break;
	case 2:
		// Hard Mode
		AIPlay = tttCheckTwo(-1);
		if (AIPlay != -1)									// If AI can win, it will
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		AIPlay = tttCheckTwo(1);
		if (AIPlay != -1)									// If user is about to win, AI will block
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		if (chanceVar > 5)
		{
			AIPlay = tttHardBlock(1);
			if (AIPlay != -1)									// If there is a guaranteed win for the user in the near future, preemptively block it (50% of time)
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		AIPlay = tttFindPlay(-1);
		if (AIPlay != -1)									// If AI has a row/column/diagonal with a possiblity of a win, finds it
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		while (true)		// The AI plays randomly if there is no possible win, block, or priority play present
		{
			AIPlay = rand() % tttSquares;
			if (tttMarks[AIPlay] == 0)
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		break;
	case 3:
		// Undefeatable Mode
		AIPlay = tttCheckTwo(-1);
		if (AIPlay != -1)									// If AI can win, it will
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		AIPlay = tttCheckTwo(1);
		if (AIPlay != -1)									// If user is about to win, AI will block
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		AIPlay = tttHardBlock(1);
		if (AIPlay != -1)									// If there is a guaranteed win for the user in the near future, preemptively block it
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		AIPlay = tttFindPlay(-1);
		if (AIPlay != -1)									// If AI has a row/column/diagonal with a possiblity of a win, finds it
		{
			tttCount++;
			tttMarks[AIPlay] = -1;
			//tttButtons[AIPlay]->SetLabelText(_("O"));
			tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
			break;
		}
		while (true)		// The AI plays randomly if there is no possible win, block, or priority play present
		{
			AIPlay = rand() % tttSquares;
			if (tttMarks[AIPlay] == 0)
			{
				tttCount++;
				tttMarks[AIPlay] = -1;
				//tttButtons[AIPlay]->SetLabelText(_("O"));
				tttButtons[AIPlay]->SetBitmap(wxBitmap("oMarkA.png", wxBITMAP_TYPE_PNG));
				break;
			}
		}
		break;
	default:
		// Throw an Error -> Should not be possible
		DBOUT("AI DIFFICULTY ERROR - SOMETHING WENT WRONG INTERNALLY...\n")
	}
}

bool MainWindow::tttWinCheck(int player)							// This function checks if the player or AI has won depending upon the player variable -> -1 or 1
{
	int currentRow;													// Temporary variable to count marks in each row
	for (int i = 0; i < tttSquares - 1; i++)
	{
		currentRow = 0;												// Reset temp var for each new row/column/diagonal
		for (int j = 0; j < tttDimension; j++)
		{
			currentRow += tttMarks[tttRows[i][j]];					// Add -1 for AI mark, 1 for user's mark, and 0 for no mark
		}
		if (abs(currentRow) == 3)									// If the temp var has absolute value of 3, then there is a win corresponding to value of player
		{
			return true;
		}
	}
	return false;													// No win detected
}

int MainWindow::tttCheckTwo(int player)								// This function checks if there are any rows/columns/diagonals that are about to win (2 of 3 marks)
{
	int currentRow;													// Temporary variable to count marks in each row
	int returnPosition;												// Integer for where mark needs to go to block or win
	for (int i = 0; i < tttSquares - 1; i++)
	{
		currentRow = 0;												// Reset temp var for each new row/column/diagonal
		for (int j = 0; j < tttDimension; j++)
		{
			currentRow += tttMarks[tttRows[i][j]];					// Add -1 for AI mark, 1 for user's mark, and 0 for no mark
		}
		if (currentRow == (2 * player))								// If the temp var is 2 or -2, then there is only one mark missing
		{
			for (int j = 0; j < tttDimension; j++)
			{
				if (!tttMarks[tttRows[i][j]])
				{
					returnPosition = tttRows[i][j];					// Find the empty square in the row and return it
				}
			}
			return returnPosition;
		}
	}
	return -1;														// Nowhere to block or win
}

int MainWindow::tttFindPlay(int player)								// This function finds a place in a row/column/diagonal not already blocked (returns -1 if none)
{
	int currentRow;													// Temporary variable to count marks in each row
	int possiblePosition[24];										// Integer array for possible places the AI can play
	int counter = 0;												// How many possible plays there are for the AI
	int returnPosition;
	for (int i = 0; i < tttSquares - 1; i++)
	{
		currentRow = 0;												// Reset temp var for each new row/column/diagonal
		for (int j = 0; j < tttDimension; j++)
		{
			currentRow += tttMarks[tttRows[i][j]];					// Add -1 for AI mark, 1 for user's mark, and 0 for no mark
		}
		if (currentRow == player)									// If row is -1, then the AI should at as a possible play
		{
			for (int j = 0; j < tttDimension; j++)
			{
				if (!tttMarks[tttRows[i][j]])
				{
					possiblePosition[counter] = tttRows[i][j];		// Find the empty square(s) in the row and add it to the array of possible plays
					counter++;										// Track the additional possibility with the counter variable
				}
			}
		}
	}
	if(counter)
	{
		returnPosition = possiblePosition[(rand() % counter)];		// Randomly choose a play out of those possible
		return returnPosition;										// Return chosen play
	}
	else
	{
		return -1;													// No specific play found
	}
}

int MainWindow::tttHardBlock(int player)							// This function keeps player from winning (impossible to beat)
{
	int currentRow;													// Temporary variable to count marks in each row
	int counter = 0;												// How many possible plays there are for the AI
	int returnPosition = -1;										// Return value (the position to play or -1 if nothing found)
	if (tttMarks[4] == player) // Check if the player played in the center
	{
		if (tttCount == 1 || tttCount == 3)
		{
			for (int i = 0; i < 4; i++)
			{
				if (tttMarks[tttCorners[i]] == 0)
				{
					returnPosition = tttCorners[i];		// Return a corner that is not filled
					break;
				}
			}
		}
	}
	if (tttMarks[0] == player || tttMarks[2] == player || tttMarks[6] == player || tttMarks[8] == player) // Check if the player played in a corner
	{
		if (tttCount == 1)
		{
			returnPosition = tttMarks[4] == 0 ? 4 : returnPosition;			// AI will always want to play center if user plays a corner first turn
		}
		else if (tttCount == 3)
		{
			returnPosition = tttMarks[0] == player ? 8 : returnPosition;		// Return Position becomes-
			returnPosition = tttMarks[2] == player ? 6 : returnPosition;		// the opposite of the corner-
			returnPosition = tttMarks[6] == player ? 2 : returnPosition;		// the player played.
			returnPosition = tttMarks[8] == player ? 0 : returnPosition;		// Remains previous value otherwise.

			if (tttMarks[returnPosition] != 0)	// If the opposite corner is not empty, any other corner works
			{
				for (int i = 0; i < 4; i++)
				{
					if (tttMarks[tttCorners[i]] == 0)
					{
						returnPosition = tttCorners[i];		// Return a corner that is not filled
					}
				}
			}
		}
	}
	if (tttMarks[1] == player || tttMarks[3] == player || tttMarks[5] == player || tttMarks[7] == player) // Check if the player played in a middle-edge
	{
		if (tttCount == 1)
		{
			returnPosition = tttMarks[4] == 0 ? 4 : returnPosition;			// AI will always want to play center if user plays a middle-edge first turn
		}
		else if (tttCount == 3)
		{
			if (tttMarks[0] == player || tttMarks[2] == player || tttMarks[6] == player || tttMarks[8] == player) // Play opposite corner if the user now played a corner
			{
				returnPosition = tttMarks[0] == player ? 8 : returnPosition;		// Return Position becomes-
				returnPosition = tttMarks[2] == player ? 6 : returnPosition;		// the opposite of the corner-
				returnPosition = tttMarks[6] == player ? 2 : returnPosition;		// the player played.
				returnPosition = tttMarks[8] == player ? 0 : returnPosition;		// Remains previous value otherwise.
			}
			else // If the User played a middle-edge, then AI wants to play a corner near a User's mark
			{
				returnPosition = tttMarks[1] == player ? 2 : returnPosition;		// Return Position becomes-
				returnPosition = tttMarks[3] == player ? 0 : returnPosition;		// the corner near the middle-edge-
				returnPosition = tttMarks[5] == player ? 8 : returnPosition;		// that the User played.
				returnPosition = tttMarks[7] == player ? 6 : returnPosition;		// Remains previous value otherwise.
			}
		}
	}
	return returnPosition;
}

int MainWindow::tttGetButtonBorders(int button)
{
	int returnStyle = 0;
	if (button < 6) // This includes all but the bottom three boxes
	{
		returnStyle |= wxBOTTOM;
	}
	if (button > 2) // This includes all but the top three boxes
	{
		returnStyle |= wxTOP;
	}
	if ((button % 3) != 0) // This includes all but the left-most three boxes
	{
		returnStyle |= wxLEFT;
	}
	if (((button - 2) % 3) != 0) // This includes all but the right-most three boxes
	{
		returnStyle |= wxRIGHT;
	}
	return returnStyle;
}

void MainWindow::OnTTTSliderChange(wxScrollEvent &evt)
{
	int newDifficulty = tttDiffSlider->GetValue();						// Store the change made on the slider
	tttDifficulty = newDifficulty;										// Change the difficulty accordingly
	tttDifficultyText->SetLabelText(tttDiffStrings[newDifficulty]);		// Show the change in the difficulty text
}

bool MainWindow::PlayAccessChange(bool state)
{
	bool errorReturn = true; // False if Error Occurs

	// Disable Play Buttons and Difficulty Settings
	errorReturn = mineGamePlayButton->Enable(state) ? errorReturn : false;
	errorReturn = tttGamePlayButton->Enable(state) ? errorReturn : false;
	errorReturn = tttDiffSlider->Enable(state) ? errorReturn : false;

	//Disable Menu Items that start games
	gamesMenu->Enable(menu::id::MINEGAMEID, state);
	gamesMenu->Enable(menu::id::TTTGAMEID, state);

	return errorReturn;
}

bool MainWindow::LoadHighscores(void)
{
	std::basic_string<TCHAR> temp = uname;

	switch (CURRENT_OS)
	{
	case 0: // Windows
		if (GetDriveType(_T("C:\\")) > 1)
		{
			path = "C:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub";
			CreateDirectoryA(path.c_str(), NULL); // Will create the path if it doesn't exist
			path += "\\scores.dat";
		}
		else if (GetDriveType(_T("D:\\")) > 1)
		{
			path = "D:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub";
			CreateDirectoryA(path.c_str(), NULL);
			path += "\\scores.dat";
		}
		else if (GetDriveType(_T("E:\\")) > 1)
		{
			path = "E:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub";
			CreateDirectoryA(path.c_str(), NULL);
			path += "\\scores.dat";
		}
		else
		{
			DBOUT("Drive Letter Not Supported")
		}
		DBOUT(path)
		DBOUT("\n")
		break;
	case 1: // Linux
		path = "~/.wxGameHub/scores.dat";
		break;
	case 2: // macOS
		path = "~/Library/Application Support/wxGameHub/scores.dat";
		break;
	case 3: // Other
		path = "~/wxGameHub/scores.dat";
		break;
	default: // Error - Should never default
		DBOUT("INTERNAL ERROR | OPERATING SYSTEM COULD NOT BE FOUND\n")
		return false;
	}
	
	std::ifstream infile(path.c_str()); // creates a file stream to read from the file in path from above
	if (!infile.good()) // If file cannot be accessed or does not exist, scores cannot be loaded
	{
		for (int i = 0; i < 10; i++)	// Load filler/empty scores - load issue or no scores exist yet
		{
			for (int j = 0; j < 15; j++)
			{
				if (j < 6) // Some periods to act as a placeholder while no scores exist
				{
					minesScoreBuffer[i].name[j] = '.';
				}
				else
				{
					minesScoreBuffer[i].name[j] = '\0'; // Terminating NULL characters for printing
				}
			}
			minesScoreBuffer[i].score = 0;
		}

		for (int i = 0; i < 4; i++)
		{
			tttScoreBuffer[i].difficulty = i;
			tttScoreBuffer[i].wins = 0;
			tttScoreBuffer[i].draws = 0;
			tttScoreBuffer[i].losses = 0;
		}
	}
	else // File can be loaded from
	{
		for (int i = 0; i < 4; i++)
		{
			if (infile.peek() != EOF)
			{
				DBOUT("READING")
				infile.read((char *)&tttScoreBuffer[i], sizeof(tttHighscore));
			}
		}

		for (int i = 0; i < 10; i++)
		{
			if (infile.peek() != EOF)
			{
				DBOUT("READING")
				infile.read((char *)&minesScoreBuffer[i], sizeof(minesHighscore));
			}
		}
		infile.close();
	}
	return true;
}

bool MainWindow::WriteHighscores(void)
{
	std::basic_string<TCHAR> temp = uname;

	switch (CURRENT_OS)
	{
	case 0: // Windows
		if (GetDriveType(_T("C:\\")) > 1)
			path = "C:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub\\scores.dat";
		else if (GetDriveType(_T("D:\\")) > 1)
			path = "D:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub\\scores.dat";
		else if (GetDriveType(_T("E:\\")) > 1)
			path = "E:\\Users\\" + temp + "\\AppData\\Roaming\\wxGameHub\\scores.dat";
		else
		{
			DBOUT("Drive Letter Not Supported")
		}
		DBOUT(path)
		DBOUT("\n")
		break;
	case 1: // Linux
		path = "~/.wxGameHub/scores.dat";
		break;
	case 2: // macOS
		path = "~/Library/Application Support/wxGameHub/scores.dat";
		break;
	case 3: // Other
		path = "~/wxGameHub/scores.dat";
		break;
	default: // Error - Should never default
		DBOUT("INTERNAL ERROR | OPERATING SYSTEM COULD NOT BE FOUND\n")
			return false;
	}

	std::ofstream outfile(path.c_str()); // creates a file stream to read from the file in path from above
	if (!outfile.good()) // If file cannot be accessed or does not exist, scores cannot be loaded
	{
		outfile.close();
		return false;	// Load error... return false
	}
	else // File can be loaded from
	{
		for (int i = 0; i < 4; i++)
			outfile.write((char *)&tttScoreBuffer[i], sizeof(tttHighscore));

		for (int i = 0; i < 10; i++)
			outfile.write((char *)&minesScoreBuffer[i], sizeof(minesHighscore));
	}
	outfile.close();
	return true;

	/*std::ofstream wf("scores.dat", std::ios::out | std::ios::binary);
	if (!wf) {
		//cout << "Cannot open file!" << endl;
		return 1;
	}

	tttHighscore tttScoreWrite[4];
	for (int i = 0; i < 4; i++)
	{
		tttScoreWrite[i].difficulty = i;
		tttScoreWrite[i].wins = i * 11;
		tttScoreWrite[i].draws = i * 11;
		tttScoreWrite[i].losses = i * 11;
	}

	minesHighscore minesScoreWrite[10];
	string namesArray[10] = { "Alex","Bob","Christy","Don","Ellen","Forrest","George","Harry","Isaiah","Jack" };
	for (int i = 0; i < 10; i++)
	{
		minesScoreWrite[i].name = namesArray[i];
		minesScoreWrite[i].score = i * 111;
	}

	for (int i = 0; i < 4; i++)
		wf.write((char *)&tttScoreWrite[i], sizeof(tttHighscore));

	for (int i = 0; i < 10; i++)
		wf.write((char *)&minesScoreWrite[i], sizeof(minesHighscore));

	wf.close();
	if (!wf.good()) {
		//cout << "Error occurred at writing time!" << endl;
		return 1;
	}*/
}

void MainWindow::gamesInfoInit(void)
{
	string minesDescriptionString = "\tWelcome to Mystery Mines! In this game, you can LEFT-CLICK any of squares to reveal if there is a mine or not. If there is, the game will be over! So be careful! If there isn't, however, then the square will show how many mines are in the immediately surrounding squares. If there is no number after it is clicked, then there are no mines nearby! You may RIGHT-CLICK a square in order to mark it with a red X which can help you keep track of the squares that might have mines! After each game, you will receive a score - the highscores are listed in below! Good luck!";
	string tttDescriptionString = "\tWelcome to Tic-Tac-Toe! You will play X's, and will go first. After each turn you take, the computer will play a O in response. Depending upon the difficulty you have selected, the computer will play more or less tactically. Your goal is to get three X's in a row without letting the computer get three O's in a row! You may score horizontally, vertically, or diagonally. It is also possible to draw, if there are no three-in-a-row matches. You can see how many wins, draws, and losses you have accumulated for each difficulty in the highscores section below! Have fun playing!";
	string tempHighscore;

	minesInfoBoxSizer = new wxBoxSizer(wxVERTICAL);
	minesInfoBoxSizer->AddSpacer(20);
	minesGamePage->SetSizer(minesInfoBoxSizer);
	minesDescription = new wxStaticText(minesGamePage, wxID_ANY, minesDescriptionString);
	minesDescription->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	minesInfoBoxSizer->Add(minesDescription, 1, wxEXPAND | wxALL);
	minesHighscoreNotebook = new wxNotebook(minesGamePage, wxID_ANY);
	minesDifficultyPage = new wxWindow(minesHighscoreNotebook, wxID_ANY);
	minesHighscoreNotebook->AddPage(minesDifficultyPage, _("Highscores"));
	minesHighscoreList = new wxStaticText(minesDifficultyPage, wxID_ANY, _("Loading..."));
	tempHighscore = " Mystery Mines Highscores:\n\n     Name\tScore";
	for (int i = 0; i < 9; i++)
	{
		tempHighscore = tempHighscore + "\n " + std::to_string(i + 1) + ".   " + minesScoreBuffer[i].name + "\t" + std::to_string(minesScoreBuffer[i].score);
	}
	tempHighscore = tempHighscore + "\n 10. " + minesScoreBuffer[9].name + "\t" + std::to_string(minesScoreBuffer[9].score);
	minesHighscoreList->SetLabelText(tempHighscore);
	minesHighscoreList->SetFont(wxFont(19, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	minesInfoBoxSizer->Add(minesHighscoreNotebook, 1, wxEXPAND | wxALL);


	tttInfoBoxSizer = new wxBoxSizer(wxVERTICAL);
	tttInfoBoxSizer->AddSpacer(20);
	tttGamePage->SetSizer(tttInfoBoxSizer);
	tttDescription = new wxStaticText(tttGamePage, wxID_ANY, tttDescriptionString);
	tttDescription->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	tttInfoBoxSizer->Add(tttDescription, 1, wxEXPAND | wxALL);

	wxNotebook* tttHighscoreNotebook = new wxNotebook(tttGamePage, wxID_ANY);
	wxWindow* tttDifficultyPages[4];
	wxBoxSizer* tttHighscoreSizers[4];
	for (int i = 0; i < 4; i++)
	{
		tttDifficultyPages[i] = new wxWindow(tttHighscoreNotebook, wxID_ANY);
		tttHighscoreNotebook->AddPage(tttDifficultyPages[i], tttDiffStrings[i]);
		tttHighscoreDisplay[i] = new wxStaticText(tttDifficultyPages[i], wxID_ANY, _("Loading..."));
		tempHighscore = " " + tttDiffStrings[tttScoreBuffer[i].difficulty] + " Tic-Tac-Toe Highscores:\n\n Wins: " + std::to_string(tttScoreBuffer[i].wins) + "\n\n Draws: " + std::to_string(tttScoreBuffer[i].draws) + "\n\n Losses: " + std::to_string(tttScoreBuffer[i].losses);
		tttHighscoreDisplay[i]->SetLabelText(tempHighscore);
		tttHighscoreDisplay[i]->SetFont(wxFont(19, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	}
	tttInfoBoxSizer->Add(tttHighscoreNotebook, 1, wxEXPAND | wxALL);

	//tttHighscoreList;
}

void MainWindow::MinesAddHighscore(int score)
{
	string nameInput; // Temporary string to store name input by user
	int j = 9; // Loop iterator
	if (score <= minesScoreBuffer[9].score) // Return if the score is not a highscore
	{
		return;
	}
	else    // Add new highscore to stored scores
	{
		wxTextEntryDialog* namePrompt = new wxTextEntryDialog(this, _("You got a new highscore! Please enter a name for the score!\n"));
		namePrompt->SetMaxLength(15);
		namePrompt->ShowModal();
		nameInput = namePrompt->GetValue();
		for (int i = 0; i < 10; i++)
		{
			if (score > minesScoreBuffer[i].score)
			{
				for (j; j > i; j--)
				{
					minesScoreBuffer[j] = minesScoreBuffer[j - 1];
				}
				minesScoreBuffer[i].score = score;
				for (j = 0; j < 15; j++)
				{
					if (j < nameInput.length())
					{
						minesScoreBuffer[i].name[j] = nameInput[j];
					}
					else
					{
						minesScoreBuffer[i].name[j] = '\0';
					}
				}
				break;
			}
		}
	}
}

void MainWindow::HighscoreUpdate(void)
{
	string tempHighscore;

	tempHighscore = " Mystery Mines Highscores:\n\n     Name\tScore";
	for (int i = 0; i < 9; i++)
	{
		tempHighscore = tempHighscore + "\n " + std::to_string(i + 1) + ".   " + minesScoreBuffer[i].name + "\t" + std::to_string(minesScoreBuffer[i].score);
	}

	for (int i = 0; i < 4; i++)
	{
		tttDifficultyPages[i] = new wxWindow(tttHighscoreNotebook, wxID_ANY);
		tttHighscoreNotebook->AddPage(tttDifficultyPages[i], tttDiffStrings[i]);
		tttHighscoreDisplay[i] = new wxStaticText(tttDifficultyPages[i], wxID_ANY, _("Loading..."));
		tempHighscore = " " + tttDiffStrings[tttScoreBuffer[i].difficulty] + " Tic-Tac-Toe Highscores:\n\n Wins: " + std::to_string(tttScoreBuffer[i].wins) + "\n\n Draws: " + std::to_string(tttScoreBuffer[i].draws) + "\n\n Losses: " + std::to_string(tttScoreBuffer[i].losses);
		tttHighscoreDisplay[i]->SetLabelText(tempHighscore);
		tttHighscoreDisplay[i]->SetFont(wxFont(19, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	}
}
