#include "Account.h"
#include "Encrypt.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

Account::Account()
	    : m_ledger(), m_name(""), m_fileName(""), m_pw(""), m_total(0.0f), m_good(false)
{
}

Account::Account(const char* fname, const char* pw)
	    : m_ledger(), m_name(""), m_fileName(""), m_pw(pw), m_total(0.0f), m_good(false)
{
	std::string line;
	std::string clean_fname = fname;
	clean_fname.erase(0, clean_fname.find_first_not_of(" \t\n\r"));
	clean_fname.erase(clean_fname.find_last_not_of(" \t\n\r") + 1);
	m_fileName = clean_fname;
	try	{ m_ledger.loadFromFile(clean_fname.c_str(), pw); }
	catch (std::exception& e) { std::cerr << e.what() << std::endl; }
	m_good = true;
	if(m_ledger.getTransactions().size() != 0)
		this->calculateBalance();
}

Account& Account::operator=(const Account& a)
{
	m_ledger = a.m_ledger;
	m_pw = a.m_pw;
	m_name = a.m_name;
	m_fileName = a.m_fileName;
	m_total = a.m_total;
	m_good = a.m_good;
	return *this;
}

Account& Account::operator=(Account&& a) noexcept
{
	m_ledger = std::move(a.m_ledger);
	m_pw = a.m_pw;
	m_name = a.m_name;
	m_fileName = a.m_fileName;
	m_total = a.m_total;
	m_good = a.m_good;
	return *this;
}

size_t Account::addTransaction(const Transaction& t)
{
	m_ledger.addTransaction(t);
	if (t.type() == Transaction::TransactionType::Credit) m_total -= (double)t.amount();
	else
	if (t.type() == Transaction::TransactionType::Debit)  m_total += (double)t.amount();
	m_ledger.appendToFile(m_fileName.c_str(), m_pw.c_str(), t);
	return m_ledger.getTransactions().size();
}

size_t Account::removeLastTransaction()
{
	m_ledger.removeLastTransaction();
	m_ledger.saveToFile(m_fileName.c_str(), m_pw.c_str());
	return m_ledger.size();
}

size_t Account::removeTransaction(const size_t& index)
{
	m_ledger.removeTransaction(index);
	m_ledger.saveToFile(m_fileName.c_str(), m_pw.c_str());
	return m_ledger.size();
}

size_t Account::removeTransaction(const Transaction& t)
{
	m_ledger.removeTransaction(t);
	//if (m_ledger.size() == 0) std::cout << "Size of ledger is 0\n";
	m_ledger.saveToFile(m_fileName.c_str(), m_pw.c_str());
	return m_ledger.size();
}

double Account::calculateBalance()
{
	m_total = 0;
	for (const auto& it : m_ledger.getTransactions())
	{
		if (it.type() == Transaction::TransactionType::Credit)
			m_total -= (double)it.amount();
		else
		if (it.type() == Transaction::TransactionType::Debit)
			m_total += (double)it.amount();
	}
	return m_total;
}
