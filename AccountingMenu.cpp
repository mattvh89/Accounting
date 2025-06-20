#include "AccountingMenu.h"
#include "AccountManager.h"
#include "Encrypt.h"
#include "Report.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <locale>


AccountingMenu::AccountingMenu()
	: m_acctManager(), m_accountNames(), m_lastViewedAccount(""),
	m_stdOutHandle(GetStdHandle(STD_OUTPUT_HANDLE)),
	m_stdInHandle(GetStdHandle(STD_INPUT_HANDLE)),
	m_outputMode(), m_coord(), m_ir(),
	m_calcResult(0.0),
	m_firstSignIn(true)
{
	if (!GetConsoleMode(m_stdOutHandle, &m_outputMode))
		std::cerr << "GetConsoleMode (output) failed with error " << GetLastError() << '\n';

	m_outputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(m_stdOutHandle, m_outputMode))
		std::cerr << "SetConsoleMode (output) failed with error " << GetLastError() << '\n';

	//// Set screen buffer size first
	//COORD bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
	//if (!SetConsoleScreenBufferSize(m_stdOutHandle, bufferSize))
	//	std::cerr << "SetConsoleScreenBufferSize failed with error " << GetLastError() << '\n';

	//SMALL_RECT windowSize = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1};
	//SetConsoleWindowInfo(m_stdOutHandle, TRUE, &windowSize);

	COORD bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };

	// Step 1: Set buffer size
	if (!SetConsoleScreenBufferSize(m_stdOutHandle, bufferSize))
		std::cerr << "SetConsoleScreenBufferSize failed with error " << GetLastError() << '\n';

	// Step 2: Define window size (must be within the buffer size)
	SMALL_RECT windowSize = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };

	// Step 3: Apply window size
	if (!SetConsoleWindowInfo(m_stdOutHandle, TRUE, &windowSize))
		std::cerr << "SetConsoleWindowInfo failed with error " << GetLastError() << '\n';

	m_coord.X = 0;
	m_coord.Y = 0;
	SetConsoleCursorPosition(m_stdOutHandle, m_coord);
}

void AccountingMenu::run()
{
	unsigned short mainMenuKey;
	bool running = true;
	bool signinStatus = SIGNED_OUT;
	std::time_t t = std::time(nullptr);
	std::tm now;
	localtime_s(&now, &t);
	int currentYear = now.tm_year + 1900;

	this->clearScreen();
	this->clearLine(SCREEN_WIDTH - 1); this->clearLine(SCREEN_WIDTH - 2);
	while (running)
	{
		// Sign in first
		while (signinStatus == SIGNED_OUT)
		{
			this->printTitle();
			signinStatus = this->signin();
		}

		// If we just signed in, print he Main account
		if (m_firstSignIn)
		{
			this->summary();
			this->printCalendarYearBottomScreen((unsigned short)currentYear);
			m_firstSignIn = false;
		}


		this->presentMenu();																				// print the menu options
		this->setCursorPosition(1, 1);																		// make sure the cursor isn't at the end of the screen causing weird effects
		mainMenuKey = this->getMainMenuInput();																// get the menu option input


		// Handle each menu option case
		switch (std::tolower(mainMenuKey))
		{
		case 'c':
			this->runCalculator();
			break;

		case 'd':
			this->printCalendarYearBottomScreen((unsigned short)currentYear);
			break;

		case 's':
			this->summary();
			break;

		case '1':
			if (this->addAccount())
			{
				this->presentMenu();
				this->setCursorPosition((SCREEN_WIDTH >> 1) - 14, SCREEN_HEIGHT >> 1);
				std::cout << "Enter menu option to continue";
			}
			else
				this->viewAccount("Main");
			break;
		case '2':
			signinStatus = SIGNED_OUT;
			m_firstSignIn = true;
			for (short i = SIGNIN_LINE; i < SCREEN_HEIGHT - 4; ++i)
				clearLine((unsigned short)i);
			break;
		case '3':
			this->viewAccount();
			break;
		case '4':
			if (this->addTransaction() == false)
			{
				this->setCursorPosition((SCREEN_WIDTH >> 1) - 13, SCREEN_HEIGHT >> 1);
				std::cout << "Input transaction cancelled";
			}
			this->setCursorPosition((SCREEN_WIDTH >> 1) - 14, (SCREEN_HEIGHT >> 1) + 1);
			std::cout << "Enter menu option to continue";
			break;
		case '5':
			this->removeTransaction();
			break;
		case '6':
			this->generateReport();
			break;
		case '7':																							// sort
			if(not m_lastViewedAccount.empty())
			{
				if (m_lastViewedAccount.compare("Main") == 0) break;
				m_acctManager->sortTransactionsByDate(m_lastViewedAccount);
				this->viewAccount(m_lastViewedAccount);
			}
			break;
		case '8':																							// exit program
			running = false;
			for (size_t i = 1; i <= SCREEN_HEIGHT; ++i) this->clearLine((unsigned short)i);
			this->setCursorPosition(0, 0);
			std::locale comma_locale("");
			std::cout.imbue(comma_locale);
			std::cout << "Memory used at exit:   " << (Ptr<Transaction>::memoryUsage(true) + Ptr<std::string>::memoryUsage(true) + Ptr<AccountManager>::memoryUsage(true)) / 1000 << " Kb\n";
			break;
		}
		this->setCursorPosition(1, 2);
		this->echoOff();
	}
}

void AccountingMenu::printCalculator(char indexToHilight)
{
	static const char CALC_BUTTON_ARRAY[4][5] = { {'=', '7', '8', '9', 'x'},
				  							    {'Q', '4', '5', '6', '/'},
												{'D', '1', '2', '3', '+'},
												{'C', 'M', '0', '.', '-'} };

	for (size_t y = 0; y < 4; ++y)
	{
		this->setCursorPosition(SCREEN_WIDTH - 9, static_cast<unsigned short>(CALCULATOR_COORD.Y + y + 1));
		std::cout << "          ";
		this->setCursorPosition(SCREEN_WIDTH - 9, static_cast<unsigned short>(CALCULATOR_COORD.Y + y + 1));
		for (size_t x = 0; x < 5; ++x)
		{
			if(indexToHilight != ' ' and std::toupper(CALC_BUTTON_ARRAY[y][x]) == std::toupper(indexToHilight)
				or (indexToHilight == '*' and y == 0 and x == 4))
			{
				this->setTextColor(ForeGroundColor::Black, BackGroundColor::Yellow);
				std::cout << CALC_BUTTON_ARRAY[y][x] << ' ';
				this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
			}
			else
				std::cout << CALC_BUTTON_ARRAY[y][x] << ' ';
		}
	}
}

