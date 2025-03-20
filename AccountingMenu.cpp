#include "AccountingMenu.h"
#include "AccountManager.h"
#include "Encrypt.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <string>
#include <cstdio>
#include <wchar.h>


AccountingMenu::AccountingMenu()
	: m_acctManager(), m_accountNames(), m_lastViewedAccount(""), m_stdOutHandle(GetStdHandle(STD_OUTPUT_HANDLE)), m_stdInHandle(GetStdHandle(STD_INPUT_HANDLE)),m_outputMode(), m_coord(), m_ir()
{
	// Enable virtual terminal processing on output handle
	if (!GetConsoleMode(m_stdOutHandle, &m_outputMode))
		std::cerr << "GetConsoleMode (output) failed with error " << GetLastError() << '\n';

	m_outputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(m_stdOutHandle, m_outputMode))
		std::cerr << "SetConsoleMode (output) failed with error " << GetLastError() << '\n';

	// Set cursor position
	m_coord.X = 0;
	m_coord.Y = 0;

	// Set console screen buffer size
	COORD bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
	if (!SetConsoleScreenBufferSize(m_stdOutHandle, bufferSize)) 
		std::cerr << "SetConsoleScreenBufferSize failed with error " << GetLastError() << '\n';

	// Set console window size
	SMALL_RECT windowSize = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
	if (!SetConsoleWindowInfo(m_stdOutHandle, TRUE, &windowSize))
		std::cerr << "SetConsoleWindowInfo failed with error " << GetLastError() << '\n';
}

void AccountingMenu::run()
{
	unsigned short mainMenuKey;
	bool running = true;
	bool signinStatus = SIGNED_OUT;
	bool firstSignin = true;

	while (running)
	{
		while (signinStatus == SIGNED_OUT)
		{
			this->printTitle();
			signinStatus = this->signin();
		}
		if (firstSignin)
		{
			this->viewAccount("Main");
			firstSignin = false;
		}
		this->presentMenu();
		mainMenuKey = this->getMainMenuInput();

		switch (mainMenuKey)
		{
		case '1':
			if (this->addAccount())
			{
				this->presentMenu();
			}
			break;
		case '2':
			signinStatus = SIGNED_OUT;
			for (size_t i = SIGNIN_LINE; i < SCREEN_HEIGHT - 4; ++i)
				clearLine((unsigned short)i);
			break;
		case '3':
			this->viewAccount();
			break;
		case '4':
			this->addTransaction();
			break;
		case '5':
			this->removeTransaction();
			break;
		case '6':
			//m_acctManager->sortTransactionsByDate(m_lastViewedAccount);
			//this->viewAccount(m_lastViewedAccount);
			break;
		case '7':
			running = false;
			break;
		}
	}
}

void AccountingMenu::printTitle()
{
	COORD border;
	border.X = SCREEN_WIDTH;
	border.Y = 1;

	printHorizontalBorder(border, ForeGroundColor::White, BackGroundColor::Blue, true);							// top border line

	setCursorPosition(SCREEN_WIDTH / 2 - (unsigned short)TITLE.length() / 2, 2);								// set cursor to print title
	std::cout << TITLE;

	border.Y = 3;																								// set cursor to print bottomr title border
	setCursorPosition(0, 3);
	printHorizontalBorder(border, ForeGroundColor::White, BackGroundColor::Blue, false);
}

