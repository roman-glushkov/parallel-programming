#include "wallet.h"

Wallet::Wallet(Money initial) : m_money(initial) {
    if (initial < 0) throw std::out_of_range("negative initial money");
}

bool Wallet::Take(Money amount) {
    if (amount < 0) throw std::out_of_range("negative amount"); // отрицательная сумма
    std::lock_guard lock(m_mutex); // замок, у каждого свой
    if (m_money < amount) return false; // не хватает - фалс
    m_money -= amount; // уменьшаем
    return true;
}

void Wallet::Put(Money amount) {
    if (amount < 0) throw std::out_of_range("negative amount");
    std::lock_guard lock(m_mutex);
    m_money += amount; // добавляем деньги
}

Money Wallet::Get() const {
    std::lock_guard lock(m_mutex);
    return m_money; // возвращаем копию
}

bool Wallet::TransferTo(Wallet& other, Money amount) {
    if (amount < 0) throw std::out_of_range("negative amount");
    if (&other == this) return false; // не передаваем сам себе
    std::scoped_lock lock(m_mutex, other.m_mutex); // два мьютекса атомарно
    if (m_money < amount) return false; // не хватает, оба мьютекса отпустятся
    m_money -= amount; // списываем у себя
    other.m_money += amount; // зачисляем другому
    return true;
}