void AccountingMenu::runCalculator()
{
	enum Operators { None, Add, Subtract, Multiply, Divide, Equals };

	std::stringstream ss;
	Ptr<CHAR_INFO> buffer(ARRAY, 5 * 10);
	SMALL_RECT window = { SCREEN_WIDTH - 9, SCREEN_HEIGHT - 7, 9, 6 };

	double num1 = 0.0, num2 = 0.0, ans = 0.0, saved = 0.0;
	Operators calcOperator = Operators::None;
	DWORD read;
	char ch;
	bool calcRunning = true;
	bool deleteOnNextNum = false;
	bool savedInMemory = false;

	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	this->echoOff();
	this->hideCursor();

	auto clearDisplay = [&]() 
						   {
						       this->setCursorPosition(SCREEN_WIDTH - 9, SCREEN_HEIGHT - 7);
							   std::cout << std::setw(10) << ' ';
						   };

	auto printNumber = [&](const std::string& str) 
						  {
						       this->setCursorPosition(SCREEN_WIDTH - static_cast<unsigned short>(str.length()), SCREEN_HEIGHT - 7);
							   std::cout << str;
						  };

	clearDisplay();
	this->setCursorPosition(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 7);

	while (calcRunning)
	{
		this->printCalculator();
		ReadConsoleInput(m_stdInHandle, &m_ir, 1, &read);
		if (m_ir.EventType != KEY_EVENT || !m_ir.Event.KeyEvent.bKeyDown)
			continue;

		ch = m_ir.Event.KeyEvent.uChar.AsciiChar;
		this->printCalculator(ch);
		Sleep(100);

		// Handle numeric input
		if ((ch >= '0' && ch <= '9') || ch == '.' ||
			(ss.str().empty() && calcOperator == Operators::None && ch == '-'))
		{
			if (deleteOnNextNum)
			{
				ss.str(""); ss.clear();
				clearDisplay();
				deleteOnNextNum = false;
			}

			if (ss.str().length() <= 7)
			{
				ss << ch;
				printNumber(ss.str());
			}
			continue;
		}

		// Quit
		if (ch == 'q' || ch == 'Q')
		{
			calcRunning = false;
			m_calcResult = ans;
			break;
		}

		// Delete last digit
		if (ch == 'd' || ch == 'D')
		{
			std::string temp = ss.str();
			if (!temp.empty()) temp.pop_back();
			ss.str(temp); ss.clear();
			clearDisplay();
			printNumber(temp);
			continue;
		}

		// Clear calculator
		if (ch == 'c' || ch == 'C')
		{
			ss.str(""); ss.clear();
			num1 = num2 = ans = 0.0;
			calcOperator = Operators::None;
			clearDisplay();
			continue;
		}

		if (ch == 'm' || ch == 'M')
		{
			if (!savedInMemory)
			{
				saved = ans;  // store last answer
				savedInMemory = true;
			}
			else
			{
				ss.str(""); ss.clear();
				ss << saved;

				clearDisplay();
				printNumber(ss.str());

				savedInMemory = false;
				deleteOnNextNum = false;
			}
			continue;
		}

		// Handle operator input
		if (std::string("+-*/x=").find(ch) != std::string::npos)
		{
			if (!ss.str().empty())
			{
				ss.clear(); ss.seekg(0);
				double currentNum = 0.0;
				ss >> currentNum;
				ss.str(""); ss.clear();

				if (calcOperator == None || calcOperator == Equals)
				{
					num1 = currentNum;
				}
				else
				{
					num2 = currentNum;

					switch (calcOperator)
					{
					case Add:      ans = num1 + num2; break;
					case Subtract: ans = num1 - num2; break;
					case Multiply: ans = num1 * num2; break;
					case Divide:
						if (num2 == 0.0) 
						{
							clearDisplay();
							printNumber(" ERROR ");
							continue;
						}
						ans = num1 / num2;
						break;
					default: break;
					}

					num1 = ans;
					clearDisplay();
					this->setCursorPosition(SCREEN_WIDTH - 9, SCREEN_HEIGHT - 7);
					std::cout << std::setw(9) << std::right << std::fixed << std::setprecision(2) << ans;
				}

				deleteOnNextNum = true;
			}

			switch (ch)
			{
			case '+': calcOperator = Add; break;
			case '-': calcOperator = Subtract; break;
			case 'x':
			case '*': calcOperator = Multiply; break;
			case '/': calcOperator = Divide; break;
			case '=': calcOperator = Equals; break;
			}
		}
	}
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
}

unsigned short AccountingMenu::getDaysInMonth(int month, int year)
{
	static const unsigned short days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	if (month == 2 && isLeap(year))
		return 29;
	return days[month - 1];
}

unsigned short AccountingMenu::getStartDay(int month, int year)
{
	std::tm time_in = { 0, 0, 0, 1, month - 1, year - 1900 }; // 1st of month
	std::mktime(&time_in);
	return static_cast<unsigned short>(time_in.tm_wday);      // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
}

void AccountingMenu::printCalendarMonth(unsigned short month, unsigned short year, unsigned short startX, unsigned short startY)
{
	static const std::string months[] = {
	"January", "February", "March",     "April",
	"May",     "June",     "July",      "August",
	"September", "October", "November", "December"
	};

	unsigned short daysInMonth = getDaysInMonth(month, year);
	unsigned short startDay = getStartDay(month, year);
	unsigned short lineCount = 0;

	std::time_t t = std::time(nullptr);
	std::tm now;
	localtime_s(&now, &t);

	int currentDay = now.tm_mday;
	int currentMonth = now.tm_mon + 1;
	//int currentYear = now.tm_year + 1900;

	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);

	this->setCursorPosition(startX, startY);															// print the month and year
	std::cout << std::setfill(' ') << "     ";
	this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
	std::cout << months[month - 1] << " " << year;

	this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	this->setCursorPosition(startX, startY + 1);														// print the days of the week
	std::cout << "Su Mo Tu We Th Fr Sa ";
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);

	this->setCursorPosition(startX, startY + 2);														// padd the first line where previous dates belong to the preceding month
	for (int i = 0; i < startDay; ++i)
		std::cout << "   " << std::right;

	for (int day = 1; day <= daysInMonth; ++day) 
	{
		if (month < currentMonth or																		// Mark the days that passed in red
			(month == currentMonth and day < currentDay))
		{
			this->setTextColor(ForeGroundColor::White, BackGroundColor::Red);
			std::cout << std::setw(2) << day;
			this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
			std::cout << " ";
		}
		else
		if(month == currentMonth and day == currentDay)													// Mark the current day in blue
		{
			this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
			std::cout << std::setw(2) << day;
			this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
			std::cout << " ";
		}
		else																							// print the other days regularly
		{
			std::cout << std::setw(2) << day << " ";
		}
		if ((startDay + day) % 7 == 0)
			this->setCursorPosition(startX, startY + 2 + ++lineCount);
	}
}