bool AccountingMenu::signin()
{
	std::stringstream ss;
	std::string name, pw;
	unsigned short attempts = 0;
	bool success = false;

	clearLine(SIGNIN_LINE); clearLine(SIGNIN_LINE + 1);

	do
	{
		this->echoOn();
		this->showCursor();
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
			//ss << "C:\\Users\\Matthew\\source\\repos\\Acct\\Acct\\data\\" << name << "\\" << name << ".dat";					// compose the file path 
			if (not std::filesystem::exists(basePath / name))
			{
				clearLine(SIGNIN_LINE);
				this->setCursorPosition(1, SIGNIN_LINE);
				std::cout << name << " does not exist, create new account (y/n)? ";
				std::cin >> ch;
				if (ch == 'y' or ch == 'Y')
				{
					std::string pw1, pw2;
					m_userName = name;
					do
					{
						std::cin.ignore();
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
					m_acctManager->CreateNewMainFile(name.c_str(), pw.c_str());
					return SIGNED_OUT;
					//m_acctManager = Ptr<AccountManager>(new AccountManager(name.c_str(), pw.c_str()));
					//goto Finish_Label;
				}
			}
		} while (not std::filesystem::exists(basePath / name));

		m_userName = name;

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
			m_acctManager = Ptr<AccountManager>(new AccountManager(name.c_str(), pw.c_str()));				// Create account object. If it failes isGood returns false, and prints an error message
			if(not m_acctManager->isGood()) 
				std::cout << ' ' << ++attempts << " attempt" << ((attempts > 1) ? "s " : " ") << "out of 3! " << ((attempts == 3) ? "Access denied" : "Try again");
		} while (not m_acctManager->isGood() and attempts < 3);

		if (m_acctManager->isGood()) success = true;
		this->echoOn();
	} while (not success);

	m_accountNames = m_acctManager->getAccountNames();

	clearLine(SIGNIN_LINE);  clearLine(SIGNIN_LINE + 1); clearLine(SIGNIN_LINE + 2); clearLine(SIGNIN_LINE + 3); clearLine(SIGNIN_LINE + 4);
	clearLine(2);
	ss.str("");
	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	ss << "(   ***" << name << "***   )";
	this->setCursorPosition(SCREEN_WIDTH / 2 - (unsigned short)ss.str().length() / 2, 2);
	std::cout << ss.str();
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	return SIGNED_IN;
}

void AccountingMenu::presentMenu()
{
	COORD cursor;
	unsigned short y = FIRST_LINE;

	this->hideCursor();
	this->echoOff();

	this->setTextColor(ForeGroundColor::Magenta, BackGroundColor::Black);
	
	setCursorPosition((RIGHT_THIRD + ((SCREEN_WIDTH - RIGHT_THIRD) / 2)) - 4, SIGNIN_LINE);
	std::cout << std::setw(21) << std::right << "Accounts:";
	setCursorPosition((RIGHT_THIRD + ((SCREEN_WIDTH - RIGHT_THIRD) / 2)) - 4, SIGNIN_LINE + 1);
	std::cout << std::setw(21) << "_________";
	unsigned short i = 1;
	for (const auto& it : m_acctManager->getAccountNames())														// Present the accounts on the right hand side
	{
		//setCursorPosition((unsigned short)((RIGHT_THIRD + ((SCREEN_WIDTH - RIGHT_THIRD) / 2) + 8) - (it.length() / 2)), y++ + 2);
		
		setCursorPosition((unsigned short)((RIGHT_THIRD + ((SCREEN_WIDTH - RIGHT_THIRD) / 2)) - (it.length() / 2)), y++ + 2);
		std::cout << std::setw(10) << std::right << i++ << ") " << it << '\t';
	}

																		
	cursor.X = SCREEN_WIDTH;
	cursor.Y = SCREEN_HEIGHT - 2;
	setCursorPosition(0, SCREEN_HEIGHT - 1);																	// Draw the bottom border
	printHorizontalBorder(cursor, ForeGroundColor::White, BackGroundColor::Red, true);

	setCursorPosition(1, SCREEN_HEIGHT);																		// Display the options at the bottom of the screen
	this->setTextColor(ForeGroundColor::Black, BackGroundColor::White);
	for (BYTE i = 0; i < NUM_OPTIONS; ++i)
		std::cout  << i + 1 << ".) " << OPTIONS[i] << ' ';
	this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
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
	clearTwoThirds();
	setCursorPosition(1, SIGNIN_LINE);
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
	std::time_t time = 0;
	std::string account, description, other;
	float amount;
	unsigned short type, month, day, year;

	m_lastViewedAccount = "";

	this->echoOn();
	this->showCursor();
	this->clearTwoThirds();
	setCursorPosition(1, SIGNIN_LINE + 5);
	std::cout << "Account: \nAmount: $\nMM DD YYYY: \nCredit (1) or Debit (2)? \nOther Acct:\nDescription:";

	do 
	{ 
		setCursorPosition(10, SIGNIN_LINE + 5); 
		clearHalfLine(); 
		std::cin >> account;
		if (account.compare("q") == 0 or account.compare("Q") == 0) return false;
	} while( not m_acctManager->accountExists(account));

	do 
	{ 
		setCursorPosition(10, SIGNIN_LINE + 6); 
		clearHalfLine(); 
		std::cin.clear(); 
		std::cin.ignore(); 
		std::cin >> amount; 
	} while(std::cin.fail());

	do
	{
		std::tm timeStruct = {};
		
		setCursorPosition(13, SIGNIN_LINE + 7);
		clearHalfLine();
		std::cin.clear();
		std::cin.ignore();
		std::cin >> month >> day >> year;

		timeStruct.tm_mday = day;
		timeStruct.tm_mon = month - 1;
		timeStruct.tm_year = year - 1900;
		
		time = std::mktime(&timeStruct);								// seconds since epoch
	} while (std::cin.fail() or time == -1);

	do
	{
		setCursorPosition(26, SIGNIN_LINE + 8);
		clearHalfLine();
		std::cin.clear();
		std::cin.ignore();
		std::cin >> type;
	} while (std::cin.fail() or (type != 1 and type != 2));

	do
	{
		setCursorPosition(13, SIGNIN_LINE + 9);
		clearHalfLine();
		std::cin >> other;
		if (other.compare("q") == 0 or other.compare("Q") == 0) return false;
	} while (not m_acctManager->accountExists(other) or account.compare(other) == 0);

	setCursorPosition(14, SIGNIN_LINE + 10);
	std::cin.ignore();
	std::getline(std::cin, description, '\n');

	// Convert to chrono::system_clock::time_point
	auto timePoint = std::chrono::system_clock::from_time_t(time);																					// turn the mm-dd-yyyy to a long long days since epoch
	// Convert to days since epoch
	auto daysSinceEpoch = std::chrono::duration_cast<std::chrono::days>(timePoint.time_since_epoch()).count();

	t = Transaction(daysSinceEpoch, amount, (Transaction::TransactionType)type, description, account);												// Create transaction object and create the double entry
	bool result = ((Transaction::TransactionType)type == Transaction::TransactionType::Credit) ? result = m_acctManager->doubleEntry(other, account, t) : result = m_acctManager->doubleEntry(account, other, t);
	//if ((Transaction::TransactionType)type == Transaction::TransactionType::Credit)
	//	result = m_acctManager->doubleEntry(other, account, t);
	//else
	//	result = m_acctManager->doubleEntry(account, other, t);

	if (result)
		std::cout << "Success!";
	else
		std::cout << "There was an error... :(";

	return false;
}

