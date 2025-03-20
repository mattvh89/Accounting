#pragma once
#include "Ledger.h"

class Account
{
public:
// Constructors
																Account								();

																Account								(const char* fname,
																									 const char* pw);

							Account&							operator=							(const Account& a);

							Account&							operator=							(Account&& a)									noexcept;

							//Account&							operator+							(const Account& a);

// Getters
	inline					std::string_view					fileName							()										const					{ return m_fileName; }

	inline					std::string_view					accountName							()										const					{ return m_name;     }

	inline					bool								isGood								()										const					{ return m_good;     }

	inline					double								getBalance							()										const					{ return m_total;    }

	inline		const		Ledger&								getLedger							()										const					{ return m_ledger;   }

	inline					Ledger&								getLedger							()																{ return m_ledger;   }



// Setters
									/* add and remove transaction will also update the corresponding file */
							size_t								addTransaction						(const Transaction& t);

							size_t								removeLastTransaction				();

							size_t								removeTransaction					(const size_t& index);

							size_t								removeTransaction					(const Transaction& t);

							double								calculateBalance					();

	inline					void								setName								(std::string_view name)											{ m_name = name;}

private:
	Ledger      m_ledger;
	std::string m_name,
				m_fileName,
				m_pw;
	double      m_total;
	bool		m_good;
};