void AccountingMenu::printCalendarYearBottomScreen(unsigned short currentYear)
{
	for (unsigned short y = 23; y < SCREEN_HEIGHT - 2; ++y)
		this->clearLine(y);
	for (unsigned short i = 1; i < 12; i += 4)
	{
		this->printCalendarMonth(i,     currentYear, CALENDAR_COL_1, 23 + (8 * (i / 4)));
		this->printCalendarMonth(i + 1, currentYear, CALENDAR_COL_2, 23 + (8 * (i / 4)));
		this->printCalendarMonth(i + 2, currentYear, CALENDAR_COL_3, 23 + (8 * (i / 4)));
		this->printCalendarMonth(i + 3, currentYear, CALENDAR_COL_4, 23 + (8 * (i / 4)));
	}
}

void AccountingMenu::printTitle()
{
	COORD border = { 0, 0 };
	border.X = SCREEN_WIDTH;
	border.Y = 1;

	for (size_t i = 1; i <= SCREEN_HEIGHT; ++i) this->clearLine((unsigned short)i);								// clear the screen

	this->setCursorPosition(1, 1);
	this->printHorizontalBorder(border, ForeGroundColor::Blue, BackGroundColor::Black, true);							// top border line

	this->setCursorPosition(SCREEN_WIDTH / 2 - (unsigned short)TITLE.length() / 2, 2);								// set cursor to print title
	std::cout << TITLE;

	border.Y = 3;																								// set cursor to print bottomr title border
	this->setCursorPosition(0, 3);
	this->printHorizontalBorder(border, ForeGroundColor::Blue, BackGroundColor::Black, false);						// bottom border
}

void AccountingMenu::printAccountNames()
{
	std::stringstream ss;
	std::time_t t = std::time(nullptr);
	std::tm now;
	localtime_s(&now, &t);
	unsigned short currentMonth = static_cast<unsigned short>(now.tm_mon + 1);
	unsigned short currentYear = static_cast<unsigned short>(now.tm_year + 1900);
	unsigned short y = FIRST_LINE;
	constexpr unsigned short X = (RIGHT_THIRD + ((SCREEN_WIDTH - RIGHT_THIRD) / 2));

	setCursorPosition(X - 4, SIGNIN_LINE);
	std::cout << std::setw(13) << std::right << "Accounts:";
	setCursorPosition(X - 4, SIGNIN_LINE + 1);
	std::cout << std::setw(13) << "_________";
	unsigned short i = 1;
	for (const auto& it : m_acctManager->getAccountNames())																// Present the accounts on the right hand side
	{
		ss.str("");																										// clear the string stream
		ss << i++ << ") " << it;																						// form the string so we know how big it is for formatted output
		setCursorPosition(X, y++ + 2);				// set cursor position to second half of the second third of the screen
		this->setTextColor(((i-1) > 6) ? ForeGroundColor::Black : ForeGroundColor::White,								// formulate the color
							static_cast<BackGroundColor>(static_cast<size_t>(BackGroundColor::Red) + (i-1) % 6));

		std::cout << std::setw(10) << std::left << ss.str();															// print it left justified
	}
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);													// reset the defualt color scheme

	this->printCalendarMonth(currentMonth, currentYear, CALENDAR_COL_4, y + 3);

	if (m_calcResult > .01)
	{
		this->setCursorPosition(X + 3, y + 11);
		std::cout << "Last Amount:";
		this->setCursorPosition(X + 6, y + 12);
		std::cout << "$";
		this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		std::cout << m_calcResult;
		this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	}
}

void AccountingMenu::printTransactionHeader()
{
	std::cout << "#####  "
		<< "Date        "
		<< "Amount    "
		<< "Credit/Debit  "
		<< "Account                                 "
		<< "Description\n";
	//this->printHorizontalBorder(COORD{ 5, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	//std::cout << " ";
	//this->printHorizontalBorder(COORD{ 6, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	//std::cout << "      ";
	//this->printHorizontalBorder(COORD{ 8, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	//std::cout << ' ';
	//this->printHorizontalBorder(COORD{ 14, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	//this->printHorizontalBorder(COORD{ 9, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	//std::cout << "  ";
	//this->printHorizontalBorder(COORD{ 48, 0 }, ForeGroundColor::Yellow, BackGroundColor::Black, false);
	std::cout << "_____  __________  __________ ___________  ________  _______________________________________________" << std::endl;
}

void AccountingMenu::printTransaction(const Transaction& transaction, const size_t& lineNumber)
{
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);																		// Print the index
	this->setCursorPosition(7, (unsigned short)lineNumber);
	std::chrono::year_month_day ymd = std::chrono::year_month_day(std::chrono::sys_days(std::chrono::days(transaction.date())));			// Print the date

	std::cout << ' ' << std::setw(2) << std::setfill('0') << (unsigned int)ymd.month() << '-'
		<< std::setw(2) << std::setfill('0') << (unsigned int)ymd.day() << '-'
		<< (int)ymd.year() << "  ";


	this->setTextColor(ForeGroundColor::Blue, BackGroundColor::White);
	std::cout  << '$';
	
	if (transaction.type() == Transaction::TransactionType::Credit)																			// Print the amount
		this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
	else
		this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	std::cout << std::setw(9) << std::right << std::fixed << std::setprecision(2) << std::setfill(' ') 
			  << transaction.amount() << ' ';

	std::cout << std::setw(9) << std::right;																								// Print the transaction type
	if ((transaction.type() == Transaction::TransactionType::Credit))
		std::cout << "   Credit";
	else
		std::cout << "   Debit";

	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);

	this->setTextColor((m_acctManager->indexOfAccount(transaction.account().data()) > 6) ? ForeGroundColor::Black : ForeGroundColor::White,
		static_cast<BackGroundColor>((size_t)BackGroundColor::Red + m_acctManager->indexOfAccount(transaction.account().data()) % 6));
	std::cout << std::setw(13) << std::right << transaction.account().data()																// Print the account. Organize accounts by color
		<< std::setw(48) << std::setfill('.') << transaction.description().data();
	std::cout << std::endl << std::setfill(' ');
}