bool AccountingMenu::removeTransaction()
{
	std::string acctName;
	size_t index;

	this->echoOn();
	this->showCursor();


	setCursorPosition(1, SCREEN_HEIGHT - 3);
	clearLine(SCREEN_HEIGHT - 2);
	clearLine(SCREEN_HEIGHT - 3);
	if(m_lastViewedAccount.empty() or m_lastViewedAccount.compare("Main") == 0)
	{
		do
		{
			std::cout << "Account: ";
			std::cin >> acctName;
		} while (not m_acctManager->accountExists(acctName));
	}
	else
	{
		std::cout << "Account: " << acctName << std::endl;
		acctName = m_lastViewedAccount;
	}
			
	if (acctName == "q" or acctName == "Q")
	{
		clearLine(SCREEN_HEIGHT - 2);
		clearLine(SCREEN_HEIGHT - 3);
		return false;
	}
	std::cout << "#:      (Bad index to quit)";
	this->setCursorPosition(4, SCREEN_HEIGHT - 2);
	std::cin >> index;
	if (std::cin.fail() or index >= m_acctManager->getAccount(acctName).getLedger().size())
	{
		std::cin.clear();
		std::cin.ignore();
		clearLine(SCREEN_HEIGHT - 2);
		clearLine(SCREEN_HEIGHT - 3);
		return false;
	}
	clearLine(SCREEN_HEIGHT - 2);
	clearLine(SCREEN_HEIGHT - 3);
	m_acctManager->removeTransaction(acctName, index);
	this->viewAccount(acctName);
}

void AccountingMenu::viewAccount()
{
	std::string account;

	this->clearScreen();

	this->echoOn();
	this->showCursor();
	clearTwoThirds();
	setCursorPosition(1, SIGNIN_LINE);

	std::cout << "Account: ";
	do
	{
		setCursorPosition(10, SIGNIN_LINE);
		clearHalfLine();
		std::cin >> account;
	} while (not m_acctManager->accountExists(account));
	m_lastViewedAccount = account;
	this->viewAccount(account);
}

