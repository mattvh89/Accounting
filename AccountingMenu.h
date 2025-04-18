#pragma once
#include "AccountManager.h"
#include "smart_pointer.h"
#include "Report.h"
#include <Windows.h>
#include <iostream>

constexpr short NUM_SCROLLS = 5;
constexpr short NUM_OPTIONS = 8;
constexpr short FIRST_LINE = 4;
constexpr short SIGNIN_LINE = 4;
constexpr short LAST_ROW = 49;
constexpr short CALENDAR_COL_1 = 1;
constexpr short CALENDAR_COL_2 = 27;
constexpr short CALENDAR_COL_3 = 53;
constexpr short CALENDAR_COL_4 = 79;

constexpr short SCREEN_WIDTH = 100;
constexpr short SCREEN_HEIGHT = 50;
constexpr short RIGHT_THIRD = (SCREEN_WIDTH / 3) * 2;

#define ESC "\x1b"
#define CSI "\x1b["

const std::string CLEAR_LINE = "                                                  ";						 // 50 spaces
const std::string TITLE = "Double enter THIS!";
const std::string UNDER_LINE = "                ---------------====================---------------";		// 75 characters
const std::string LINE = "----------------------------------------------------------------------------------------------------";
const std::string CLEAR_CALC_LINE = "                     ";

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
																										 const BackGroundColor& bg)							{ printf(CSI "%d;%dm", fg, bg); } 

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

	inline				void							clearScreen										()													{ 	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
																																								for (size_t i = SIGNIN_LINE; i < SCREEN_HEIGHT - 2; ++i) 
																																									this->clearLine((unsigned short)i); }

// Special Calculator functions
						void							saveConsoleArea									(SMALL_RECT& rect,
																										 const COORD& coord,
																										 Ptr<CHAR_INFO>& buffer)			const;

						void							restoreConsoleArea								(SMALL_RECT& rect,
																										 const COORD& coord,
																										 Ptr<CHAR_INFO>& buffer)			const;

						void							printCalculator									(char indexToHilight = ' ');

						void							runCalculator									();

// Special Calendar functions
	inline	static		bool							isLeap											(int year)											{ return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); }

			static		int								getDaysInMonth									(int month, 
																										 int year);

			static		int								getStartDay										(int month, 
																										 int year);

						void							printCalendarMonth								(int month, 
																										 int year, 
																										 int startX,
																										 int startY);

						void							printCalendarYearBottomScreen					(int year);

// Menu functions
	// Print functions
						void							printTitle										();

						void							printAccountNames								();

						void							printTransactionHeader							();

						void							printTransaction								(const Transaction& transaction,
																										 const size_t& lineNumber);

						void							printAllTransactionsWithScrolling				(const Ptr<Transaction>& list);

						void							printAllTransactions							(const Ptr<Transaction>& list);
	// Input fucntions
						int								inputDate										(const size_t& lineNumber);

						bool							signin											();

	// Other functions
						void							presentMenu										();

						void							highlighScrollOption							(const unsigned short& options);

						unsigned short					getMainMenuInput								();

						bool							addAccount										();

						bool							addTransaction									();

						bool							removeTransaction								();

						bool							viewAccount										();

						bool							viewAccount										(const std::string& acctName);

						bool							generateReport									();

						bool							generateReport									(const std::string& acctName);



private:
	const std::string OPTIONS[NUM_OPTIONS]   = { "+ Acct", "Chng User", "View", "Add Trans", "Rmv Trans", "Report", "Sort", "Quit"};
	const std::string SCROLLING[NUM_SCROLLS] = { "[/|\\]Up", "[\\|/]Down", "[Pg Up]", "[Pg Dwn]", "[End]Quit" };

	const COORD CALCULATOR_COORD = { SCREEN_WIDTH  - 9,
									 SCREEN_HEIGHT - 7  };

private:
	Ptr<AccountManager> m_acctManager;
	Ptr<std::string>    m_accountNames;
	Ptr<Report>			m_reports;
	std::string			m_userName;
	std::string			m_lastViewedAccount;
	HANDLE				m_stdOutHandle,
						m_stdInHandle;
	DWORD				m_outputMode;
	COORD				m_coord;
	INPUT_RECORD		m_ir;
	double				m_calcResult;
};