void AccountingMenu::printAllTransactionsWithScrolling(const Ptr<Transaction>& list)
{
	std::stringstream ss;
	int y, totalY;
	int scroller = 0;
	DWORD rc;
	bool viewing = true;

	
	this->hideCursor();
	this->echoOff();

	// Print the options at the bottom
	this->printHorizontalBorder(COORD{ SCREEN_WIDTH, SCREEN_HEIGHT - 1 }, ForeGroundColor::White, BackGroundColor::Red, true);


	do
	{
		// Print all of the transactions that will fit in the console window
		y = SIGNIN_LINE + 4;
		for (int i = (int)list.size() - 1 - scroller; i >= 0 and y < SCREEN_HEIGHT; --i)
		{
			this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
			this->setCursorPosition(1, (unsigned short)y);
			std::cout << std::right << std::setw(5) << std::setfill(' ') << i << ' ';
			this->printTransaction(list[i], y++);
		}
		totalY = y;

		// reprint the scrolling options in white on black to make the highlight on selection effect
		this->setCursorPosition(1, SCREEN_HEIGHT);
		this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
		for (size_t i = 0; i < NUM_SCROLLS; ++i)
			std::cout << std::left << std::setw(SCREEN_WIDTH / NUM_SCROLLS) << SCROLLING[i];

		// Get input and scroll the console window accordingly, or stop
		ReadConsoleInput(m_stdInHandle, &m_ir, 1, &rc);
		if (m_ir.EventType == KEY_EVENT and
			m_ir.Event.KeyEvent.bKeyDown)
		{
			WORD vk = m_ir.Event.KeyEvent.wVirtualKeyCode;
			switch (vk)
			{
			case VK_UP:
				if (--scroller < 0) scroller = 0;

				this->highlighScrollOption(0);
				break;

			case VK_DOWN:
				if (++scroller > list.size() - 1 - totalY + 8) scroller = (int)list.size() - 42;

				this->highlighScrollOption(1);
				break;

			case VK_PRIOR:
				scroller -= (totalY - 8);
				if (scroller < 0) scroller = 0;

				this->highlighScrollOption(2);
				break;

			case VK_NEXT:
				scroller += (totalY - 8);
				if (scroller > list.size() - 1 - totalY) scroller = (unsigned short)list.size() - 42;

				this->highlighScrollOption(3);
				break;

			case VK_END:
				viewing = false;
			}
		}
	} while (viewing);
}

void AccountingMenu::printAllTransactions(const Ptr<Transaction>& list)
{
	// Loop through and print all transactions, starting at the top, saving room for the options at the bottom and total
	unsigned short y = SIGNIN_LINE + 4;
	for (int i = (int)list.size() - 1; i >= 0 and y < SCREEN_HEIGHT - 2; --i)
	{
		this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
		this->setCursorPosition(1, (unsigned short)y);
		std::cout << std::right << std::setw(5) << std::setfill('0') << i;
		this->printTransaction(list[i], y++);
	}
}

int AccountingMenu::inputDate(const size_t& lineNumber)
{
	unsigned short month, day, year;
	std::tm time = {};
	std::time_t date;
	do
	{
		std::cin.clear();
		std::cin.ignore();
		this->setCursorPosition(1, (unsigned short)lineNumber);
		this->clearHalfLine((unsigned short)lineNumber);
		std::cout << "MM DD YYYY: ";
		std::cin >> month >> day >> year;
		time.tm_mday = day;
		time.tm_mon = month - 1;
		time.tm_year = year - 1900;
		date = std::mktime(&time);
	} while (std::cin.fail() or date == -1);											// turn the mm-dd-yyyy to a int days since epoch

	return std::chrono::duration_cast<std::chrono::days>(std::chrono::system_clock::from_time_t(date).time_since_epoch()).count();
}

bool AccountingMenu::signin()
{
	std::stringstream ss;
	std::string name, pw;
	unsigned short attempts = 0;
	bool success = false;

	clearLine(SIGNIN_LINE); clearLine(SIGNIN_LINE + 1);

	// First, get the username
	do
	{
		this->echoOn();
		this->showCursor();
		// If we failed to login 3x and are trying again, make a clean slate
		if (attempts >= 3)
		{
			clearLine(SIGNIN_LINE); clearLine(SIGNIN_LINE + 1); clearLine(SIGNIN_LINE + 2);
		}
		attempts = 0;
		char ch;
		std::filesystem::path basePath = std::filesystem::current_path() / "data";
		do
		{
			this->setCursorPosition(0, SIGNIN_LINE);
			clearLine(SIGNIN_LINE);

			ss.str("");																							// clear the string stream in case of retries
			std::cout << "Enter name: ";
			std::cin >> name;
			if (not std::filesystem::exists(basePath / name))													// check if {pwd}/name is a valid directory, if not, see if we want to create a new account
			{
				clearLine(SIGNIN_LINE);
				this->setCursorPosition(1, SIGNIN_LINE);
				std::cout << name << " does not exist, create new account (y/n)? ";
				std::cin >> ch;
				if (ch == 'y' or ch == 'Y')																		// if it's anything other than 'y' or 'Y', we will cancel,
				{
					std::string pw1, pw2;
					m_userName = name;
					do
					{
						std::cin.ignore();																		// otherwise, get and validate a new password
						echoOff();
						clearLine(SIGNIN_LINE + 1);
						setCursorPosition(1, SIGNIN_LINE + 1);
						std::cout << "Password: ";
						std::cin >> pw1;
						setCursorPosition(1, SIGNIN_LINE + 1);
						std::cout << "Re-enter: ";
						std::cin >> pw2;
						pw = pw1;
					} while (pw1.compare(pw2) != 0);
					m_acctManager->CreateNewMainFile(name.c_str(), pw.c_str());									// Create a new account
					return SIGNED_OUT;																			// returned signed out so user has to signin with their new credentials
				}
			}
		} while (not std::filesystem::exists(basePath / name));

		m_userName = name;

		// Now get the password
		clearLine(SIGNIN_LINE + 2);																				// clear the third line if there was output from incorrect password entry
		clearLine(SIGNIN_LINE + 3);
		clearLine(SIGNIN_LINE + 4);
		this->echoOff();																						// turn off echo for password entry
		do
		{
			this->hideCursor();
			this->setCursorPosition(0, SIGNIN_LINE + 1);
			clearLine(SIGNIN_LINE + 1);																			// set cursor to the second line and clear the line

			std::cout << "Password: ";
			std::cin >> pw;																						// read in password
										
			this->setCursorPosition(0, SIGNIN_LINE + 3);														// prepare to print invalid password on the third line
			m_acctManager = Ptr<AccountManager>(new AccountManager(name.c_str(), pw.c_str()));					// Create account object. If it failes isGood returns false, and prints an error message
			if(not m_acctManager->isGood()) 
				std::cout << ' ' << ++attempts << " attempt" << ((attempts > 1) ? "s " : " ") << "out of 3! " << ((attempts == 3) ? "Access denied" : "Try again");
		} while (not m_acctManager->isGood() and attempts < 3);													// Keep trying while input isn't valid and we haven't tried 3x

		if (m_acctManager->isGood()) success = true;															// If we successfully got a password, then we're good
		this->echoOn();
	} while (not success);																						// If we didn't get a successful password, then we tried 3x and need to restart the signin process

	m_accountNames = m_acctManager->getAccountNames();															// Get all the names of the accounts

	clearLine(SIGNIN_LINE);  clearLine(SIGNIN_LINE + 1); clearLine(SIGNIN_LINE + 2); clearLine(SIGNIN_LINE + 3); clearLine(SIGNIN_LINE + 4);
	clearLine(2);
	ss.str("");
	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	ss << "(   ***" << name << "***   )";																		// print the account name at the top
	this->setCursorPosition(SCREEN_WIDTH / 2 - (unsigned short)ss.str().length() / 2, 2);
	std::cout << ss.str();
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);

	return SIGNED_IN;
}

