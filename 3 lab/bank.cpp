#include "bank.h"

Bank::Bank(Money cash) : m_cash(cash) {
    if (cash < 0) throw std::out_of_range("negative initial cash");
}

AccountId Bank::OpenAccount() {
    m_opsCount.fetch_add(1, std::memory_order_relaxed); // счётчик
    std::lock_guard lock(m_mutex); // замок
    AccountId id = m_nextId++; // выдача id
    m_accounts[id] = 0; // новый счёт
    return id; // выводим id
}

Money Bank::CloseAccount(AccountId account) {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard lock(m_mutex);
    auto it = m_accounts.find(account); // ищем счёт
    if (it == m_accounts.end()) throw BankOperationError("no such account"); // если не нашли аккаунт - бросаем
    Money balance = it->second; // запоминаем остаток
    m_accounts.erase(it); // удаляем счёт
    m_cash += balance; // остаток уходит в наличные
    return balance; // возвращаем сколько было на балансе
}

void Bank::DepositMoney(AccountId account, Money amount) {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    if (amount < 0) throw std::out_of_range("negative amount"); // отрицателньая сумма
    std::lock_guard lock(m_mutex);
    auto it = m_accounts.find(account);
    if (it == m_accounts.end()) throw BankOperationError("no such account");
    if (m_cash < amount) throw BankOperationError("not enough cash"); // в банке недостаточно наличных
    m_cash -= amount; // уменьшаем наличные
    it->second += amount; // увеличиваем баланс счёта
}

void Bank::WithdrawMoney(AccountId account, Money amount) {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    if (amount < 0) throw std::out_of_range("negative amount");
    std::lock_guard lock(m_mutex);
    auto it = m_accounts.find(account);
    if (it == m_accounts.end()) throw BankOperationError("no such account");
    if (it->second < amount) throw BankOperationError("not enough money"); // на счете не хватает
    it->second -= amount; // уменьшаем баланс
    m_cash += amount; // наличные растут
}

bool Bank::TryWithdrawMoney(AccountId account, Money amount) {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    if (amount < 0) throw std::out_of_range("negative amount");
    std::lock_guard lock(m_mutex);
    auto it = m_accounts.find(account);
    if (it == m_accounts.end()) throw BankOperationError("no such account");
    if (it->second < amount) return false; // не хватает - возвращаем фалс
    it->second -= amount; 
    m_cash += amount;
    return true; // успех
}

Money Bank::GetAccountBalance(AccountId account) const {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard lock(m_mutex);
    auto it = m_accounts.find(account);
    if (it == m_accounts.end()) throw BankOperationError("no such account");
    return it->second; // возвращаем копию баланса счёта
}

Money Bank::GetCash() const {
    m_opsCount.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard lock(m_mutex);
    return m_cash; // копия возвращаем наличные
}

unsigned long long Bank::GetOperationsCount() const {
    return m_opsCount.load(std::memory_order_relaxed);
}