bool AccountingMenu::viewAccount(const std::string& account)
{
	setCursorPosition(1, SIGNIN_LINE);
	this->setTextColor(ForeGroundColor::Cyan, BackGroundColor::Black);
	std::cout << account << " Account\n________________________________________________________________________________________\n";
	setCursorPosition(1, SIGNIN_LINE + 2);
	this->setTextColor(ForeGroundColor::Yellow, BackGroundColor::Black);
	std::cout << "### "
		<< "Date:        "
		<< "Amount:   "
		<< "Credit/Debit: "
		<< "Account:              "
		<< "Description:\n";
	std::cout << "___ ____________ __________ ___________  ________  ________________________" << std::endl;
	const Ptr<Transaction>& transactions = m_acctManager->getAccount(account).getLedger().getTransactions();
	std::ofstream ofs("log.sav", std::ios::out);
	for (size_t i = transactions.size() - 1; i >= 0 and i < SCREEN_HEIGHT - 6; --i)
	{
		if ((i + 1) % 2 == 0) std::cout << std::endl;

		this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
		std::cout << std::setw(3) << std::setfill('0') << i << ' ';																				// Print the index

		std::chrono::year_month_day ymd = std::chrono::year_month_day(std::chrono::sys_days(std::chrono::days(transactions[i].date())));		// Print the date
		std::cout << std::setw(2) << std::setfill('0') << (unsigned int)ymd.month() << '-'
			<< std::setw(2) << std::setfill('0') << (unsigned int)ymd.day() << '-'
			<< (int)ymd.year() << ' ';
	

		std::cout << std::setw(3) << std::setfill(' ') << std::right << '$'; 
		if (transactions[i].type() == Transaction::TransactionType::Credit)																		// Print the amount
			this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
		else
			this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
		std::cout << std::setw(9) << std::right << std::fixed << std::setprecision(2) << transactions[i].amount() << ' ';

		std::cout << std::setw(9) << std::left;																									// Print the transaction type
		if ((transactions[i].type() == Transaction::TransactionType::Credit))
			std::cout << "   Credit";
		else
			std::cout << "   Debit";

		this->setTextColor(ForeGroundColor::White, BackGroundColor::Black);
		
		ofs << i << ": " << (size_t)BackGroundColor::Yellow + m_acctManager->getAccountNames().indexOf(transactions[i].account().data()) << std::endl;
		
		this->setTextColor((m_acctManager->indexOfAccount(transactions[i].account().data()) > 4) ? ForeGroundColor::Black : ForeGroundColor::White,
						   static_cast<BackGroundColor>((size_t)BackGroundColor::Yellow + m_acctManager->indexOfAccount(transactions[i].account().data()) % 4 ));
		std::cout << std::setw(10) << std::right << transactions[i].account().data()															// Print the account. Organize accounts by color
			<< std::setw(38)  << std::setfill('.') << transactions[i].description().data();
		std::cout << std::endl << std::setfill(' ');
	}
	ofs.close();
	setCursorPosition(1, SCREEN_HEIGHT - 3);
	clearLine();
	if (m_acctManager->getAccount(account).getBalance() < .01 and m_acctManager->getAccount(account).getBalance() > -0.01)
		this->setTextColor(ForeGroundColor::Blue, BackGroundColor::Black);
	else
	if (m_acctManager->getAccount(account).getBalance() > .01) 
		this->setTextColor(ForeGroundColor::Green, BackGroundColor::Black);
	else
		this->setTextColor(ForeGroundColor::Red, BackGroundColor::Black);
	
	std::cout << "Total: $" << m_acctManager->getAccount(account).getBalance();
	setTextColor(ForeGroundColor::White, BackGroundColor::Black);
	return true;
}

void AccountingMenu::updateMainFile()
{
	m_acctManager->updateMainFile(m_userName.c_str());
}

// Private helper functions
void AccountingMenu::printHorizontalBorder(const COORD& size, const ForeGroundColor& fg, const BackGroundColor& bg, bool fIsTop)
{
	printf(ESC "(0");																							// Enter Line drawing mode
	printf(CSI "%d;%dm", bg, fg);																				// Make the border ehite on blue
	printf(fIsTop ? "l" : "m");																					// print left corner 

	for (size_t i = 1; i < size.X - 1; ++i)
		printf("q");																							// in line drawing mode, \x71 -> \u2500 "HORIZONTAL SCAN LINE-5"

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
	for (unsigned int i = 0; i < SCREEN_HEIGHT - 2; ++i)
	{
		setCursorPosition(1, SIGNIN_LINE + i);
		clearHalfLine(i);
	}
}

void AccountingMenu::clearTwoThirds()
{
	for (unsigned int i = 0; i < SCREEN_HEIGHT - 6; ++i)
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