void AccountingMenu::presentMenu()
{
	std::stringstream ss;
	COORD cursor{ SCREEN_WIDTH, SCREEN_HEIGHT - 2 };
	

	this->hideCursor();
	this->echoOff();

	this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
																		
	setCursorPosition(0, SCREEN_HEIGHT - 1);																	// Draw the bottom border
	printHorizontalBorder(cursor, ForeGroundColor::White, BackGroundColor::Red, true);

	setCursorPosition(1, SCREEN_HEIGHT);																		// Display the options at the bottom of the screen
	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	for (BYTE i = 0; i < NUM_OPTIONS; ++i)
		ss << i + 1 << ".) " << MENU_OPTIONS[i] << "  ";
	std::cout << std::setw(SCREEN_WIDTH) << ss.str();
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	this->setCursorPosition(1, 1);
}

void AccountingMenu::highlighScrollOption(const unsigned short& option)
{
	this->setCursorPosition(1, SCREEN_HEIGHT);
	for (size_t i = 0; i < NUM_SCROLLS; ++i)
	{
		if (i == option)
			this->setTextColor(ForeGroundColor::White, BackGroundColor::Yellow);
		std::cout << std::left << std::setw(SCREEN_WIDTH / NUM_SCROLLS) << SCROLLING[i];
		if (i == option)
			this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	}
}

unsigned short AccountingMenu::getMainMenuInput()
{
	DWORD rc;
	ReadConsoleInput(m_stdInHandle, &m_ir, 1, &rc);
	return (unsigned short)m_ir.Event.KeyEvent.uChar.AsciiChar;
}

bool AccountingMenu::addAccount()
{
	std::string name;

	m_lastViewedAccount = "";

	this->echoOn();
	this->clearScreen();
	this->printAccountNames();
	setCursorPosition(1, SIGNIN_LINE);

	// Get the name of the new account and check if it already exists, if not add then add it, otherwise don't. q to quit
	std::cout << "** Create a new Account ** (q to stop)\nName: ";
	std::cin >> name;

	if (name.compare("q") == 0 or
		name.compare("Q") == 0)
	{
		clearLine(SIGNIN_LINE); clearLine(SIGNIN_LINE + 1);
		return false;
	}
	if (not m_acctManager->addAccount(name.c_str())) 
	{
		std::cout << "Already exists!"; 
		return false; 
	}
	else
	{
		m_accountNames.addBack(name);
		std::cout << "Success!";
	}
	this->updateMainFile();
	return true;
}

bool AccountingMenu::addTransaction()
{
	Transaction t;
	//std::time_t time = 0;
	std::string account, description, other;
	float amount;
	int daysSinceEpoch;
	unsigned short type;

	m_lastViewedAccount = "";

	this->echoOn();
	this->showCursor();
	this->clearScreen();
	this->printAccountNames();
	setCursorPosition(1, SIGNIN_LINE + 5);
	std::cout << "Account: \nAmount: $\nMM DD YYYY: \nCredit (1) or Debit (2)? \nOther Acct:\nDescription:";

	// Which account
	do 
	{ 
		setCursorPosition(10, SIGNIN_LINE + 5); 
		clearHalfLine(); 
		std::cin >> account;
		if (std::atoi(account.c_str()) != 0)
		{
			unsigned short acctNum = static_cast<unsigned short>(atoi(account.c_str()));
			account = this->getManager()->getAccountNames()[acctNum - 1];
		}
		if (account.compare("q") == 0 or account.compare("Q") == 0) return false;
	} while( not m_acctManager->accountExists(account));

	// For how much
	do 
	{ 
		setCursorPosition(10, SIGNIN_LINE + 6); 
		clearHalfLine(); 
		std::cin.clear(); 
		std::cin.ignore(); 
		std::cin >> amount;
		if (amount <= 0.01) return false;
	} while(std::cin.fail());

	// When
	daysSinceEpoch = this->inputDate((size_t(SIGNIN_LINE + 7)));
	if (daysSinceEpoch == -1) return false;

	// Credit or Debit
	do
	{
		setCursorPosition(26, SIGNIN_LINE + 8);
		clearHalfLine();
		std::cin.clear();
		std::cin.ignore();
		std::cin >> type;
		if (type != 1 and type != 2) return false;
	} while (std::cin.fail());

	// The other account
	do
	{
		setCursorPosition(13, SIGNIN_LINE + 9);
		clearHalfLine();
		std::cin >> other;
		if (std::atoi(other.c_str()) != 0)
		{
			unsigned short acctNum = static_cast<unsigned short>(atoi(other.c_str()));
			other = this->getManager()->getAccountNames()[acctNum - 1];
		}
		if (other.compare("q") == 0 or other.compare("Q") == 0) return false;
	} while (not m_acctManager->accountExists(other) or account.compare(other) == 0);

	setCursorPosition(14, SIGNIN_LINE + 10);
	std::cin.ignore();
	std::getline(std::cin, description, '\n');

	// Create trandaction and add it as a double entry
	t = Transaction(daysSinceEpoch, amount, (Transaction::TransactionType)type, description, account);												// Create transaction object and create the double entry
	bool result = ((Transaction::TransactionType)type == Transaction::TransactionType::Credit) ? result = m_acctManager->doubleEntry(other, account, t) : result = m_acctManager->doubleEntry(account, other, t);

	// Return whether successful or not
	if (result)
		std::cout << "Success!";
	else
		std::cout << "There was an error... :(";

	return true;
}

bool AccountingMenu::removeTransaction()
{
	std::string acctName;
	size_t index;

	this->echoOn();
	this->showCursor();
	this->clearLine(SCREEN_HEIGHT - 2);
	
	// Check if we have an open account other than Main
	if (m_lastViewedAccount.empty() or m_lastViewedAccount.compare("Main") == 0)
	{
		do
		{
			this->clearLine(SCREEN_HEIGHT - 3);
			this->setCursorPosition(1, SCREEN_HEIGHT - 3);
			std::cout << "Account: ";
			std::cin >> acctName;
			if (acctName == "q" or acctName == "Q")
			{
				this->setCursorPosition(1, SCREEN_HEIGHT - 2);
				std::cout << "Quit";
				return false;
			}
		} while (not m_acctManager->accountExists(acctName) or acctName.compare("Main") == 0);
	}
	// Otherwise we'll use the account we're viewing
	else
	{
		std::cout << "Account: " << acctName << std::endl;
		acctName = m_lastViewedAccount;
	}
	
	// q or Q or quit
	if (acctName == "q" or acctName == "Q")
	{
		this->clearLine(SCREEN_HEIGHT - 2);
		this->clearLine(SCREEN_HEIGHT - 3);
		return false;
	}
	// Prompt of the index number, a bad index number will stop input
	this->clearLine(SCREEN_HEIGHT - 1);
	std::cout << "#:      (Bad index to quit)";
	this->setCursorPosition(4, SCREEN_HEIGHT - 1);
	std::cin >> index;

	//Bad index or bad input will stop the process
	if (std::cin.fail() or index >= m_acctManager->getAccount(acctName).getLedger().size())
	{
		std::cin.clear();
		std::cin.ignore();
		this->clearLine(SCREEN_HEIGHT - 2);
		this->clearLine(SCREEN_HEIGHT - 3);
		return false;
	}
	clearLine(SCREEN_HEIGHT - 2);
	clearLine(SCREEN_HEIGHT - 3);

	// Remove transaction and print the new ledger with the transaction removed
	m_acctManager->removeTransaction(acctName, index);
	this->viewAccount(acctName);
	return true;
}

