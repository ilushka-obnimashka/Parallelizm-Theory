#ifndef CLIENT_H
#define CLIENT_H

#include <random>
#include <string>
#include "functions.h"
#include "server.h"

std::mutex outputMutex;


void safePrint(std::ofstream& out, const std::string &message) {
    std::lock_guard<std::mutex> lock(outputMutex);
    out << message << std::endl;
}

std::string getTypeName(const std::type_info &type) {
    std::string name = type.name();
    if (name == "d") return "double";
    if (name == "i") return "int";
    if (name == "f") return "float";
    if (name == "c") return "char";
    if (name == "l") return "long";
    if (name == "x") return "long long";
    if (name == "s") return "short";
    if (name == "m") return "unsigned long";
    if (name == "j") return "unsigned int";
    if (name == "h") return "unsigned short";
    if (name == "y") return "unsigned long long";
    if (name == "b") return "bool";
    return name;
}

template<typename Tserver, typename Tclient>
std::string formatMessage(size_t taskId, Tclient arg1, Tclient *arg2 = nullptr) {
    std::string typeNameServer = getTypeName(typeid(Tserver));
    std::string typeNameClient = getTypeName(typeid(Tclient));

    if constexpr (std::is_same_v<Tclient, void>) {
        return std::to_string(taskId) + "," + typeNameServer + "," + typeNameClient + ",sin," +
               std::to_string(static_cast<double>(arg1));
    } else if (arg2 != nullptr) {
        return std::to_string(taskId) + "," + typeNameServer + "," + typeNameClient + ",pow," +
               std::to_string(static_cast<double>(arg1)) + "," + std::to_string(static_cast<double>(*arg2));
    } else {
        return std::to_string(taskId) + "," + typeNameServer + "," + typeNameClient + ",sqrt," +
               std::to_string(static_cast<double>(arg1));
    }
}

template<typename Tclient, typename Tserver>
class Client {
public:

    Client(std::ofstream& out) : out_(out) {};
    virtual ~Client() = default;
    virtual size_t Client2ServerTask(Server<Tserver> &server) = 0;

protected:

    std::ofstream& out_;

    std::mt19937 gen_{std::random_device{}()};

    Tclient GenerateRandom(Tclient min, Tclient max) {
        if constexpr (std::is_integral_v<Tclient>) {
            std::uniform_int_distribution<Tclient> dist(min, max);
            return dist(gen_);
        } else {
            std::uniform_real_distribution<Tclient> dist(min, max);
            return dist(gen_);
        }
    }
};

template<typename Tclient, typename Tserver>
class SinClient : public Client<Tclient, Tserver> {
public:
    SinClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}

    size_t Client2ServerTask(Server<Tserver> &server) override {
        Tclient arg = this->GenerateRandom(Tclient(-10), Tclient(10));
        int delay_ms = this->GenerateRandom(1000, 4000);
        size_t task_id = server.AddTask([arg, delay_ms] { return MathFunctions::FunSin(arg, delay_ms); });

        safePrint(this->out_,formatMessage<Tserver, Tclient>(task_id, arg));

        return task_id;
    }
};

template<typename Tclient, typename Tserver>
class SqrtClient : public Client<Tclient, Tserver> {
public:
    SqrtClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}
    size_t Client2ServerTask(Server<Tserver> &server) override {
        Tclient arg = this->GenerateRandom(Tclient(0), Tclient(100));
        int delay_ms = this->GenerateRandom(1000, 4000);
        size_t task_id = server.AddTask([arg, delay_ms] { return MathFunctions::FunSqrt(arg, delay_ms); });

        safePrint(this->out_,formatMessage<Tserver, Tclient>(task_id, arg));

        return task_id;
    }
};

template<typename Tclient, typename Tserver>
class PowClient : public Client<Tclient, Tserver> {
public:
    PowClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}
    size_t Client2ServerTask(Server<Tserver> &server) override {
        Tclient x = this->GenerateRandom(Tclient(0), Tclient(10));

        Tclient y = x == 0 ? this->GenerateRandom(Tclient(0), Tclient(5))
                           : y = this->GenerateRandom(Tclient(-5), Tclient(5));
        int delay_ms = this->GenerateRandom(1000, 4000);
        size_t task_id = server.AddTask([x, y, delay_ms] { return MathFunctions::FunPow(x, y, delay_ms); });

        safePrint(this->out_, formatMessage<Tserver, Tclient>(task_id, x, &y));

        return task_id;
    }
};

#endif
