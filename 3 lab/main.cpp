#include "bank.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <syncstream>
#include <thread>
#include <vector>

namespace {
    std::atomic<bool> g_stop{false};
    thread_local std::mt19937 g_rng{std::random_device{}()};

    void signal_handler(int) { g_stop.store(true); }

    int rand_int(int n) {
        return std::uniform_int_distribution<int>(0, n - 1)(g_rng);
    }

    void log(const std::string& s) {
        std::osyncstream(std::cout) << s << "\n";
    }
}

struct Args {
    std::string mode = "parallel";
    int seconds = 5;
    bool quiet = false;
};

static Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--quiet") a.quiet = true;
        else if (s == "--mode" && i + 1 < argc) a.mode = argv[++i];
        else if (s == "--seconds" && i + 1 < argc) a.seconds = std::stoi(argv[++i]);
        else throw std::runtime_error("unknown argument: " + s);
    }
    if (a.mode != "single" && a.mode != "parallel")
        throw std::runtime_error("--mode must be single or parallel");
    if (a.seconds <= 0)
        throw std::runtime_error("--seconds must be positive");
    return a;
}

int main(int argc, char** argv) {
    Args args;
    try { args = parse_args(argc, argv); }
    catch (const std::exception& e) {
        std::cerr << "usage: bank --mode <single|parallel> --seconds <N> [--quiet]\n";
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    std::signal(SIGINT, signal_handler);

    constexpr Money TOTAL = 12900;

    Bank bank(TOTAL);
    AccountId directorAcc = bank.OpenAccount();
    AccountId fatherAcc   = bank.OpenAccount();
    AccountId sellerAcc   = bank.OpenAccount();

    Wallet directorWallet(10000);
    Wallet fatherWallet(500);
    Wallet motherWallet(500);
    Wallet child1Wallet(200);
    Wallet child2Wallet(200);
    Wallet sellerWallet(1500);

    struct Actor {
        std::string name;
        std::function<void()> act;
    };

    std::vector<Actor> actors;

    actors.push_back({"Director", [&] {
        if (directorWallet.Get() < 500) {
            if (bank.TryWithdrawMoney(directorAcc, 500)) {
                directorWallet.Put(500);
                if (!args.quiet) log("Director withdraws 500");
            }
            return;
        }
        Money salary = 100;
        if (directorWallet.Take(salary)) {
            try {
                bank.DepositMoney(fatherAcc, salary);
                if (!args.quiet) log("Director pays salary 100 to Father");
            } catch (...) { directorWallet.Put(salary); }
        }
    }});

    actors.push_back({"Father", [&] {
        if (fatherWallet.Get() < 100) {
            if (bank.TryWithdrawMoney(fatherAcc, 300)) {
                fatherWallet.Put(300);
                if (!args.quiet) log("Father withdraws 300");
            }
            return;
        }
        int c = rand_int(3);
        if (c == 0) {
            Money a = 30;
            if (fatherWallet.Take(a)) {
                try {
                    bank.DepositMoney(directorAcc, a);
                    if (!args.quiet) log("Father pays electricity 30");
                } catch (...) { fatherWallet.Put(a); }
            }
        } else if (c == 1) {
            Money a = 50;
            if (fatherWallet.TransferTo(motherWallet, a)) {
                if (!args.quiet) log("Father gives 50 to Mother");
            }
        } else {
            Money a = 20;
            Wallet& child = rand_int(2) ? child1Wallet : child2Wallet;
            if (fatherWallet.TransferTo(child, a)) {
                if (!args.quiet) log("Father gives 20 to Child");
            }
        }
    }});

    actors.push_back({"Mother", [&] {
        if (motherWallet.TransferTo(sellerWallet, 40)) {
            if (!args.quiet) log("Mother buys 40 in store");
        }
    }});

    actors.push_back({"Child1", [&] {
        if (child1Wallet.TransferTo(sellerWallet, 15)) {
            if (!args.quiet) log("Child1 buys 15 in store");
        }
    }});

    actors.push_back({"Child2", [&] {
        if (child2Wallet.TransferTo(sellerWallet, 15)) {
            if (!args.quiet) log("Child2 buys 15 in store");
        }
    }});

    actors.push_back({"Seller", [&] {
        if (sellerWallet.Get() < 100) {
            if (bank.TryWithdrawMoney(sellerAcc, 200)) {
                sellerWallet.Put(200);
                if (!args.quiet) log("Seller withdraws 200");
            }
            return;
        }
        int c = rand_int(2);
        if (c == 0) {
            Money a = 100;
            if (sellerWallet.Take(a)) {
                try {
                    bank.DepositMoney(sellerAcc, a);
                    if (!args.quiet) log("Seller deposits 100");
                } catch (...) { sellerWallet.Put(a); }
            }
        } else {
            Money a = 50;
            if (sellerWallet.Take(a)) {
                try {
                    bank.DepositMoney(directorAcc, a);
                    if (!args.quiet) log("Seller pays electricity 50");
                } catch (...) { sellerWallet.Put(a); }
            }
        }
    }});

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(args.seconds);

    if (args.mode == "single") {
        while (!g_stop.load() && std::chrono::steady_clock::now() < deadline) {
            for (auto& actor : actors) {
                actor.act();
                if (g_stop.load()) break;
            }
        }
    } else {
        std::vector<std::jthread> threads;
        for (auto& actor : actors) {
            threads.emplace_back([&actor, &deadline] {
                while (!g_stop.load() && std::chrono::steady_clock::now() < deadline) {
                    actor.act();
                }
            });
        }
        while (!g_stop.load() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        g_stop.store(true);
    }

    auto totalOps = bank.GetOperationsCount();

    Money sumWallets = directorWallet.Get() + fatherWallet.Get() + motherWallet.Get()
                     + child1Wallet.Get() + child2Wallet.Get() + sellerWallet.Get();
    Money cash = bank.GetCash();
    Money sumAccounts = bank.GetAccountBalance(directorAcc)
                      + bank.GetAccountBalance(fatherAcc)
                      + bank.GetAccountBalance(sellerAcc);

    std::cout << "mode: " << args.mode << "\n";
    std::cout << "seconds: " << args.seconds << "\n";
    std::cout << "operations: " << totalOps << "\n";
    std::cout << "ops/sec: " << (double)totalOps / args.seconds << "\n";
    std::cout << "sum wallets: " << sumWallets << "\n";
    std::cout << "bank cash: " << cash << "\n";
    std::cout << "sum accounts: " << sumAccounts << "\n";

    bool ok1 = (sumWallets == cash);
    bool ok2 = (cash + sumAccounts == TOTAL);
    std::cout << "invariant wallets==cash: " << (ok1 ? "OK" : "FAIL") << "\n";
    std::cout << "invariant cash+accounts==total: " << (ok2 ? "OK" : "FAIL") << "\n";

    return (ok1 && ok2) ? 0 : 1;
}