bool AccountingMenu::viewAccount()
{
	// This viewAccount will get the account name we want to view and then call viewAccount(acctName)
	std::string account;

	this->clearScreen();

	this->printAccountNames();

	this->echoOn();
	this->showCursor();
	this->clearScreen();
	this->printAccountNames();
	setCursorPosition(1, SIGNIN_LINE);

	std::cout << "Account: ";
	do
	{
		setCursorPosition(10, SIGNIN_LINE);
		clearHalfLine();
		std::cin >> account;

		if (std::atoi(account.c_str()) != 0)
		{
			unsigned short acctNum = static_cast<unsigned short>(atoi(account.c_str()));
			account = this->getManager()->getAccountNames()[acctNum -1];
		}
		else
		{
			if(static_cast<int>(account[0]) >= 48 and static_cast<int>(account[0]) <= 57)
			{
				account[0] = static_cast<char>(std::toupper(account[0]));
				for (unsigned short i = 1; i < account.length(); ++i) account[i] = static_cast<char>(std::tolower(account[i]));
			}
		}

	} while (not m_acctManager->accountExists(account));
	m_lastViewedAccount = account;
	return this->viewAccount(account);
}

bool AccountingMenu::viewAccount(const std::string& account)
{
	this->echoOff();
	this->hideCursor();
	this->clearScreen();
	setCursorPosition(1, SIGNIN_LINE);
	this->setTextColor(ForeGroundColor::Cyan, BackGroundColor::Black);

	// print the header of the account name and how many transactions
	std::cout << "  " << account << " Account \t\t\t# of transactions: " << m_acctManager->getAccount(account).getLedger().getTransactions().size() << std::endl;

	COORD line = { 0, 0 };
	line.X = SCREEN_WIDTH;
	line.Y = SIGNIN_LINE + 1;
	this->printHorizontalBorder(line, ForeGroundColor::Cyan, BackGroundColor::Black, false);

	setCursorPosition(1, SIGNIN_LINE + 2);
	this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);

	// Print the header and the transactions. If scrolling isn't needed to view all the accounts then don't view with scrolling, otherwise view with scrolling
	this->printTransactionHeader();

	if (m_acctManager->getAccount(account).getLedger().size() < (size_t)(SCREEN_HEIGHT - SIGNIN_LINE - 5)
	or  m_firstSignIn)
		this->printAllTransactions(m_acctManager->getAccount(account).getLedger().getTransactions());
	else
		this->printAllTransactionsWithScrolling(m_acctManager->getAccount(account).getLedger().getTransactions());

	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	this->clearLine(SCREEN_HEIGHT - 2);
	this->setCursorPosition(1, SCREEN_HEIGHT - 2);

	// Print the balance; blue if 0.00, green if positive, red if negative
	if (m_acctManager->getAccount(account).getBalance() < .01 and m_acctManager->getAccount(account).getBalance() > -0.01)
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
	else
	if (m_acctManager->getAccount(account).getBalance() > .01) 
		this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	else
		this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
	
	// Print the total
	std::cout << "Total: $" << m_acctManager->getAccount(account).getBalance();
	setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	return true;
}

bool AccountingMenu::generateReport()
{
	// generateReport will get the name of the account to generate the report for, and then call generateReport(acctName)
	this->echoOn();
	this->showCursor();
	std::string account;
	if (not m_lastViewedAccount.empty())
		return this->generateReport(m_lastViewedAccount);
	do
	{
		clearScreen();
		setCursorPosition(1, SIGNIN_LINE);
		std::cout << "Account: ";
		std::cin >> account;
	}while(not m_acctManager->accountExists(account));
	return this->generateReport(account);
}

