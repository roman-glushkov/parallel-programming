#pragma once
#include "common.h"
#include <mutex>

class Wallet { // кошелек, наличные у каждого конкретного участника
public:
    explicit Wallet(Money initial); // конструктор
    // Бросает std::out_of_range, если initial < 0.

    bool Take(Money amount); // взять наличные из кошелька
    void Put(Money amount); // положить наличные в кошелек
    Money Get() const; // сколько в кошельке
    bool TransferTo(Wallet& other, Money amount); // передача денег

private:
    mutable std::mutex m_mutex; // мьютекс
    Money m_money; // наличные в кошельке
};