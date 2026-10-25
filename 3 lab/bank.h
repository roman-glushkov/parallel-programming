#pragma once
#include "common.h"
#include <atomic>
#include <mutex>
#include <unordered_map>

class Bank { // работает с наличными и счетами
public:
    explicit Bank(Money cash); // конструктор
    // explicit — запрещает неявное преобразование из Money в Bank

    AccountId OpenAccount(); // открыть счёт
    Money CloseAccount(AccountId account); // закрыть счет

    // деньги
    void DepositMoney(AccountId account, Money amount); // положить наличные на счёт
    void WithdrawMoney(AccountId account, Money amount); // снять с счёта
    bool TryWithdrawMoney(AccountId account, Money amount); // снятие без исключения

    // чтение
    Money GetAccountBalance(AccountId account) const; // возвращает баланс счёта
    Money GetCash() const; // количество наличных 
    unsigned long long GetOperationsCount() const; // счётчик вызовов методов в банк

private:
    mutable std::mutex m_mutex; // один мьютекс на весь банк
    std::unordered_map<AccountId, Money> m_accounts; // счета банка
    AccountId m_nextId = 1; // след id для нового счёта
    Money m_cash; // наличные в обороте
    mutable std::atomic<unsigned long long> m_opsCount{0}; // счётчик вызовов методов
};