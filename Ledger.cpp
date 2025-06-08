#include "Ledger.h"
#include "Encrypt.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

Ledger::Ledger()
	   : m_transactions()
{
}

bool Ledger::loadFromFile(const char* fname, const char* pw)
{
	Transaction t;
	std::stringstream ss;
	std::ifstream ifs;
	try
	{
		ifs.open(fname, std::ios::binary);
		while (ifs.peek() != EOF)
		{
			size_t size = 0;
			ifs.read(reinterpret_cast<char*>(&size), sizeof(size));
			std::string line(size, '\0');
			ifs.read(&line[0], size);
			ss << encrypt(line, pw);
			ss >> t;
			ss.str("");
			m_transactions.addBack(t);
		}
		ifs.close();
	}
	catch (std::exception& e) { std::cerr << "***Exception***\n" << e.what() << std::endl; return false; }
	return true;
}

bool Ledger::saveToFile(const char* fname, const char* pw)
{
	std::ofstream ofs; 
	std::stringstream ss, encrypted_ss;
	std::string str;
	size_t size;

	try
	{
		ofs.open(fname, std::ios::binary | std::ios::trunc);
		if (m_transactions.size() == 0)
		{
			ofs.close();
			return true;
		}
		for (const auto& it : m_transactions)
		{
			ss << it;																											// send string to string stream for serialization
			str = encrypt(ss.str(), pw);																						// encrypt string
			size = str.size();	
			ofs.write(reinterpret_cast<char*>(&size), sizeof(size));															// write the size of the string
			ofs.write(str.c_str(), size);																						// write the string
			ss.str("");
			encrypted_ss.str("");																								// clear the string stream
		}
		ofs.close();
	}
	catch (std::exception& e) { std::cerr << e.what() << std::endl; return false; }

	return true;
}

bool Ledger::appendToFile(const char* fname, const char* pw, const Transaction& t)
{
	std::ofstream ofs;
	std::stringstream ss, encrypted_ss;
	std::string str;
	size_t size;

	if (strlen(pw) < 1) throw std::invalid_argument("***Exception***\n\tPassword must not be empty");

	try
	{
		if (not std::filesystem::exists(fname))
		{
			ofs.open(fname, std::ios::binary | std::ios::out);
		}
		else
		{
			ofs.open(fname, std::ios::binary | std::ios::app);																		// open file in binary mode for appending
		}
		ss << t;																												// send transaction to string stream for serialization
		str = encrypt(ss.str(), pw);																							// encrypt the string for writing
		size = str.size();																										// get the size of the string to write to the file first
		ofs.write(reinterpret_cast<char*>(&size), sizeof(size));																// write the size
		ofs.write(str.c_str(), size);																							// write the string
	}
	catch (std::exception& e) { std::cerr << e.what() << std::endl; return false; }

	return true;
}

size_t Ledger::addTransaction(const Transaction& t)
{
	m_transactions.addBack(t);
	return m_transactions.size();
}

size_t Ledger::removeTransaction(const Transaction& t)
{
	size_t index = 0;
	for (const auto& it : m_transactions)
	{
		if (it == t)
			m_transactions.remove(index);
		++index;
	}
	return m_transactions.size();
}

size_t Ledger::removeTransaction(const size_t& index)
{
	m_transactions.remove(index);
	return m_transactions.size();
}