bool AccountingMenu::generateReport(const std::string& acctName)
{
	Report report(m_acctManager->getAccount(acctName).getLedger().getTransactions());
	if (m_reports.size() > 0) report = m_reports[m_reports.size() - 1];
	std::stringstream ss;
	std::string line;
	static const std::string OPTIONS[] = { "Single Date", "Date Range", "Single Amount", "Amount Range", "Description" };
	std::time_t t = std::time(nullptr);
	std::tm now;
	unsigned short currentYear;
	int type = 0;
	int daysSinceEpoch, otherDaysSinceEpoch;
	double balance;
	float amount1, amount2;
	char ch;

	// Display header of which account we're working on and how many transaction that ledger contains
	this->clearScreen();
	setCursorPosition(1, SIGNIN_LINE);
	std::cout << std::setfill(' ') << acctName << " account\t# Transactions: " << report.getAccountLedgerSize() << std::endl;

	for (size_t i = 0; i < 5; ++i)																										// Output options
	{
		ss.str("");
		ss << i + 1 << ".) " << OPTIONS[i] << " ";
		std::cout << std::setw(20) << std::left << ss.str();
	}
	do																																	// input options and validate input
	{
		std::cin.clear();
		std::cin.ignore();
		this->clearLine(SIGNIN_LINE + 2);
		this->setCursorPosition(1, SIGNIN_LINE + 2);
		std::cout << "> ";
		std::cin >> type;
	} while (std::cin.fail() or type > 5);

	switch (type)
	{
	case 1:																																// Single date
		daysSinceEpoch = this->inputDate((size_t)SIGNIN_LINE + 3);
		if (m_reports.size() > 0)
			report.generateReportByDate(report, daysSinceEpoch);
		else
			report.generateReportByDate(daysSinceEpoch);
		break;
	case 2:																																// date range
		localtime_s(&now, &t);																											// print calendar year
		currentYear = static_cast<unsigned short>(now.tm_year + 1900);
		this->printCalendarYearBottomScreen(currentYear);

		this->setCursorPosition(1, (size_t)SIGNIN_LINE + 3);
		std::cout << "From:";
		daysSinceEpoch = this->inputDate((size_t)SIGNIN_LINE + 4);
		this->setCursorPosition(1, (size_t)SIGNIN_LINE + 5);
		std::cout << "To:";
		otherDaysSinceEpoch = this->inputDate((size_t)SIGNIN_LINE + 6);
		if (daysSinceEpoch > otherDaysSinceEpoch)
		{
			this->setCursorPosition((SCREEN_WIDTH >> 1) - 14, SCREEN_HEIGHT >> 1);
			std::cout << "Input error. From date first!\n";
			this->setCursorPosition((SCREEN_WIDTH >> 1) - 14, (SCREEN_HEIGHT >> 1) + 1);
			std::cout << "Enter menu option to continue";
			return false;
		}
		if (m_reports.size() > 0)
			report.generateReportByDate(report, daysSinceEpoch, otherDaysSinceEpoch);
		else
			report.generateReportByDate(daysSinceEpoch, otherDaysSinceEpoch);
		break;
	case 3:					
		do																																// single amount
		{
			std::cin.clear();
			std::cin.ignore();
			this->clearLine(SIGNIN_LINE + 3);
			this->setCursorPosition(1, SIGNIN_LINE + 3);
			std::cout << "> $";
			std::cin >> amount1;
		} while (std::cin.fail());
		if (m_reports.size() > 0)
			report.generateReportByAmount(report, amount1);
		else
			report.generateReportByAmount(amount1);
		break;
	case 4:																																// amount range
		do																																
		{
			std::cin.clear();
			std::cin.ignore();
			this->clearLine(SIGNIN_LINE + 3);
			this->setCursorPosition(1, SIGNIN_LINE + 3);
			std::cout << "From Amount > $";
			std::cin >> amount1;
		} while (std::cin.fail());
		do																																
		{
			std::cin.clear();
			std::cin.ignore();
			this->clearLine(SIGNIN_LINE + 4);
			this->setCursorPosition(1, SIGNIN_LINE + 4);
			std::cout << "To Amount   > $";
			std::cin >> amount2;
		} while (std::cin.fail());
		if (amount1 > amount2)
			std::cout << "Input error. From amount first!";
		else
			if (m_reports.size() > 0)
				report.generateReportByAmount(report, amount1, amount2);
			else
				report.generateReportByAmount(amount1, amount2);
		break;
	case 5:																																// description
		std::cin.clear();
		std::cin.ignore();
		this->clearLine(SIGNIN_LINE + 3);
		this->setCursorPosition(1, SIGNIN_LINE + 3);
		std::cout << "Search: ";
		std::getline(std::cin, line);
		if (m_reports.size() > 0)
			report.generateReportByDescription(report, line);
		else
			report.generateReportByDescription(line);
		break;
	}

	// Sort the transactions by date or amount depending on the search criteria
	if (type == 2 or type == 5)
		qsort((void*)&(report.getReport()[0]), report.getReport().size(), sizeof(Transaction), Transaction::ComparatorByDate);
	else
	if(type == 1 or type == 3 or type == 4)
		qsort((void*)&(report.getReport()[0]), report.getReport().size(), sizeof(Transaction), Transaction::ComparatorByAmount);

	// Display the number of matches to the	query
	ss.str("");
	ss << "Number of matches: " << report.getReport().size();
	this->setCursorPosition((unsigned short)(SCREEN_WIDTH >> 1) - (unsigned short)(ss.str().length() >> 1), (unsigned short)SIGNIN_LINE + 3);
	std::cout << ss.str();

	// Print the header for the output
	this->setCursorPosition(1, SIGNIN_LINE + 2);
	this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
	this->printTransactionHeader();
	if (report.getReport().size() < (size_t)(SCREEN_HEIGHT - SIGNIN_LINE - 5))
		this->printAllTransactions(report.getReport());
	else
		this->printAllTransactionsWithScrolling(report.getReport());

		// Calculate balance and print it
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	this->clearLine(SCREEN_HEIGHT - 2);
	this->setCursorPosition(1, SCREEN_HEIGHT - 2);
	balance = report.calculateTotal();
	if (balance < 0.01 and balance > -0.01) this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
	else
	if (balance > 0.01)						this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	else
											this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
	std::cout << "Total: $" << std::setprecision(2) << balance;
	double memKb = static_cast<double>(Ptr<Transaction>::memoryUsage(false) / 1000);
	this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
	std::cout << "    Total Heap Memory Usage: " <<  memKb << "Kb";
	

	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	this->showCursor();
	this->echoOn();
	
	// check if user wants to generate a new report using the generated report, go back to the last report, or quit
	do
	{
		this->setCursorPosition(1, SCREEN_HEIGHT - 3);
		this->clearLine(SCREEN_HEIGHT - 3);
		std::cout << "Enter 'q' to stop, 'r' to refine the report " << std::flush;
		if(m_reports.size() >= 1) std::cout << "or 'b' to go back " << std::flush;
		std::cout << ':';
		std::cin >> ch;
	} while (std::cin.fail() or (ch != 'q' and ch != 'r' and ch != 'b'));

	switch (ch)
	{
	case 'q':
		m_reports.empty();
		this->setCursorPosition((SCREEN_WIDTH >> 1) - 15, SCREEN_HEIGHT >> 1);
		std::cout << "Enter menu option to continue";
		return true;

	case 'b':
		if (m_reports.size() >= 1)
			m_reports.removeBack();
		else
			std::cout << "Can't go back any more";
		return this->generateReport(acctName);

	case 'r':
		m_reports.addBack(report);
		return this->generateReport(acctName);
	}
	return true;
}

