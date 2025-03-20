#include "AccountManager.h"
#include "smart_pointer.h"
#include "Encrypt.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

AccountManager::AccountManager(const char* fname, const char* pw)
			   : m_signinSuccess(false), m_pw(pw)
{
	std::ifstream ifs;
	std::string hash, fileName, accountName;
	std::filesystem::path basePath = std::filesystem::current_path() / "data" / fname;
	try
	{
		std::string hashLine(64, '\0');
		ifs.open(basePath / ((std::string(fname) + ".dat")), std::ios::in | std::ios::binary);						// varify hashed password
		ifs.read(&hashLine[0], 64);
		//hash = encrypt(hashLine, pw);
		if (hashLine != SHA256::hash(pw))
		{
			std::cerr << "Invalid password!\n";
			return;
		}

		while(ifs.peek() != EOF)
		{
			size_t size;
			ifs.read(reinterpret_cast<char*>(&size), sizeof(size));				// read the file name
			std::string line(std::streamsize(size), '\0');
			ifs.read(&line[0], size);											

			fileName = encrypt(line, pw);										// decrypt the path
			//std::cout << "File name: \"" << fileName << "\"" << std::endl;

			ifs.read(reinterpret_cast<char*>(&size), sizeof(size));				// read the name of the account
			line = std::string(std::streamsize(size), '\0');
			ifs.read(&line[0], size);

			accountName = encrypt(line, pw);									// decrypt the account name
			//std::cout << "Account name: " << accountName << std::endl;
			m_accounts[accountName] = Account(fileName.c_str(), pw);			// add the account to the map
			m_accounts[accountName].setName(accountName);
		}

		ifs.close();
	}
	catch (std::exception& e) { std::cerr << e.what() << std::endl; }
	m_signinSuccess = true;
}

