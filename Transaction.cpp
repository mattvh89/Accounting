#include "Transaction.h"

Transaction::Transaction()
		    : m_date(0), m_amnt(0.0f), m_type(Transaction::TransactionType::None), m_desc("")
{
}

Transaction::Transaction(const long long& date, const float amount, const TransactionType& type, const std::string& description, const std::string& account)
			: m_date(date), m_amnt(amount), m_type(type), m_desc(description), m_acct(account)
{
}

Transaction::Transaction(const Transaction& t)
		    : m_date(t.m_date), m_amnt(t.m_amnt), m_type(t.m_type), m_desc(t.m_desc), m_acct(t.m_acct)
{
}

Transaction& Transaction::operator=(const Transaction& t)
{
	m_date = t.m_date;
	m_amnt = t.m_amnt;
	m_type = t.m_type;
	m_desc = t.m_desc;
	m_acct = t.m_acct;
	return *this;
}

bool Transaction::operator==(const Transaction& t) const
{
	return m_acct == t.account() and 
		   m_amnt == t.amount()  and 
		   m_date == t.date()    and 
		   m_type == t.type()    and 
		   m_desc == t.description();
}

bool Transaction::equalExceptAccount(const Transaction& t) const
{
	return m_amnt == t.amount() and
		   m_date == t.date() and
		   m_type == t.type() and
		   m_desc == t.description();
}

std::ostream& operator<<(std::ostream& os, const Transaction& t)
{
	os << t.amount() << ' ' << t.date() << ' ' << (unsigned)t.type() << ' ' << t.description() << ';' << t.account() << std::endl;		// Serialize data using the amount first. ready to be encrypted
	return os;
}

std::istream& operator>>(std::istream& is, Transaction& t)
{
	long long date;
	float amount;
	unsigned type;
	std::string description, account;

	is >> amount >> date >> type;																											// Serialization uses the amount first to get a more random seed for the encryption
	is.ignore(1);																															// there's a space next, ignore it
	//description = description.substr(0, description.size() - 2);
	std::getline(is, description, ';');																										// rest of the line is the description
	std::getline(is, account, '\n');

	t = Transaction(date, amount, (Transaction::TransactionType)type, description, account);												// form transaction object

	return is;
}
