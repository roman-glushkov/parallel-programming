#pragma once
#include <stdexcept>

using AccountId = unsigned long long; // идентификатор счёта
using Money = long long; // сумма денег

class BankOperationError : public std::runtime_error { // исключение
public:
    using std::runtime_error::runtime_error;
};