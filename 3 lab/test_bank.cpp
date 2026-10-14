#include "bank.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::cerr << "FAIL: " << #cond << " (line " << __LINE__ << ")\n"; \
        std::exit(1); \
    } \
} while (0)

void test_open() {
    Bank b(0);
    auto id = b.OpenAccount();
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_open OK\n";
}

void test_deposit() {
    Bank b(100);
    auto id = b.OpenAccount();
    b.DepositMoney(id, 50);
    CHECK(b.GetAccountBalance(id) == 50);
    CHECK(b.GetCash() == 50);
    std::cout << "test_deposit OK\n";
}

void test_withdraw() {
    Bank b(100);
    auto id = b.OpenAccount();
    b.DepositMoney(id, 50);
    b.WithdrawMoney(id, 30);
    CHECK(b.GetAccountBalance(id) == 20);
    CHECK(b.GetCash() == 80);
    std::cout << "test_withdraw OK\n";
}

void test_negative() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.DepositMoney(id, -1); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_negative OK\n";
}

void test_no_account() {
    Bank b(100);
    bool threw = false;
    try { b.DepositMoney(12345, 10); } catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account OK\n";
}

void test_not_enough() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.WithdrawMoney(id, 10); } catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_not_enough OK\n";
}

void test_try_withdraw() {
    Bank b(100);
    auto id = b.OpenAccount();
    CHECK(!b.TryWithdrawMoney(id, 10));
    b.DepositMoney(id, 50);
    CHECK(b.TryWithdrawMoney(id, 30));
    CHECK(b.GetAccountBalance(id) == 20);
    std::cout << "test_try_withdraw OK\n";
}

void test_close() {
    Bank b(100);
    auto id = b.OpenAccount();
    b.DepositMoney(id, 50);
    Money bal = b.CloseAccount(id);
    CHECK(bal == 50);
    CHECK(b.GetCash() == 100);
    bool threw = false;
    try { b.GetAccountBalance(id); } catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_close OK\n";
}

void test_ops_count() {
    Bank b(100);
    auto id = b.OpenAccount();
    auto before = b.GetOperationsCount();
    b.DepositMoney(id, 50);
    b.GetAccountBalance(id);
    auto after = b.GetOperationsCount();
    CHECK(after - before == 2);
    std::cout << "test_ops_count OK\n";
}

void test_failed_ops_counted() {
    Bank b(100);
    auto id = b.OpenAccount();
    auto before = b.GetOperationsCount();
    try { b.WithdrawMoney(id, 999); } catch (...) {}
    auto after = b.GetOperationsCount();
    CHECK(after - before == 1);
    std::cout << "test_failed_ops_counted OK\n";
}

void test_multithreaded() {
    Bank b(1000);
    auto id = b.OpenAccount();
    b.DepositMoney(id, 100);

    Money cashBefore = b.GetCash();
    Money balanceBefore = b.GetAccountBalance(id);
    unsigned long long opsBefore = b.GetOperationsCount();

    constexpr int THREADS = 8;
    constexpr int ITERS = 100000;

    {
        std::vector<std::jthread> threads;
        for (int t = 0; t < THREADS; ++t) {
            threads.emplace_back([&b, id] {
                for (int i = 0; i < ITERS; ++i) {
                    b.DepositMoney(id, 1);
                    b.WithdrawMoney(id, 1);
                }
            });
        }
    }

    unsigned long long opsAfter = b.GetOperationsCount();
    Money cashAfter = b.GetCash();
    Money balanceAfter = b.GetAccountBalance(id);

    unsigned long long expected = 2ULL * THREADS * ITERS;
    CHECK(opsAfter - opsBefore == expected);
    CHECK(cashAfter == cashBefore);
    CHECK(balanceAfter == balanceBefore);

    std::cout << "test_multithreaded OK (ops delta " << (opsAfter - opsBefore) << ")\n";
}

int main() {
    test_open();
    test_deposit();
    test_withdraw();
    test_negative();
    test_no_account();
    test_not_enough();
    test_try_withdraw();
    test_close();
    test_ops_count();
    test_failed_ops_counted();
    test_multithreaded();
    std::cout << "\nAll tests passed\n";
    return 0;
}