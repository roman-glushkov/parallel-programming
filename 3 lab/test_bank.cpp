#include "bank.h"
#include "wallet.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::cerr << "ERROR\n"; \
        std::exit(1); \
    } \
} while (0)\

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

void test_negative_deposit() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.DepositMoney(id, -1); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_negative_deposit OK\n";
}

void test_negative_withdraw() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.WithdrawMoney(id, -1); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_negative_withdraw OK\n";
}

void test_negative_try_withdraw() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.TryWithdrawMoney(id, -1); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    std::cout << "test_negative_try_withdraw OK\n";
}

void test_negative_initial_cash() {
    bool threw = false;
    try { Bank b(-10); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    std::cout << "test_negative_initial_cash OK\n";
}

void test_no_account_deposit() {
    Bank b(100);
    bool threw = false;
    try { b.DepositMoney(999999, 10); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account_deposit OK\n";
}

void test_no_account_withdraw() {
    Bank b(100);
    bool threw = false;
    try { b.WithdrawMoney(999999, 10); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account_withdraw OK\n";
}

void test_no_account_try_withdraw() {
    Bank b(100);
    bool threw = false;
    try { b.TryWithdrawMoney(999999, 10); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account_try_withdraw OK\n";
}

void test_no_account_get_balance() {
    Bank b(100);
    bool threw = false;
    try { b.GetAccountBalance(999999); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account_get_balance OK\n";
}

void test_no_account_close() {
    Bank b(100);
    bool threw = false;
    try { b.CloseAccount(999999); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    std::cout << "test_no_account_close OK\n";
}

void test_not_enough_withdraw() {
    Bank b(100);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.WithdrawMoney(id, 10); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_not_enough_withdraw OK\n";
}

void test_not_enough_cash_deposit() {
    Bank b(10);
    auto id = b.OpenAccount();
    bool threw = false;
    try { b.DepositMoney(id, 100); }
    catch (const BankOperationError&) { threw = true; }
    CHECK(threw);
    CHECK(b.GetAccountBalance(id) == 0);
    std::cout << "test_not_enough_cash_deposit OK\n";
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
    try { b.GetAccountBalance(id); }
    catch (const BankOperationError&) { threw = true; }
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

void test_wallet_take_put() {
    Wallet w(50);
    CHECK(w.Take(20));
    CHECK(w.Get() == 30);
    CHECK(!w.Take(40));
    w.Put(10);
    CHECK(w.Get() == 40);
    std::cout << "test_wallet_take_put OK\n";
}

void test_wallet_negative() {
    Wallet w(50);
    bool threw = false;
    try { w.Take(-10); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(w.Get() == 50);

    threw = false;
    try { w.Put(-10); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    CHECK(w.Get() == 50);

    threw = false;
    try { Wallet w2(-5); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    std::cout << "test_wallet_negative OK\n";
}

void test_wallet_transfer() {
    Wallet a(100);
    Wallet b(50);

    CHECK(a.TransferTo(b, 30));
    CHECK(a.Get() == 70);
    CHECK(b.Get() == 80);

    CHECK(!a.TransferTo(b, 1000));
    CHECK(a.Get() == 70);
    CHECK(b.Get() == 80);

    CHECK(!a.TransferTo(a, 10));
    CHECK(a.Get() == 70);

    bool threw = false;
    try { a.TransferTo(a, -10); }
    catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);
    std::cout << "test_wallet_transfer OK\n";
}

void test_wallet_concurrent_transfer() {
    Wallet a(1000000);
    Wallet b(1000000);

    constexpr int THREADS = 4;
    constexpr int ITERS = 10000;

    {
        std::vector<std::jthread> threads;

        for (int t = 0; t < THREADS; ++t) {
            threads.emplace_back([&a, &b, t] {
                for (int i = 0; i < ITERS; ++i) {
                    if (t % 2 == 0) {
                        a.TransferTo(b, 1);  
                    } else {
                        b.TransferTo(a, 1); 
                    }
                }
            });
        }
    }

    CHECK(a.Get() + b.Get() == 2000000);
    std::cout << "test_wallet_concurrent_transfer OK\n";
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

    std::cout << "test_multithreaded OK \n";
}

int main() {
    test_open();
    test_deposit();
    test_withdraw();
    test_negative_deposit();
    test_negative_withdraw();
    test_negative_try_withdraw();
    test_negative_initial_cash();
    test_no_account_deposit();
    test_no_account_withdraw();
    test_no_account_try_withdraw();
    test_no_account_get_balance();
    test_no_account_close();
    test_not_enough_withdraw();
    test_not_enough_cash_deposit();
    test_try_withdraw();
    test_close();
    test_ops_count();
    test_failed_ops_counted();
    test_wallet_take_put();
    test_wallet_negative();
    test_wallet_transfer();
    test_wallet_concurrent_transfer();
    test_multithreaded();

    std::cout << "\nAll tests passed\n";
    return 0;
}