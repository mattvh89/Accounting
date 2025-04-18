#include "Report.h"

Report::Report()
	   : m_accountLedger(), m_generatedList()
{
}

Report::Report(Ptr<Transaction>& ledger)
	   : m_accountLedger(ledger), m_generatedList(ARRAY, 5)
{
}

Report& Report::operator=(const Report& other)
{
	if (&other == this) return *this;
	this->m_generatedList = other.getReportCopy();
	this->m_accountLedger = other.getWorkingLedger();
	return *this;
}

size_t Report::generateReportByDate(const long long& days_since_epoch)
{
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
		if (m_accountLedger[i].date() == days_since_epoch)
			m_generatedList.addBack(m_accountLedger[i]);
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByDate(const long long& dse_start, const long long& dse_end)
{
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
		if (m_accountLedger[i].date() >= dse_start and
			m_accountLedger[i].date() <= dse_end)
				m_generatedList.addBack(m_accountLedger[i]);
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByDate(const Report& report, const long long& days_since_epoch)
{
	m_accountLedger = report.getWorkingLedger();
	m_generatedList.empty();
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
	{
		if (m_accountLedger[i].date() == days_since_epoch)
			m_generatedList.addBack(m_accountLedger[i]);
	}
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByDate(const Report& report, const long long& dse_start, const long long& dse_end)
{
	m_accountLedger = report.getWorkingLedger();
	m_generatedList.empty();
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
	{
		if (m_accountLedger[i].date() >= dse_start and m_accountLedger[i].date() <= dse_end)
			m_generatedList.addBack(m_accountLedger[i]);
	}
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByDescription(std::string_view phrase)
{
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
		if (m_accountLedger[i].description().find(phrase) != std::string::npos)
			m_generatedList.addBack(m_accountLedger[i]);
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByAmount(const double& amount)
{
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
		if (m_accountLedger[i].amount() == amount)
			m_generatedList.addBack(m_accountLedger[i]);
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByAmount(const double& amount_lo, const double& amount_hi)
{
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
		if (m_accountLedger[i].amount() >= amount_lo and
			m_accountLedger[i].amount() <= amount_hi)
				m_generatedList.addBack(m_accountLedger[i]);
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByAmount(const Report& report, const double& amount)
{
	m_accountLedger = report.getWorkingLedger();
	m_generatedList.empty();
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
	{
		if (m_accountLedger[i].amount() == amount)
			m_generatedList.addBack(m_accountLedger[i]);
	}
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByAmount(const Report& report, const double& amount_lo, const double& amount_hi)
{
	m_accountLedger = report.getWorkingLedger();
	m_generatedList.empty();
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
	{
		if (m_accountLedger[i].amount() >= amount_lo and m_accountLedger[i].amount() <= amount_hi)
			m_generatedList.addBack(m_accountLedger[i]);
	}
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

size_t Report::generateReportByDescription(const Report& report, std::string_view phrase)
{
	m_accountLedger = report.getWorkingLedger();
	m_generatedList.empty();
	for (size_t i = 0; i < m_accountLedger.size(); ++i)
	{
		if (m_accountLedger[i].description().find(phrase) != std::string::npos)
			m_generatedList.addBack(m_accountLedger[i]);
	}
	m_accountLedger = m_generatedList;
	return m_generatedList.size();
}

double Report::calculateTotal() const
{
	double total = 0.0;
	if (m_generatedList.size() == 0) return total;
	for (size_t i = 0; i < m_generatedList.size(); ++i)
	{
		if (m_generatedList[i].type() == Transaction::TransactionType::Credit)
			total -= m_generatedList[i].amount();
		else
			total += m_generatedList[i].amount();
	}
	return total;
}