void AccountManager::updateMainFile(const char* name)
{
	std::stringstream ss;
	std::string line;
	size_t size;
	const std::string MAIN = "Main";

	// Get the base path from the current working directory
	std::filesystem::path basePath = std::filesystem::current_path() / "data" / name;

	try
	{
		// Write to name.dat
		std::ofstream ofs(basePath / (std::string(name) + ".dat"), std::ios::binary | std::ios::out);
		ofs.write(SHA256::hash(m_pw).c_str(), 64);

		// Write to Main.dat
		ss.str("");
		ss << (basePath / (MAIN + ".dat")).string();
		size = ss.str().size();
		ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
		ofs.write(encrypt(ss.str(), m_pw).c_str(), size);

		size = MAIN.size();
		ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
		ofs.write(encrypt(MAIN, m_pw).c_str(), size);

		// Write other account files
		for (const auto& it : m_accounts)
		{
			ss.str("");
			ss << (basePath / (it.first + ".dat")).string();
			size = ss.str().size();
			ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
			ofs.write(encrypt(ss.str(), m_pw).c_str(), size);

			size = it.first.size();
			ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
			ofs.write(encrypt(it.first, m_pw).c_str(), size);
		}

		ofs.close();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}


void AccountManager::CreateNewMainFile(const char* name, const char* pw)
{
	std::ofstream ofs;
	std::stringstream ss;
	std::string line;
	size_t size;
	const std::string MAIN = "Main";

	std::filesystem::path basePath = std::filesystem::current_path() / "data" / std::string(name);

	if (not std::filesystem::exists(basePath)) { std::filesystem::create_directories(basePath); std::cout << "Doesn't exist, creating it\n"; }

	try
	{
		ofs.open(basePath / (std::string(name) + ".dat"), std::ios::binary | std::ios::out);
		ofs.write(SHA256::hash(pw).c_str(), 64);

		for (size_t i = 0; i < NUM_DEFAULT_ACCOUNTS; ++i)
		{
			ss.str("");
			ss << (basePath / (DEFAULT_ACCOUNTS[i] + ".dat")).string();
			size = ss.str().size();
			ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
			ofs.write(encrypt(ss.str(), pw).c_str(), size);

			size = DEFAULT_ACCOUNTS[i].size();
			ofs.write(reinterpret_cast<char*>(&size), sizeof(size));
			ofs.write(encrypt(DEFAULT_ACCOUNTS[i], pw).c_str(), size);
		}
		ofs.close();
	}
	catch (std::exception& e) { std::cerr << e.what(); }
}

bool AccountManager::addAccount(const char* accountName)
{
	for (const auto& it : m_accounts)
		if (it.first == accountName) return false;
	Account acct(accountName, m_pw.c_str());
	m_accounts[accountName] = acct;

	return true;
}

bool AccountManager::doubleEntry(const std::string& debitAccountName, const std::string& creditAccountName, const Transaction& t)
{
	Transaction transaction(t);
	Transaction dble(transaction);
	std::string transactionAcctName = transaction.account().data();
	bool transactionIsCredit = (transaction.type() == Transaction::TransactionType::Credit);
	
	if (not m_accounts.contains(creditAccountName) or
		not m_accounts.contains(debitAccountName))
			{ throw(std::out_of_range("***Exception***\nBad Key")); return false; }
	dble.setType((transactionIsCredit) ? Transaction::TransactionType::Debit : Transaction::TransactionType::Credit);	// set the type and account for the double entry transaction
	dble.setAccount((transactionIsCredit) ? debitAccountName : creditAccountName);

	m_accounts["Main"].addTransaction(transaction);																		// add accounts to main
	m_accounts["Main"].addTransaction(dble);

	transaction.setAccount(dble.account().data());																		// switch the accounts to reference each other
	dble.setAccount(transactionAcctName);

	m_accounts[creditAccountName].addTransaction((transactionIsCredit) ? transaction : dble);							// add the accounts
	m_accounts[debitAccountName].addTransaction((transactionIsCredit) ? dble : transaction);
	
	return true;
}

bool AccountManager::removeTransaction(std::string_view acctName, const size_t& index)
{
	if (index >= this->getAccount(acctName).getLedger().size()) return false;

	Transaction transaction = this->getAccount(acctName).getLedger().getTransactions()[index];						// Get transaction by index from the ledger
	Transaction otherTransaction(transaction);																		// Set the other transaction as a copy of the first

	if(transaction.type() == Transaction::TransactionType::Credit)													// The other trans has the opposite transaction type
		otherTransaction.setType(Transaction::TransactionType::Debit);
	else
		otherTransaction.setType(Transaction::TransactionType::Credit);

	otherTransaction.setAccount(acctName.data());																	// The original transaction has a reference to the other account's ledger

	this->getAccount(transaction.account()).removeTransaction(otherTransaction);									// remove each from their own ledger
	this->getAccount(acctName).removeTransaction(transaction);

	otherTransaction.setAccount(transaction.account().data());														// In the main ledger they don't reference each other's account, but their own, so swithc the account names
	transaction.setAccount(acctName.data());

	this->getAccount("Main").removeTransaction(otherTransaction);													// remove each of them from the main ledger
	this->getAccount("Main").removeTransaction(transaction);

	this->getAccount(otherTransaction.account().data()).calculateBalance();											// recalculate the balances
	this->getAccount(transaction.account().data()).calculateBalance();															// for all 3 accounts
	this->getAccount("Main").calculateBalance();
	
	return true;
}

//Ptr<Transaction> AccountManager::getTransactionsSortedByDate(std::string_view acctname) const
//{
//	Ptr<Transaction> sortedList = m_accounts.find(acctname.data())->second.getLedger().getTransactions();
//	std::sort(sortedList.begin(), sortedList.end(), Transaction::ComparatorByDate);
//	return sortedList;
//}
//
//Ptr<Transaction> AccountManager::getTransactionsSortedByAmount(std::string_view acctname) const
//{
//	Ptr<Transaction> sortedList = m_accounts.find(acctname.data())->second.getLedger().getTransactions();
//	std::sort(sortedList.begin(), sortedList.end(), Transaction::ComparatorByAmount);
//	return sortedList;
//}
//
//void AccountManager::sortTransactionsByDate(std::string_view acctname)
//{
//	std::sort(this->getAccount(acctname).getLedger().getTransactions().begin(),
//			  this->getAccount(acctname).getLedger().getTransactions().end(),
//			  Transaction::ComparatorByDate);
//}
//
//void AccountManager::sortTransactionsByAmount(std::string_view acctname)
//{
//	std::sort(this->getAccount(acctname).getLedger().getTransactions().begin(),
//		this->getAccount(acctname).getLedger().getTransactions().end(),
//		Transaction::ComparatorByAmount);
//}

bool AccountManager::accountExists(const std::string& acct) const
{
	if (m_accounts.find(acct) == m_accounts.end()) return false;
	else return true;
}

Account& AccountManager::getAccount(std::string_view acctName)
{
	//std::cout << "get account\n";
	if (m_accounts.contains(acctName.data())) return m_accounts[acctName.data()];
	else throw(std::out_of_range("***Exception***\nBad Key"));
}

void AccountManager::printAccountNames() const
{
	for (const auto& it : m_accounts)
		std::cout << it.first << '\t';
}

Ptr<std::string> AccountManager::getAccountNames() const
{
	Ptr<std::string> names(ARRAY, 3);
	for (const auto& it : m_accounts)
		names.addBack(it.first);
	return names;
}

Ptr<Transaction> AccountManager::getByDate(std::string_view accountName, const long long& dsed) const
{
	Ptr<Transaction> list;
	//for (auto& it : m_accounts.find(accountName.data())->second.getLedger().getTransactions())
	//	if(it.date() == dsed)
	//		list.addBack(it);

	return list;
}

int AccountManager::indexOfAccount(const std::string& name) const
{
	const auto it = m_accounts.find(name);
	if (it != m_accounts.end())
		return (int)std::distance(m_accounts.begin(), it);
	return -1;
}