void AccountingMenu::summary()
{
	constexpr unsigned short ACCOUNT_LENGTH = 16;
	constexpr unsigned short BALANCE_LENGTH = 11;
	unsigned short totalLine = 8;
	double total = 0.00;
	std::locale original_locale = std::cout.getloc();
	std::locale comma_locale("");

	std::cout.imbue(comma_locale);

	this->clearScreen();
	this->clearLine(SCREEN_HEIGHT - 2);

	for(unsigned short x = 1; x < 90; x += 31)
	{
		this->setCursorPosition(x, 4);
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << std::left << std::setw(ACCOUNT_LENGTH) << std::setfill(' ')
			<< " Account";

		this->setTextColor(ForeGroundColor::Cyan, BackGroundColor::Black);
		std::cout << std::setw(BALANCE_LENGTH) << " Balance";
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
		if (x < 50) std::cout << "   | " << std::fixed << std::setprecision(2);
	}

	this->setCursorPosition(1, 5);
	this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);

	this->printHorizontalBorder(COORD{ SCREEN_WIDTH, 4 }, ForeGroundColor::Magenta, BackGroundColor::Black, true);

	this->setCursorPosition(1, 6);
	for (auto& accountName : m_acctManager->getAccountNames())
	{
		if (accountName == "Main"
		or  accountName == "Capital"
		or  accountName == "WorkDone"
		or  accountName == "Expense"
		or  accountName == "Taxes"
		or  accountName.substr(0, 2).compare("20") == 0) 
			continue;
	
		++totalLine;
		double balance = m_acctManager->getAccount(accountName).calculateBalance();

		
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << ' ' << std::left << std::setw(ACCOUNT_LENGTH) << std::setfill(' ') << accountName;

		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black); 
		std::cout << "$" << std::setw(BALANCE_LENGTH) << std::setfill('.');

		total += balance;

		if (balance > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else
		{
			this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
			if (balance < 0.00) balance *= -1;
		}

		std::cout << std::right << balance;
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
		std::cout << " | \n";
	}

	this->setCursorPosition(1, totalLine - 1);
	this->printHorizontalBorder(COORD{ SCREEN_WIDTH, 4 }, ForeGroundColor::Magenta, BackGroundColor::Black, false);
	this->setCursorPosition(1, totalLine);
	this->printHorizontalBorder(COORD{ SCREEN_WIDTH, 4 }, ForeGroundColor::Magenta, BackGroundColor::Black, true);

	this->setCursorPosition(1, totalLine + 1);
	std::cout << ' ';
	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	std::cout << "Total:";
	this->setTextColor(ForeGroundColor::Blue,  BackGroundColor::Black);
	std::cout << " $";

	if (total > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	else			  this->setTextColor(ForeGroundColor::Red,   BackGroundColor::Black);
	std::cout << total << std::endl;

	this->printHorizontalBorder(COORD{ SCREEN_WIDTH, 4 }, ForeGroundColor::Magenta, BackGroundColor::Black, false);

	if(m_acctManager->accountExists("WorkDone") and m_acctManager->accountExists("Expense"))
	{
		double bal = m_acctManager->getAccount("WorkDone").calculateBalance() * -1;
		double total2 = bal;

		this->setCursorPosition(33, 6);
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << std::left << std::setfill(' ') << std::setw(ACCOUNT_LENGTH) << "WorkDone";

		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << '$';
		if (bal > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else			this->setTextColor(ForeGroundColor::Red,   BackGroundColor::Black);
		std::cout << std::setw(BALANCE_LENGTH) << std::setfill('.') << std::right << bal;
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
		std::cout << " | ";


		// Expense account
		bal = m_acctManager->getAccount("Expense").calculateBalance() * -1;
		total2 += bal;

		this->setCursorPosition(33, 7);
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << std::left << std::setfill(' ') << std::setw(ACCOUNT_LENGTH) << "Expense";
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << "$";
		if (bal > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else 
		{
			this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black); 
			bal *= -1; 
		}
		std::cout << std::setw(BALANCE_LENGTH) << std::setfill('.') << std::right << bal;
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
		std::cout << " | ";


		// print total
		this->setCursorPosition(33, totalLine + 1);
		this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
		std::cout << "Total:";
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << " $";
		if (total2 > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else			   this->setTextColor(ForeGroundColor::Red,   BackGroundColor::Black);
		std::cout << total2;
	}

	if (m_acctManager->accountExists("2025-Exp_cl") and m_acctManager->accountExists("2025-WD_cl"))
	{
		// WorkDone Close account
		double bal = m_acctManager->getAccount("2025-WD_cl").calculateBalance() * -1;
		double total2 = bal;

		this->setCursorPosition(66, 6);
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << std::left << std::setfill(' ') << std::setw(ACCOUNT_LENGTH) << "WorkDone Close";

		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << '$';
		if (bal > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else			this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
		std::cout << std::setw(BALANCE_LENGTH) << std::setfill('.') << std::right << bal;
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);


		// Expense close account
		bal = m_acctManager->getAccount("2025-Exp_cl").calculateBalance() * -1;
		total2 += bal;

		this->setCursorPosition(66, 7);
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << std::left << std::setfill(' ') << std::setw(ACCOUNT_LENGTH) << "Expense Close";
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << "$";
		if (bal > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else
		{
			this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
			bal *= -1;
		}
		std::cout << std::setw(BALANCE_LENGTH) << std::setfill('.') << std::right << bal;
		this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);


		// Taxes account
		bal = m_acctManager->getAccount("Taxes").calculateBalance();
		total2 -= bal;

		this->setCursorPosition(66, 8);
		this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
		std::cout << std::left << std::setfill(' ') << std::setw(ACCOUNT_LENGTH) << "Taxes";

		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << '$';
		this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
		std::cout << std::setw(BALANCE_LENGTH) << std::setfill('.') << std::right << bal;


		// total
		this->setCursorPosition(66, totalLine + 1);
		this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
		std::cout << "Total:";
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
		std::cout << " $";
		if (total2 > 0.00) this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		else			   this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
		std::cout << total2;
	}

	std::cout.imbue(original_locale);
}

void AccountingMenu::updateOverviewList()
{
	this->clearScreen();

	this->setCursorPosition(1, SIGNIN_LINE);
	std::cout << "";
}

void AccountingMenu::updateMainFile()
{
	m_acctManager->updateMainFile(m_userName.c_str());
}

// Private helper functions
void AccountingMenu::printHorizontalBorder(const COORD& size, const ForeGroundColor& fg, const BackGroundColor& bg, bool fIsTop)
{
	printf(ESC "(0");																							// Enter Line drawing mode
	printf(CSI "%d;%dm", bg, fg);																				// Make the border white on blue
	printf(fIsTop ? "l" : "m");																					// print left corner 

	for (unsigned short i = 1; i < size.X - 1; ++i)
		printf("q");																							// draw the line from side to side, non inclusively

	printf(fIsTop ? "k" : "j");																					// print right corner
	printf(CSI "0m");
	printf(ESC "(B");																							// exit line drawing mode
}

void AccountingMenu::setCursorPosition(const unsigned short& x, const unsigned short& y)
{
	m_coord.X = x;
	m_coord.Y = y;
	printf(ESC "[%d;%dH", y, x);
}

void AccountingMenu::clearHalfScreen()
{
	for (unsigned short i = 0; i < SCREEN_HEIGHT - 2; ++i)
	{
		setCursorPosition(1, SIGNIN_LINE + i);
		clearHalfLine(i);
	}
}

void AccountingMenu::clearTwoThirds()
{
	for (unsigned short i = 0; i < SCREEN_HEIGHT - 6; ++i)
	{
		setCursorPosition(1, SIGNIN_LINE + i);
		std::cout << CLEAR_LINE << "                      ";												// should be 71 spaces
	}
}

void AccountingMenu::clearLine(const unsigned short& y)
{
	COORD save = m_coord;
	m_coord.Y = (SHORT)y;
	m_coord.X = 0;
	this->clearLine();
	m_coord = save;
}

void AccountingMenu::clearHalfLine(const unsigned short& y)
{
	COORD save = m_coord;
	m_coord.Y = (SHORT)y;
	//m_coord.X = 0;
	this->clearHalfLine();
	m_coord = save;
}
