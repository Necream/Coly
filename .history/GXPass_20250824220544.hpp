#ifndef __GXPASS_HPP__
#define __GXPASS_HPP__

#include <sstream>
#include <string>
#include <limits>

namespace GXPass {

    // 类型转换工具：c1类型转c2类型（如数字转字符串）
    template<class c1, class c2>
    c2 c12c2(const c1& data) {
        c2 ret;
        std::stringstream ss;
        ss << data;
        ss >> ret;
        return ret;
    }

    // 快速幂模板函数，默认模为类型最大值（表示无模）
    template<class T = unsigned long long>
    T ksm(T a, T n, T mod = static_cast<type>(-1)) {
        if (n == 0) return 1 % mod;
        if (n == 1) return a % mod;

        T temp = ksm<T>(a, n / 2, mod);
        temp = (temp * temp) % mod;

        if (n % 2 == 1) {
            temp = (temp * (a % mod)) % mod;
        }
        return temp;
    }

    // 计算字符串字符ASCII码和，返回字符串
    template<class type = unsigned long long>
    std::string sum(const std::string& password) {
        type sum = 0;
        for (char c : password) {
            sum += static_cast<type>(c);
        }
        return c12c2<type, std::string>(sum);
    }

    // 奇偶位置加减和，返回字符串
    template<class type = unsigned long long>
    std::string dxsum1(const std::string& password) {
        type sum = 0;
        for (type i = 0; i < static_cast<type>(password.size()); i++) {
            if (i % 2 == 0) {
                sum += static_cast<type>(password[i]);
            } else {
                sum -= static_cast<type>(password[i]);
            }
        }
        return c12c2<type, std::string>(sum);
    }

    // 另一种奇偶位置加减和，返回字符串
    template<class type = unsigned long long>
    std::string dxsum2(const std::string& password) {
        type sum = 0;
        for (type i = 0; i < static_cast<type>(password.size()); i++) {
            if (i % 2 == 0) {
                sum -= static_cast<type>(password[i]);
            } else {
                sum += static_cast<type>(password[i]);
            }
        }
        return c12c2<type, std::string>(sum);
    }

    // 使用快速幂累加加密，模默认为类型最大值（无模）
    template<class type = unsigned long long>
    std::string ksmsum(const std::string& password, type modulus = static_cast<type>(-1)) {
        type sum = 0;
        for (char c : password) {
            // 显式指定模板参数，确保推导正确
            sum = (sum + ksm<type>(static_cast<type>(c), static_cast<type>(password.size()), modulus)) % modulus;
        }
        return c12c2<type, std::string>(sum);
    }

    // 编译入口，调用各种密码运算  version:-1最新，0基础，1顺序重置
    template<class type = unsigned long long>
    std::string compile(const std::string& data, int version = -1) {
        std::string ret;
        switch (version) {
            case -1: // latest
            case 1: {
                ret += dxsum1<type>(data);
                ret += sum<type>(data);
                ret += dxsum2<type>(data);
                ret += ksmsum<type>(data);
                break;
            }
            case 0: {
                ret += sum<type>(data);
                ret += dxsum1<type>(data);
                ret += dxsum2<type>(data);
                ret += ksmsum<type>(data);
                break;
            }
            default:
                break;
        }
        return ret;
    }

    // 数字转字母映射辅助函数
    std::string number2ABC(const std::string& data) {
        std::string ret = "";
        int number = -1;
        for (int i = 0; i < static_cast<int>(data.size()); i++) {
            if (number == -1) {
                number = data[i] - '0';
                continue;
            }
            if (number == 0) {
                if (data[i] == '0') continue;
                else {
                    number = data[i] - '0';
                    ret += char('A' + number - 1);
                    number = -1;
                }
            } else {
                switch (number) {
                    case 1: {
                        number = number * 10 + data[i] - '0';
                        ret += char('A' + number - 1);
                        number = -1;
                        break;
                    }
                    case 2: {
                        if (data[i] <= '6') {
                            number = number * 10 + data[i] - '0';
                            ret += char('A' + number - 1);
                            number = -1;
                        } else {
                            ret += char('A' + number - 1);
                            number = -1;
                            ret += char('A' + (data[i] - '0') - 1);
                        }
                        break;
                    }
                    default: {
                        ret += char('A' + number - 1);
                        number = data[i] - '0';
                        break;
                    }
                }
            }
        }
        if (number != 0 && number != -1) {
            ret += char('A' + number - 1);
        }
        return ret;
    }

} // namespace GXPass

#endif // __GXPASS_HPP__
