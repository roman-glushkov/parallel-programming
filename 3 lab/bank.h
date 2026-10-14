#pragma once
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

using AccountId = unsigned long long;
using Money = long long;

class BankOperationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Bank {
public:
    explicit Bank(Money cash) : m_cash(cash) {}

    AccountId OpenAccount() {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard lock(m_mutex);
        AccountId id = m_nextId++;
        m_accounts[id] = 0;
        return id;
    }

    Money CloseAccount(AccountId account) {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard lock(m_mutex);
        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) throw BankOperationError("no such account");
        Money balance = it->second;
        m_accounts.erase(it);
        m_cash += balance;
        return balance;
    }

    void DepositMoney(AccountId account, Money amount) {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        if (amount < 0) throw std::out_of_range("negative amount");
        std::lock_guard lock(m_mutex);
        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) throw BankOperationError("no such account");
        if (m_cash < amount) throw BankOperationError("not enough cash");
        m_cash -= amount;
        it->second += amount;
    }

    void WithdrawMoney(AccountId account, Money amount) {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        if (amount < 0) throw std::out_of_range("negative amount");
        std::lock_guard lock(m_mutex);
        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) throw BankOperationError("no such account");
        if (it->second < amount) throw BankOperationError("not enough money");
        it->second -= amount;
        m_cash += amount;
    }

    bool TryWithdrawMoney(AccountId account, Money amount) {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        if (amount < 0) throw std::out_of_range("negative amount");
        std::lock_guard lock(m_mutex);
        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) throw BankOperationError("no such account");
        if (it->second < amount) return false;
        it->second -= amount;
        m_cash += amount;
        return true;
    }

    Money GetAccountBalance(AccountId account) const {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard lock(m_mutex);
        auto it = m_accounts.find(account);
        if (it == m_accounts.end()) throw BankOperationError("no such account");
        return it->second;
    }

    Money GetCash() const {
        m_opsCount.fetch_add(1, std::memory_order_relaxed);
        std::lock_guard lock(m_mutex);
        return m_cash;
    }

    unsigned long long GetOperationsCount() const {
        return m_opsCount.load(std::memory_order_relaxed);
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<AccountId, Money> m_accounts;
    AccountId m_nextId = 1;
    Money m_cash;
    mutable std::atomic<unsigned long long> m_opsCount{0};
};

class Wallet {
public:
    explicit Wallet(Money initial) : m_money(initial) {}

    bool Take(Money amount) {
        std::lock_guard lock(m_mutex);
        if (m_money < amount) return false;
        m_money -= amount;
        return true;
    }

    void Put(Money amount) {
        std::lock_guard lock(m_mutex);
        m_money += amount;
    }

    Money Get() const {
        std::lock_guard lock(m_mutex);
        return m_money;
    }

    bool TransferTo(Wallet& other, Money amount) {
        std::scoped_lock lock(m_mutex, other.m_mutex);
        if (m_money < amount) return false;
        m_money -= amount;
        other.m_money += amount;
        return true;
    }

private:
    mutable std::mutex m_mutex;
    Money m_money;
};