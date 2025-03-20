#pragma once
#include "AccountManager.h"
#include "smart_pointer.h"
#include <Windows.h>
#include <iostream>

const std::string CLEAR_LINE = "                                                  ";						 // 50 spaces
const std::string TITLE = "Double enter THIS!";
const std::string UNDER_LINE = "                ---------------====================---------------";		// 75 characters
const std::string LINE = "----------------------------------------------------------------------------------------------------";

#define NUM_OPTIONS 6
#define FIRST_LINE 4
#define SIGNIN_LINE 4
#define LAST_ROW 49

#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 50
#define RIGHT_THIRD	  66

#define ESC "\x1b"
#define CSI "\x1b["

enum class ForeGroundColor
{
	None = 29, Black = 30, Red, Green, Yellow, Blue, Magenta, Cyan, White
};

enum class BackGroundColor
{
	None = 39, Black = 40, Red, Green, Yellow, Blue, Magenta, Cyan, White
};

class AccountingMenu
{
public:

														AccountingMenu									();

	inline		const	Ptr<AccountManager>&			getManager										()									const			{ return m_acctManager; }

						void							run												();


private:
						void							updateMainFile									();

						void							printHorizontalBorder							(const COORD& size,
																										 const ForeGroundColor& fg,
																										 const BackGroundColor& bg,
																										 bool fIsTop);

// Text and cursor functions
						void							setCursorPosition								(const unsigned short& x,
																										 const unsigned short& y);

	inline				void							setTextColor									(const ForeGroundColor& fg,
																										 const BackGroundColor& bg)							{ printf(CSI "%d;%dm", fg, bg); } // bright yellow on bright blue}

	inline				void							echoOff											()													{  m_outputMode &= ~ENABLE_ECHO_INPUT; 
																																							   SetConsoleMode(m_stdInHandle, m_outputMode); }

	inline				void							echoOn											()													{ m_outputMode |= ENABLE_ECHO_INPUT;
																																							  SetConsoleMode(m_stdInHandle, m_outputMode);  }

	inline				void							hideCursor										()									const			{ printf(ESC "[?25l"); }

	inline				void							showCursor										()									const			{ printf(ESC "[?25h"); }

// Screen clearing methods

	inline				void							clearLine										()													{ setCursorPosition(0, m_coord.Y);
																																							  std::cout << CLEAR_LINE << CLEAR_LINE << std::flush;
																																							  setCursorPosition(0, m_coord.Y);}

	inline				void							clearHalfLine									()													{ setCursorPosition(m_coord.X, m_coord.Y);
																																							  std::cout << CLEAR_LINE << std::flush;
																																							  setCursorPosition(m_coord.X, m_coord.Y); }
						void							clearHalfScreen									();

						void							clearTwoThirds									();

						void							clearLine										(const unsigned short& y);

						void							clearHalfLine									(const unsigned short& y);

	inline				void							clearScreen										()													{ 	for (size_t i = SIGNIN_LINE; i < SCREEN_HEIGHT - 4; ++i) this->clearLine((unsigned short)i); }

// Menu functions
						void							printTitle										();

						bool							signin											();

						void							presentMenu										();

						unsigned short					getMainMenuInput								();

						bool							addAccount										();

						bool							addTransaction									();

						bool							removeTransaction								();

						void							viewAccount										();

						bool							viewAccount										(const std::string& acctName);

protected:
	const std::string OPTIONS[NUM_OPTIONS] = { "Add Acct", "Open Acct", "View Acct", "Add Trans", "Remove Trans", "Quit"};

private:
	Ptr<AccountManager> m_acctManager;
	Ptr<std::string>    m_accountNames;
	std::string			m_userName;
	std::string			m_lastViewedAccount;
	HANDLE				m_stdOutHandle,
						m_stdInHandle;
	DWORD				m_outputMode;
	COORD				m_coord;
	INPUT_RECORD		m_ir;
};

